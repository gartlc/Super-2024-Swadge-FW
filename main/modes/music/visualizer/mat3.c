#include "mat3.h"
#include "vec3.h"
#include <stdlib.h>
#include <stdio.h>
#include "fixedPtMath.h"
#include "trigonometry.h"

#define INT_TO_STR(n) ({ \
    static char str[32]; \
    snprintf(str, sizeof(str), "%d", (n)); \
    str; \
})

// NOTE: Fixed-point operations assume Q28.4 fixed point numbers

/**
 * @brief Converts a 16-bit mat3_t to a 32-bit fixed-point mat3q_t
 *
 * @param mat A mat3_t
 * @param shift Number of bits to shift
 * @return A new mat3q_t
 */
mat3q_t mat3_toFixed(mat3_t mat, int shift) {
    mat3q_t matFixed = {{{0}}};
    for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 3; col++) {
            matFixed.m[row][col] = TO_FIXED(mat.m[row][col], shift);
        }
    }
    return matFixed;
}

/**
 * @brief Converts a 32-bit fixed-point mat3q_t to a 16-bit mat3_t
 *
 * @param mat A mat3q_t
 * @param shift Number of bits to shift
 * @return A new mat3_t
 */
mat3_t mat3q_fromFixed(mat3q_t mat, int shift) {
    mat3_t matInt = {{{0}}};
    for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 3; col++) {
            matInt.m[row][col] = FROM_FIXED(mat.m[row][col], shift);
        }
    }
    return matInt;
}

/**
 * @brief Constructs a 3x3 matrix filled with zeros
 *
 * @return The initialized matrix
 */
mat3_t mat3_zero(void) {
    mat3_t result = {{{0}}};
    return result;
}

/**
 * @brief Makes a rotation matrix from a rotation euler (in degrees)
 *
 * @param eul A rotation euler/vector in xyz order
 * @return A 3x3 rotation matrix
 */
mat3_t mat3_fromEuler(vec3_t eul) {
    // Sin/Cos values are effectively in Q1.10
    // Range of values is (-1024, 1024)
    int32_t sx = getSin1024(eul.x);
    int32_t cx = getCos1024(eul.x);
    int32_t sy = getSin1024(eul.y);
    int32_t cy = getCos1024(eul.y);
    int32_t sz = getSin1024(eul.z);
    int32_t cz = getCos1024(eul.z);

    // Row 1
    int16_t r00 = (int16_t)((cz * cy) >> 10);
    int32_t r01a = (cz * sy * sx) >> 20;
    int32_t r01b = (sz * cx) >> 10;
    int16_t r01 = (int16_t)(r01a - r01b);
    int32_t r02a = (cz * sy * cx) >> 20;
    int32_t r02b = (sz * sx) >> 10;
    int16_t r02 = (int16_t)(r02a + r02b);

    // Row 2
    int16_t r10 = (int16_t)((sz * cy) >> 10);
    int32_t r11a = (sz * sy * sx) >> 20;
    int32_t r11b = (cz * cx) >> 10;
    int16_t r11 = (int16_t)(r11a + r11b);
    int32_t r12a = (sz * sy * cx) >> 20;
    int32_t r12b = (cz * sx) >> 10;
    int16_t r12 = (int16_t)(r12a - r12b);

    // Row 3
    int16_t r20 = (int16_t)(sy * -1);
    int16_t r21 = (int16_t)((cy * sx) >> 10);
    int16_t r22 = (int16_t)((cy * cx) >> 10);

    mat3_t mat = {
        {
            {r00, r01, r02},
            {r10, r11, r12},
            {r20, r21, r22}
        }
    };

    return mat;

}

/**
 * @brief Constructs a 3x3 matrix filled with zeros
 *
 * @return The initialized matrix
 */
mat3q_t mat3q_zero(void) {
    mat3q_t result = {{{0}}};
    return result;
}

/**
 * @brief Constructs a 3x3 identity matrix
 *
 * @return The initialized matrix   
 */
mat3_t mat3_identity(void) {
    mat3_t result = {{{0}}};
    for (int i = 0; i < 3; i++) {
        result.m[i][i] = 1;
    }
    return result;
}

/**
 * @brief Constructs a 3x3 perspectival projection matrix (K) from a set of intrinsics
 *
 * @param focalLength The focal length (in pixels)
 * @param resX Horizontal image resolution
 * @param resY Vertical image resolution
 * @return A 3x3 intrinsic matrix
 */
mat3_t mat3_fromIntrinsics(int16_t focalLength, int16_t resX, int16_t resY) {
    // Assumes square pixels and an optical center at the center of the image
    mat3_t camK = {
        {
            {focalLength, 0, resX/2},
            {0, focalLength, resY/2},
            {0,           0,      1}
        }
    };
    return camK;

}

/**
 * @brief Multiplies two matrices and returns the result
 *
 * @param matA The first 3x3 matrix (mat3_t)
 * @param matB The second 3x3 matrix (mat3_t)
 * @return The product of matA * matB
 */
mat3q_t mat3q_multMat(mat3q_t matA, mat3q_t matB) {
    mat3q_t matC = mat3q_zero();
    // Element-wise multiplication
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            for (int k = 0; k < 3; k++) {
                int64_t dbl = (int64_t)(matA.m[i][k] * matB.m[k][j]);
                matC.m[i][j] += (int32_t)(dbl >> 4);
            }
        }
    }
    return matC;
}

/**
 * @brief Multiplies a 3D vector by a 3x3 matrix
 *
 * @param mat The 3x3 transformation matrix (mat3_t)
 * @param vec The vector to transform (vec3q_t)
 * @return The product of mat * vec
 */
vec3q_t mat3q_multVec(mat3q_t mat, vec3q_t vec) {
    // Get vector components in order to be compatible with the loop
    int32_t vec3[3] = {vec.x, vec.y, vec.z};
    int64_t vecTransformed[3] = {0, 0, 0};
    vec3q_t vecOut;

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            vecTransformed[i] += (int64_t)mat.m[i][j] * vec3[j];
        }
    }
    vecOut.x = (vecTransformed[0] + 8) >> 4;
    vecOut.y = (vecTransformed[1] + 8) >> 4;
    vecOut.z = (vecTransformed[2] + 8) >> 4;
    return vecOut;
}

/**
 * @brief Rotates a vector using an inverted (transposed) 3x3 matrix
 *
 * @param mat The 3x3 rotation matrix (mat3_t)
 * @param vec The vector to transform (vec3q_t)
 * @return The product of mat-1 * vec
 */
vec3q_t mat3q_rotVecInv(mat3q_t mat, vec3q_t vec) {
    // Get vector components in order to be compatible with the loop
    int32_t vec3[3] = {vec.x, vec.y, vec.z};
    int32_t vecTransformed[3] = {0, 0, 0};
    vec3q_t vecOut;

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            // Access mat in column-major order ([j][i])
            // Avoids producing a new transposed matrix
            vecTransformed[i] += (int64_t)mat.m[j][i] * vec3[j];
        }
    }
    vecOut.x = (vecTransformed[0] + 8) >> 4;
    vecOut.y = (vecTransformed[1] + 8) >> 4;
    vecOut.z = (vecTransformed[2] + 8) >> 4;
    return vecOut;
}

vec3_t mat3_rotVec(mat3_t mat, vec3_t vec) {
    // Get vector components in order to be compatible with the loop
    int32_t vec3[3] = {vec.x, vec.y, vec.z};
    int32_t vecTransformed[3] = {0, 0, 0};
    vec3_t vecOut;

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            vecTransformed[i] += (int32_t)((mat.m[i][j] * vec3[j]) >> 10);
        }
    }
    vecOut.x = (int16_t)(vecTransformed[0]);
    vecOut.y = (int16_t)(vecTransformed[1]);
    vecOut.z = (int16_t)(vecTransformed[2]);
    return vecOut;
}

/**
 * @brief Transposes a 3x3 matrix
 *
 * @param mat The 3x3 matrix to transpose
 * @return The transpose of the matrix
 */
mat3q_t mat3q_transpose(mat3q_t mat) {
    // Transposes a 3x3 matrix
    mat3q_t matT = mat3q_zero();
    for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 3; col++) {
            // Transposed (col/row -> row/col)
            matT.m[row][col] = mat.m[row][col];
        }
    }
    return matT;
}

/**
 * @brief Projects a point to the image plane
 *
 * @param K The 3x3 camera intrinsics matrix (K)
 * @param vec The vector to project
 * @return The projected vector in image coordinates, with a zeroed z component
 */
vec3q_t mat3q_projectVec(mat3q_t K, vec3q_t vec){
    // Project point into homogeneous coordinates
    // Divide by Z component to get image coordinates (u, v)
    vec3q_t vecHomog = mat3q_multVec(K, vec);
    if (vecHomog.z == 0)
    {
        vecHomog.z = 1; // Avoid divide-by-zero
    }
        
    int32_t u = (int32_t)(((int64_t)vecHomog.x << 4) / vecHomog.z);
    int32_t v = (int32_t)(((int64_t)vecHomog.y << 4) / vecHomog.z);
    vec3q_t vecImage = {
        .x = u,
        .y = v,
        .z = 0
    };
    return vecImage;
}