#include "mat4.h"
#include <stdlib.h>

/**
 * @brief Constructs a 4x4 matrix filled with zeros
 *
 * @return The initialized matrix
 */
mat4_t mat4_zero(void) {
    mat4_t result = {{{0}}};
    return result;
}

/**
 * @brief Constructs a 4x4 identity matrix
 *
 * @return The initialized matrix
 */
mat4_t mat4_identity(void) {
    mat4_t result = {{{0}}};
    for (int i = 0; i < 4; i++) {
        result.m[i][i] = 1;
    }
    return result;
}


/**
 * @brief Multiplies two matrices and returns the result
 *
 * @param matA The first 4x4 matrix (mat4_t)
 * @param matB The second 4x4 matrix (mat4_t)
 * @return The product of matA * matB
 */
mat4_t mat4_multMat(mat4_t matA, mat4_t matB) {
    mat4_t matC = mat4_zero();
    // Element-wise multiplication
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < 4; k++) {
                matC.m[i][j] += matA.m[i][k] * matB.m[k][j];
            }
        }
    }
    return matC;
}

/**
 * @brief Multiplies a 3D vector by a 4x4 matrix
 *
 * @param mat The 4x4 transformation matrix (mat4_t)
 * @param vec The vector to transform (vec3_t)
 * @return The product of mat * vec
 */
vec3_t mat4_multVec(mat4_t mat, vec3_t vec) {
    // Expand vector to 4 for 4x4 matrix multiplication
    float vec4[4] = {vec.x, vec.y, vec.z, 1.0f};
    float vecTransformed[3] = {0, 0, 0};
    vec3_t vecOut;
    
    // Exclude last row of 4x4 matrix (0, 0, 0, 1)
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 4; j++) {
            vecTransformed[i] += mat.m[i][j] * vec4[j];
        }
    }
    vecOut.x = vecTransformed[0];
    vecOut.y = vecTransformed[1];
    vecOut.z = vecTransformed[2];
    return vecOut;
}

/**
 * @brief Gets the translation component of a matrix
 *
 * @param mat The 4x4 transformation matrix
 * @return The translation component as a vec3_t
 */
vec3_t mat4_getTranslation(mat4_t mat) {
    vec3_t translation = {
        .x = mat.m[0][3],
        .y = mat.m[1][3],
        .z = mat.m[2][3]
    };
    return translation;
}

/**
 * @brief Inverts a 4x4 transformation matrix
 *
 * @param mat The 4x4 transformation matrix to invert
 * @return The inverse of the matrix
 */
mat4_t mat4_invert(mat4_t mat) {
    // TODO: Account for scale in this function
    // Currently only supports rotation/translation
    float matRot3x3[3][3];
    // Get transposed rotation matrix
    for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 3; col++) {
            // Transposed (col/row -> row/col)
            matRot3x3[row][col] = mat.m[col][row];
        }
    }

    vec3_t vecTranslation = mat4_getTranslation(mat);
    vecTranslation = vec3_scale(vecTranslation, -1.0f);

    // Set up inverse translation matrix
    mat4_t matTranslation = mat4_identity();
    matTranslation.m[0][3] = vecTranslation.x;
    matTranslation.m[1][3] = vecTranslation.y;
    matTranslation.m[2][3] = vecTranslation.z;

    // Set up inverse rotation matrix
    mat4_t matRotation = mat4_identity();
    for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 3; col++) {
            // Transposed (col/row -> row/col)
            matRotation.m[row][col] = matRot3x3[row][col];
        }
    }

    // Compose final matrix inverse by translating and then rotating
    mat4_t matInv = mat4_multMat(matRotation, matTranslation);
    return matInv;
}