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
#define FRAC_BITS_8 8
#include <stdint.h>

typedef struct {
    int16_t x;
    int16_t y;
    int16_t z;
} vec3_t;

vec3_t vec3_add(vec3_t vecA, vec3_t vecB);
vec3_t vec3_sub(vec3_t vecA, vec3_t vecB);
vec3_t vec3_scale(vec3_t vec, int16_t scalar);

int16_t vec3_mag(vec3_t vec);
int16_t vec3_dot(vec3_t vecA, vec3_t vecB);
vec3_t vec3_norm(vec3_t vec);
vec3_t vec3_cross(vec3_t vecA, vec3_t vecB);

#endif