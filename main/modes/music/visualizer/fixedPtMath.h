// Contains macros for working with fixed-point numbers and parsing integers
#ifndef FIXEDPOINT_H
#define FIXEDPOINT_H

#include <stdint.h>
#include <stdlib.h>

#define STR_TO_INT(s) ((int32_t)(atoi(s)))
#define TO_FIXED(x, b) ((int32_t)((x) << b))  // if x is already integer
#define FROM_FIXED(x, b) ((x) >> b)

#endif