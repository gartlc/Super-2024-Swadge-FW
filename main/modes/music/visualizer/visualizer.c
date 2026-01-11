#include "visualizer.h"
#include "vec3.h"
#include "mat3.h"
#include "curve3.h"
#include <stdlib.h>
#include <stdbool.h>
#include <esp_log.h>
#include <esp_timer.h>
#include "cnfs.h"
#include "hdw-imu.h"

#define MAXSHAPELINE 64
#define ROTSPEEDMAX 10
#define ROTSPEEDMIN -10

#include <stdint.h>
#define Q10_SCALE 1024
#define Q10_MAX 2047
#define Q10_MIN -2048

// Convert from float to Q1.10 fixed point (for working with quaternions)
static inline int16_t float_to_q1_10(float x)
{
    int32_t q = (int32_t)(x * Q10_SCALE + (x >= 0 ? 0.5f : -0.5f));

    if (q > Q10_MAX) q = Q10_MAX;
    if (q < Q10_MIN) q = Q10_MIN;

    return (int16_t)q;
}

// Mode-specific function declarations
void visualizerAudioCallback(uint16_t* samples, uint32_t sampleCnt);
void visualizerButtonCallback(buttonEvt_t* evt);
const char visualizerModeName[] = "Visualizer";
static void visualizerEnterMode(void);
static void visualizerExitMode(void);
static void visualizerMainLoop(int64_t elapsedUs);
bool loadShapesFile(cnfsFileIdx_t fileIdx);
 
swadgeMode_t visualizerMode = {
    .modeName                 = visualizerModeName,
    .wifiMode                 = NO_WIFI,
    .overrideUsb              = false,
    .usesAccelerometer        = true,
    .usesThermometer          = false,
    .overrideSelectBtn        = false,
    .fnEnterMode              = visualizerEnterMode,
    .fnExitMode               = visualizerExitMode,
    .fnMainLoop               = visualizerMainLoop,
    .fnAudioCallback          = visualizerAudioCallback,
    .fnBackgroundDrawCallback = NULL,
    .fnEspNowRecvCb           = NULL,
    .fnEspNowSendCb           = NULL,
    .fnAdvancedUSB            = NULL,
};

// Declare curves and some basic transformation matrices/vectors
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
int16_t rotSpeed = 3;
vec3_t rotEuler;

cnfsFileIdx_t logo;
size_t logoIdx = 0;

bool accelEnabled = true;

// These enum members are auto-generated at compile time *before* this file is compiled
static const cnfsFileIdx_t logos[] = {
    VIZ_VECTOR_U_BIN,
    VIZ_DANCH_BIN,
    VIZ_THEO_BIN,
    VIZ_P_2_BIN
};

static size_t numLogos = 4;

int allocCurve(curve3_t **array, size_t *count)
// Allocates memory for a new curve3_t to be added, attempting realloc with a tmp pointer first
{   
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

static char* mem_fgets(char* dst, int max, const char** src)
{
    // null terminator at end of file
    if (**src == '\0')
        return NULL;

    int i = 0;
    while (i < max - 1 && **src && **src != '\n')
    {
        dst[i++] = *(*src)++;
    }

    if (**src == '\n')
        (*src)++;

    // Add null terminator at end of line
    dst[i] = '\0';
    return dst;
}

bool loadShapesFile(cnfsFileIdx_t fileIdx)
// Loads a custom shapes file provided the corresponding file index from the enum in cnfs_image.h
{
    size_t fileLen;
    char* fileText = (char*)cnfsReadFile(fileIdx, &fileLen, false);
    if (!fileText)
    {
        return false;
    }

    char line[MAXSHAPELINE];
    const char* cursor = fileText;

    // Open and parse custom .shapes file
    // TODO: Do an initial malloc with heap_caps_malloc() instead of only realloc
    while (mem_fgets(line, MAXSHAPELINE, &cursor) != NULL)
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

    free(fileText);
    return true;
}

static void visualizerEnterMode()
{   
    // Load Vector U as default
    logo = logos[logoIdx];
    loadShapesFile(logo);

    vecOffset = (vec3_t){
        .x = 0,
        .y = 0,
        .z = -100 // Translate 100 units away from camera (depth)
    };
    vecOffsetFixed = vec3_toFixed(vecOffset, 4);

    // Translates scene origin to the center of the screen
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
 
static void visualizerExitMode()
{   
    // Clean up 
    heap_caps_free(curves);
    curves = NULL;
}

static void visualizerMainLoop(int64_t elapsedUs)
{
    // Use joystick input to drive out-of-plane rotation
    int32_t phi, r, intensity, joystickAngle;
    if (getTouchJoystick(&phi, &r, &intensity)) {
        joystickAngle = phi - 90;
        if (joystickAngle < 0) {
            joystickAngle += 360;
        }
        joystickAngle -= 360;
        joystickAngle *= -1;
        rotEuler.z = joystickAngle;
    }

    // Use d-pad to change other angles
    // Use AB buttons to cycle logos
    buttonEvt_t evt;
    while (checkButtonQueueWrapper(&evt))
    {   
        if (evt.down)
        {
            if (evt.button & PB_UP)
            {
                rotSpeed += 1;
            } else if (evt.button & PB_DOWN)
            {
                rotSpeed -= 1;            
            } else if (evt.button & PB_LEFT)
            {
                accelEnabled = false;
                rotEuler.z = 0;
            } else if (evt.button & PB_RIGHT)
            {
                accelEnabled = true;
                rotEuler.z = 0;
            } else if (evt.button & PB_A)
            {
                heap_caps_free(curves);
                countCurves = 0;
                curves = NULL;
                logoIdx++;
                if (logoIdx >= numLogos) {
                    logoIdx = 0;
                }
                logo = logos[logoIdx];
                loadShapesFile(logo);
            } else if (evt.button & PB_B)
            {
                heap_caps_free(curves);
                countCurves = 0;
                curves = NULL;
                if (logoIdx == 0) {
                    logoIdx = numLogos - 1;
                } else {
                    logoIdx--;
                }
                logo = logos[logoIdx];
                loadShapesFile(logo);
            }
        }
    }

    if (rotSpeed < ROTSPEEDMIN) {
        rotSpeed = ROTSPEEDMIN;
    } else if (rotSpeed > ROTSPEEDMAX) {
        rotSpeed = ROTSPEEDMAX;
    }
    rotEuler.y -= rotSpeed;

    rotEuler = vec3_validateEuler(rotEuler);
    mat3_t rotMat = mat3_fromEuler(rotEuler);

    if (accelEnabled) {
        // Sample accelerometer
        int16_t a_x, a_y, a_z;
        accelIntegrate();
        if (ESP_OK != accelGetOrientVec(&a_x, &a_y, &a_z))
        {
            a_x = 0;
            a_y = 0;
            a_z = 0;
        }
        
        // Get X/Y/Z axes in world space
        float plusx_out[3] = {1, 0, 0};
        float plusy_out[3] = {0, 1, 0};
        float plusz_out[3] = {0, 0, 1};

        // TODO: Check handedness of accelerometer coordinate frame vs our asssumed frame
        mathRotateVectorByQuaternion(plusy_out, LSM6DSL.fqQuat, plusy_out);
        mathRotateVectorByQuaternion(plusx_out, LSM6DSL.fqQuat, plusx_out);
        mathRotateVectorByQuaternion(plusz_out, LSM6DSL.fqQuat, plusz_out);

        // Manually construct rotation matrix from world-space vectors
        rotMat = mat3_identity();
        rotMat.m[0][0] = float_to_q1_10(plusx_out[0]);
        rotMat.m[1][0] = float_to_q1_10(plusx_out[1]);
        rotMat.m[2][0] = float_to_q1_10(plusx_out[2]);

        rotMat.m[0][1] = float_to_q1_10(plusy_out[0]);
        rotMat.m[1][1] = float_to_q1_10(plusy_out[1]);
        rotMat.m[2][1] = float_to_q1_10(plusy_out[2]);

        rotMat.m[0][2] = float_to_q1_10(plusz_out[0]);
        rotMat.m[1][2] = float_to_q1_10(plusz_out[1]);
        rotMat.m[2][2] = float_to_q1_10(plusz_out[2]);
    }

    // Store transformed points in vT
    vec3_t vT[4];

    // Clear screen before every draw call
    clearPxTft();
    for (size_t i = 0; i < countCurves; i++) 
    {
        curve3_t curve = curves[i];
        // Apply xform to all curve points
        for (int j = 0; j < curve.num_points; j++) 
        {
            vec3_t vec = curve.points[j];
            vec = vec3_mult(vec, vecFlipY);
            vec = mat3_rotVec(rotMat, vec);
            vec3q_t vecFixed = vec3_toFixed(vec, 4);
            // vecFixed = vec3q_scale(vecFixed, 1);
            vecFixed = vec3q_add(vecFixed, vecOffsetCenterFixed);

            // vecFixed = vec3q_add(vecFixed, vecOffsetFixed);
            // TODO: This is old code that projects a vector to the image plane using an intrinsic 3x3 matrix
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

void visualizerAudioCallback(uint16_t* samples, uint32_t sampleCnt) {

}

void visualizerButtonCallback(buttonEvt_t* evt) {

}