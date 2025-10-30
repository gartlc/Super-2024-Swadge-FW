/*! \file vec3.h
 *
 * \section vec3_design Design Philosophy
 *
 * This adds functionality for working with 3D vectors. Contains similar functionality to vector2d with the addition of a cross product.
 *
 * \section vec3_usage Usage
 *
 * Each function returns a new vector (vec3_t), or in some cases an integer where applicable (magnitude, dot product)
 *
 * \section vec3_example Example
 *
 * \code{.c}
 * vec3_t vecA = {
 *     .x = 0,
 *     .y = 0,
 *     .z = 0
 * };
 * vec3_t vecB = {
 *     .x = 1,
 *     .y = 0,
 *     .z = 1
 * };
 * 
 * // Result: (1, 0, 1)
 * vec3_t vecC = vec3_add(vecA, vecB);
 * \endcode
 */

#ifndef _VECTOR_3D_H_
#define _VECTOR_3D_H_
#include <stdint.h>
#include <stdio.h>
#include "fixedPTMath.h"

// vec3_t is for storing data in world/pixel coordinates
typedef struct {
    int16_t x;
    int16_t y;
    int16_t z;
} vec3_t;

// vec3q_t is for transforming using fixed-point numbers
typedef struct {
    int32_t x;
    int32_t y;
    int32_t z;
} vec3q_t;

vec3q_t vec3_toFixed(vec3_t vec, int shift);
vec3_t vec3q_fromFixed(vec3q_t vec, int shift);

vec3_t vec3_validateEuler(vec3_t vec);
vec3_t vec3_mult(vec3_t vecA, vec3_t vecB);
vec3q_t vec3q_add(vec3q_t vecA, vec3q_t vecB);
vec3q_t vec3q_sub(vec3q_t vecA, vec3q_t vecB);
vec3q_t vec3q_scale(vec3q_t vec, int32_t scalar);
int32_t vec3q_mag(vec3q_t vec);
int32_t vec3q_dot(vec3q_t vecA, vec3q_t vecB);
vec3q_t vec3q_norm(vec3q_t vec);
vec3q_t vec3q_cross(vec3q_t vecA, vec3q_t vecB);
#endif