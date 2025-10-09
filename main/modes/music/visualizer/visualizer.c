#include "visualizer.h"
#include "vec3.h"
#include "mat3.h"
#include "mat4.h"
#include "curve3.h"
#include <stdlib.h>

#define MAXSHAPELINE 64
#define INT_TO_STR(n) ({ \
    static char str[32]; \
    snprintf(str, sizeof(str), "%d", (n)); \
    str; \
})

const char visualizerModeName[] = "Visualizer";
 
static void visualizerEnterMode(void);
static void visualizerExitMode(void);
static void visualizerMainLoop(int64_t elapsedUs);
 
swadgeMode_t visualizerMode = {
    .modeName                 = visualizerModeName,
    .wifiMode                 = NO_WIFI,
    .overrideUsb              = false,
    .usesAccelerometer        = false,
    .usesThermometer          = false,
    .overrideSelectBtn        = false,
    .fnEnterMode              = visualizerEnterMode,
    .fnExitMode               = visualizerExitMode,
    .fnMainLoop               = visualizerMainLoop,
    .fnAudioCallback          = NULL,
    .fnBackgroundDrawCallback = NULL,
    .fnEspNowRecvCb           = NULL,
    .fnEspNowSendCb           = NULL,
    .fnAdvancedUSB            = NULL,
};

// Declare curves here so we can access them in the main loop
static size_t countCurves = 0;
static curve3_t *curves = NULL;
static mat3_t camK = {
    {
        {120,         0,   TFT_WIDTH/2},
        {0,         120,  TFT_HEIGHT/2},
        {0,           0,             1}
    }
};

int allocCurve(curve3_t **array, size_t *count)
// Allocates memory for a new curve3_t to be added, attempting realloc with a tmp pointer first
{   
    curve3_t *tmp = realloc(*array, (*count + 1) * sizeof(curve3_t));
    if (!tmp) {
        perror("realloc failed");
        return 0;
    }

    *array = tmp;
    memset(&(*array)[*count], 0, sizeof(curve3_t));
    return 1;
}

static void visualizerEnterMode()
{
    // Open and parse custom .shapes file
    FILE *f = fopen("./main/modes/music/visualizer/shapes/vector_u_offset.shapes", "r");
    char line[MAXSHAPELINE];
    puts("FINISH OPEN");
    if (f == NULL) {
        puts("PROBLEM OPENING");
    }
    while(fgets(line, MAXSHAPELINE, f) != NULL)
    {   
        // Allocate memory for new curves
        allocCurve(&curves, &countCurves);
        curve3_t *curve = &curves[countCurves];
        char *ptrSpace;
        char *token;
        
        // Parse info from each line, sample line:
        // 0 c050 303,271,0 249,-27,0
        token = strtok_r(line, " ", &ptrSpace);
        curve->spline_id = (int8_t)atoi(token);
        
        // color e.g. c052
        token = strtok_r(NULL, " ", &ptrSpace);
        int r = token[1] - '0';
        int g = token[2] - '0';
        int b = token[3] - '0';
        int color = 0;

        // Calculate enum value using digits of color code
        // Each channel is an int between 0 and 5 (6 values)
        color += b;
        color += g * 6;
        color += r * 36;
        curve->color = (int8_t)color;
        
        // Parse sequential points, each point separated by a space
        // and each component separated by a comma
        int numPoints = 0;
        token = strtok_r(NULL, " ", &ptrSpace); // Get first point
        while (token != NULL) {
            vec3_t curvePoint;
            
            char *ptrComma;
            char *subtoken = strtok_r(token, ",", &ptrComma);
            curvePoint.x = (int16_t)strtol(subtoken, NULL, 10);

            subtoken = strtok_r(NULL, ",", &ptrComma);
            curvePoint.y = (int16_t)strtol(subtoken, NULL, 10);

            subtoken = strtok_r(NULL, ",", &ptrComma);
            curvePoint.z = (int16_t)strtol(subtoken, NULL, 10);

            curve->points[numPoints++] = curvePoint;
            token = strtok_r(NULL, " ", &ptrSpace);
        }
        
        curve->num_points = (int8_t)numPoints;
        countCurves++;
    }

    fclose(f);
}
 
static void visualizerExitMode()
{
}
 
static void visualizerMainLoop(int64_t elapsedUs)
{
    int factor = 4;
    int halfWidth = TFT_WIDTH / 2;
    int halfHeight = TFT_HEIGHT / 2;
    for (size_t i = 0; i < countCurves; i++) {
        curve3_t curve = curves[i];
        if (curve.num_points == 2) {
            int16_t x1, y1, x2, y2;
            vec3_t pt1 = mat3_projectVec(camK, curve.points[0]);
            vec3_t pt2 = mat3_projectVec(camK, curve.points[1]);
            x1 = pt1.x;
            y1 = pt1.y;
            x2 = pt2.x;
            y2 = pt2.y;
            
            // x1 = pt1.x / factor + halfWidth;
            // y1 = pt1.y / factor + halfHeight;
            // x2 = pt2.x / factor + halfWidth;
            // y2 = pt2.y / factor + halfHeight;
            drawLine(x1, y1, x2, y2, curve.color, 0);
            
        } else if (curve.num_points == 4) {
            int16_t x1, y1, x2, y2, x3, y3, x4, y4;
            vec3_t pt1 = mat3_projectVec(camK, curve.points[0]);
            vec3_t pt2 = mat3_projectVec(camK, curve.points[1]);
            vec3_t pt3 = mat3_projectVec(camK, curve.points[2]);
            vec3_t pt4 = mat3_projectVec(camK, curve.points[3]);
            x1 = pt1.x;
            y1 = pt1.y;
            x2 = pt2.x;
            y2 = pt2.y;
            x3 = pt3.x;
            y3 = pt3.y;
            x4 = pt4.x;
            y4 = pt4.y;

            // x1 = pt1.x / factor + halfWidth;
            // y1 = pt1.y / factor + halfHeight;
            // x2 = pt2.x / factor + halfWidth;
            // y2 = pt2.y / factor + halfHeight;
            // x3 = pt3.x / factor + halfWidth;
            // y3 = pt3.y / factor + halfHeight;
            // x4 = pt4.x / factor + halfWidth;
            // y4 = pt4.y / factor + halfHeight;
            drawCubicBezier(x1, y1, x2, y2, x3, y3, x4, y4, curve.color);
        }
    }
    // drawLine(0, 184, TFT_WIDTH, 156, c555, 0);
    // drawCubicBezier(0, 0, 0, TFT_HEIGHT, TFT_WIDTH, TFT_HEIGHT, TFT_WIDTH, 0, c555);
}