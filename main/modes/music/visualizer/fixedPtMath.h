// Contains macros for working with fixed-point numbers and parsing integers
#ifndef FIXEDPOINT_H
#define FIXEDPOINT_H

#include <stdint.h>
#include <stdlib.h>

// Q24.8 Fixed point (1/256 precision)
#define DECIMAL_BITS 8
#define STR_TO_INT(s) ((int32_t)(atoi(s)))
#define TO_FIXED(x) ((int32_t)((x) << DECIMAL_BITS))  // if x is already integer
#define FROM_FIXED(x) ((x) >> DECIMAL_BITS)

#endif