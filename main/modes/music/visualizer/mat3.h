/*! \file mat3.h
 *
 * \section mat3_design Design Philosophy
 *
 * This adds functionality to work with 3x3 matrices such as a rotation matrix or projection matrix (K)
 *
 * \section mat3_usage Usage
 *
 * Each function returns a new 3x3 matrix (mat3_t)
 */

#ifndef _MAT3_H_
#define _MAT3_H_
#include "vec3.h"
#include <stdint.h>

typedef struct {
    int16_t m[3][3];
} mat3_t;

mat3_t mat3_zero(void);
mat3_t mat3_identity(void);
mat3_t mat3_multMat(mat3_t matA, mat3_t matB);
vec3_t mat3_multVec(mat3_t mat, vec3_t vec);
vec3_t mat3_rotVecInv(mat3_t mat, vec3_t vec);
mat3_t mat3_transpose(mat3_t mat);
mat3_t mat3_fromIntrinsics(int16_t focalLength, int16_t resX, int16_t resY);
vec3_t mat3_projectVec(mat3_t K, vec3_t vec);
#endif