/*! \file curve3.h
 *
 * \section curve3_design Design Philosophy
 *
 * This allows for easy access to a custom curve object, either a line segment or cubic bezier curve
 */

#ifndef _CURVE3_H_
#define _CURVE3_H_
#include "vec3.h"
#include <stdint.h>

typedef struct {
    int8_t spline_id;   // 0 or 1
    int8_t color;      // e.g. c123\0
    vec3_t points[4];   // Either 2 or 4 points (2 for a segment, 4 for a bezier)
    int8_t num_points;
} curve3_t;

int allocCurve(curve3_t **array, size_t *count);

#endif