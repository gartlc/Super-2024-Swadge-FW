#include "mat3.h"
#include "vec3.h"
#include <stdlib.h>
#include <stdio.h>
#include "fixedPtMath.h"

#define INT_TO_STR(n) ({ \
    static char str[32]; \
    snprintf(str, sizeof(str), "%d", (n)); \
    str; \
})

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
 * @brief Constructs a 3x3 identity matrix
 *
 * @return The initialized matrix   
 */
mat3_t mat3_identity(void) {
    mat3_t result = {{{0}}};
    for (int i = 0; i < 2; i++) {
        result.m[i][i] = 1;
    }
    return result;
}

/**
 * @brief Constructs a 3x3 perspectival projection matrix (K) from a set of intrinsics
 *
 * @param matA The first 3x3 matrix (mat3_t)
 * @param matB The second 3x3 matrix (mat3_t)
 * @return The product of matA * matB
 */
mat3_t mat3_fromIntrinsics(int16_t focalLength, int16_t resX, int16_t resY) {
    // Convert res/focal length to fixed point
    int32_t resXFixed = TO_FIXED(resX);
    int32_t resYFixed = TO_FIXED(resY);
    int32_t focalLengthXFixed = TO_FIXED(focalLength);
    int32_t aspectRatio = resYFixed / resXFixed;

    // Calculate Y focal length using image aspect ratio (assume square pixels)
    int16_t focalLengthY = FROM_FIXED(focalLengthXFixed * aspectRatio);
    int16_t focalLengthX = FROM_FIXED(focalLengthXFixed);
    mat3_t camK = {
        {
            {focalLengthX, 0, resX},
            {0, focalLengthY, resY},
            {0,            0,    1}
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
mat3_t mat3_multMat(mat3_t matA, mat3_t matB) {
    mat3_t matC = mat3_zero();
    // Element-wise multiplication
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            for (int k = 0; k < 3; k++) {
                matC.m[i][j] += matA.m[i][k] * matB.m[k][j];
            }
        }
    }
    return matC;
}

/**
 * @brief Multiplies a 3D vector by a 3x3 matrix
 *
 * @param mat The 3x3 transformation matrix (mat3_t)
 * @param vec The vector to transform (vec3_t)
 * @return The product of mat * vec
 */
vec3_t mat3_multVec(mat3_t mat, vec3_t vec) {
    // Get vector components in order to be compatible with the loop
    int16_t vec3[3] = {vec.x, vec.y, vec.z};
    int16_t vecTransformed[3] = {0, 0, 0};
    vec3_t vecOut;

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            vecTransformed[i] += mat.m[i][j] * vec3[j];
        }
    }
    vecOut.x = vecTransformed[0];
    vecOut.y = vecTransformed[1];
    vecOut.z = vecTransformed[2];
    return vecOut;
}

/**
 * @brief Rotates a vector using an inverted (transposed) 3x3 matrix
 *
 * @param mat The 3x3 rotation matrix (mat3_t)
 * @param vec The vector to transform (vec3_t)
 * @return The product of mat-1 * vec
 */
vec3_t mat3_rotVecInv(mat3_t mat, vec3_t vec) {
    // Get vector components in order to be compatible with the loop
    int16_t vec3[3] = {vec.x, vec.y, vec.z};
    int16_t vecTransformed[3] = {0, 0, 0};
    vec3_t vecOut;

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            // Access mat in column-major order ([j][i])
            // Avoids producing a new transposed matrix
            vecTransformed[i] += mat.m[j][i] * vec3[j];
        }
    }
    vecOut.x = vecTransformed[0];
    vecOut.y = vecTransformed[1];
    vecOut.z = vecTransformed[2];
    return vecOut;
}

/**
 * @brief Transposes a 3x3 matrix
 *
 * @param mat The 3x3 matrix to transpose
 * @return The transpose of the matrix
 */
mat3_t mat3_transpose(mat3_t mat) {
    // Transposes a 3x3 matrix
    mat3_t matT = mat3_identity();
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
vec3_t mat3_projectVec(mat3_t K, vec3_t vec){
    // Project point into homogeneous coordinates
    // Divide by Z component to get image coordinates (u, v)
    vec3_t vecH = mat3_multVec(K, vec);
    // puts("VEC XYZ");
    // puts(INT_TO_STR(vecH.x));
    // puts(INT_TO_STR(vecH.y));
    // puts(INT_TO_STR(vecH.z));
    int16_t u = vecH.x / vecH.z;
    int16_t v = vecH.y / vecH.z;
    vec3_t vecImage = {
        .x = u,
        .y = v,
        .z = 0
    };
    // puts("VEC UV");
    // puts(INT_TO_STR(u));
    // puts(INT_TO_STR(v));
    return vecImage;
}