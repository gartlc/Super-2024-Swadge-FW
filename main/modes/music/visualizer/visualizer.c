#include "visualizer.h"
#include "vec3.h"
#include "mat3.h"
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
mat3_t camK;
mat3q_t camKFixed;

vec3_t vecOffset;
vec3q_t vecOffsetFixed;

vec3_t vecOffsetCenter;
vec3q_t vecOffsetCenterFixed;

vec3_t vecFlipY;

int64_t numIters = 0;
vec3_t rotEuler;

int allocCurve(curve3_t **array, size_t *count)
// Allocates memory for a new curve3_t to be added, attempting realloc with a tmp pointer first
{   
    // curve3_t *tmp = realloc(*array, (*count + 1) * sizeof(curve3_t));
    // Allocate using faster 8-bit RAM. If we run out, we can use MALLOC_CAP_SPIRAM (larger, but slower)
    curve3_t *tmp = (curve3_t*)heap_caps_realloc(*array, (*count + 1) * sizeof(curve3_t), MALLOC_CAP_8BIT);
    if (!tmp) 
    {
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
    FILE *f = fopen("./main/modes/music/visualizer/shapes/p2.shapes", "r");
    char line[MAXSHAPELINE];
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

        // Convert from cRGB notation to actual index in the palette color enum
        color += b;
        color += g * 6;
        color += r * 36;
        curve->color = (int8_t)color;
        
        // Parse sequential points, each point separated by a space
        // and each component separated by a comma, e.g. 1,2,3 4,5,6
        int numPoints = 0;
        token = strtok_r(NULL, " ", &ptrSpace); // Get first point
        while (token != NULL) 
        {
            vec3_t curvePoint;
            char *ptrComma;

            // Parse x,y,z
            char *subtoken = strtok_r(token, ",", &ptrComma);
            curvePoint.x = (int16_t)strtol(subtoken, NULL, 10);
            subtoken = strtok_r(NULL, ",", &ptrComma);
            curvePoint.y = (int16_t)strtol(subtoken, NULL, 10);
            subtoken = strtok_r(NULL, ",", &ptrComma);
            curvePoint.z = (int16_t)strtol(subtoken, NULL, 10);
            
            // Add parsed points to curve3_t struct and continue to next point
            curve->points[numPoints++] = curvePoint;
            token = strtok_r(NULL, " ", &ptrSpace);
        }
        
        curve->num_points = (int8_t)numPoints;
        countCurves++;
    }

    fclose(f);

    vecOffset = (vec3_t){
        .x = 0,
        .y = 0,
        .z = -40
    };
    vecOffsetFixed = vec3_toFixed(vecOffset, 4);

    vecOffsetCenter = (vec3_t){
        .x = TFT_WIDTH/2,
        .y = TFT_HEIGHT/2,
        .z = 0
    };
    vecOffsetCenterFixed = vec3_toFixed(vecOffsetCenter, 4);

    camK = (mat3_t){
        {
            {40,         0,   TFT_WIDTH/2},
            {0,         40,  TFT_HEIGHT/2},
            {0,          0,             1}
        }
    };
    camKFixed = mat3_toFixed(camK, 4);

    rotEuler = (vec3_t){
        .x = 0,
        .y = 0,
        .z = 0
    };

    vecFlipY = (vec3_t){
        .x = 1,
        .y = -1,
        .z = 1
    };
}
 
static void visualizerMainLoop(int64_t elapsedUs)
{
    buttonEvt_t evt;
    char str[24];
    // while (checkButtonQueueWrapper(&evt))
    // {
    //     if (evt.down)
    //     {
    //         if (evt.button & PB_UP)
    //         {
    //             vecOffsetFixed.y += TO_FIXED(2, 4);
    //         } else if (evt.button & PB_DOWN)
    //         {
    //             vecOffsetFixed.y -= TO_FIXED(2, 4);
    //         } else if (evt.button & PB_LEFT)
    //         {
    //             vecOffsetFixed.x += TO_FIXED(2, 4);
    //         } else if (evt.button & PB_RIGHT)
    //         {
    //             vecOffsetFixed.x -= TO_FIXED(2, 4);
    //         } else if (evt.button & PB_A)
    //         {
    //             vecOffsetFixed.z -= TO_FIXED(1, 4);
    //         } else if (evt.button & PB_B)
    //         {
    //             vecOffsetFixed.z += TO_FIXED(1, 4);
    //         }
    //     }
    // }

    while (checkButtonQueueWrapper(&evt))
    {
        if (evt.down)
        {
            if (evt.button & PB_UP)
            {
                rotEuler.x += 3;
            } else if (evt.button & PB_DOWN)
            {
                rotEuler.x -= 3;
            } else if (evt.button & PB_LEFT)
            {
                rotEuler.y += 3;
            } else if (evt.button & PB_RIGHT)
            {
                rotEuler.y -= 3;
            } else if (evt.button & PB_A)
            {
                rotEuler.z += 3;
            } else if (evt.button & PB_B)
            {
                rotEuler.z -= 3;
            }
        }
    }

    // rotEuler.x += 3;
    rotEuler.y -= 5;
    // rotEuler.z += 3;

    rotEuler = vec3_validateEuler(rotEuler);
    mat3_t rotMat = mat3_fromEuler(rotEuler);

    // Store transformed points in vT
    vec3_t vT[4];

    // Clear screen before every draw call
    clearPxTft();
    for (size_t i = 0; i < countCurves; i++) 
    {
        curve3_t curve = curves[i];
        for (int j = 0; j < curve.num_points; j++) 
        {
            // char str[12];
            // sprintf(str, "%d", curve.points[j].x);
            // puts(str);

            vec3_t vec = curve.points[j];
            vec = vec3_mult(vec, vecFlipY);
            vec = mat3_rotVec(rotMat, vec);
            vec3q_t vecFixed = vec3_toFixed(vec, 4);
            // Scale by 1/8
            vecFixed = vec3q_scale(vecFixed, 1);
            vecFixed = vec3q_add(vecFixed, vecOffsetCenterFixed);

            // vecFixed = vec3q_add(vecFixed, vecOffsetFixed);
            // vecFixed = mat3q_projectVec(camKFixed, vecFixed);

            vT[j] = vec3q_fromFixed(vecFixed, 4); // Convert to int just before drawing
        }
        
        if (curve.num_points == 2) 
        {
            drawLine(vT[0].x, vT[0].y, vT[1].x, vT[1].y, curve.color, 0);
            
        } else if (curve.num_points == 4) 
        { 
            drawCubicBezier(vT[0].x, vT[0].y, vT[1].x, vT[1].y, vT[2].x, vT[2].y, vT[3].x, vT[3].y, curve.color);
        }
        
    }

}

static void visualizerExitMode()
{
    heap_caps_free(curves);
}