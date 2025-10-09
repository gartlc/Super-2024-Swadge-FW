/*! \file mat4.h
 *
 * \section mat4_design Design Philosophy
 *
 * This adds functionality to work with 4x4 matrices
 *
 * \section mat4_usage Usage
 *
 * Each function returns a new 4x4 matrix (mat4_t)
 */

#ifndef _MAT4_H_
#define _MAT4_H_
#include "vec3.h"
#include <stdint.h>

typedef struct {
    int16_t m[4][4];
} mat4_t;

mat4_t mat4_zero(void);
mat4_t mat4_identity(void);
mat4_t mat4_multMat(mat4_t matA, mat4_t matB);
vec3_t mat4_multVec(mat4_t mat, vec3_t vec);
mat4_t mat4_invert(mat4_t mat);
mat4_t mat4_transpose(mat4_t mat);
vec3_t mat4_getTranslation(mat4_t mat);
#endif