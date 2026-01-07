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
#include <stdint.h>
#include "vec3.h"
#include "fixedPTMath.h"

typedef struct {
    int16_t m[3][3];
} mat3_t;

typedef struct {
    int32_t m[3][3];
} mat3q_t;

mat3q_t mat3_toFixed(mat3_t mat, int shift);
mat3_t mat3q_fromFixed(mat3q_t mat, int shift);

mat3_t mat3_fromEuler(vec3_t eul);
mat3_t mat3_zero(void);
mat3q_t mat3q_zero(void);
mat3_t mat3_identity(void);
mat3q_t mat3q_multMat(mat3q_t matA, mat3q_t matB);
vec3q_t mat3q_multVec(mat3q_t mat, vec3q_t vec);
vec3_t mat3_rotVec(mat3_t mat, vec3_t vec);
vec3q_t mat3_rotVecInv(mat3q_t mat, vec3q_t vec);
mat3q_t mat3_transpose(mat3q_t mat);
mat3_t mat3_fromIntrinsics(int16_t focalLength, int16_t resX, int16_t resY);
vec3q_t mat3q_projectVec(mat3q_t K, vec3q_t vec);
#endif