#include "vec3.h"

/**
 * @brief Add two vectors and return the resulting vector
 *
 * @param vecA The first vector
 * @param vecB The second vector
 * @return The sum of both vectors
 */
vec3_t vec3_add(vec3_t vecA, vec3_t vecB) {
    vec3_t result = {
        .x = vecA.x + vecB.x,
        .y = vecA.y + vecB.y,
        .z = vecA.z + vecB.z
    };
    return result;
}

/**
 * @brief Subtract two vectors and return the resulting vector. A-B will yield a vector that goes from the tip of B to the tip of A
 *
 * @param vecA The first vector
 * @param vecB The second vector
 * @return The difference of both vectors
 */
vec3_t vec3_sub(vec3_t vecA, vec3_t vecB) {
    vec3_t result = {
        .x = vecA.x - vecB.x,
        .y = vecA.y - vecB.y,
        .z = vecA.z - vecB.z
    };
    return result;
}

/**
 * @brief Calculate the dot product of two vectors
 *
 * @param vecA The first vector
 * @param vecB The second vector
 * @return The dot product of the two vectors (vecA dot vecB)
 */
int16_t vec3_dot(vec3_t vecA, vec3_t vecB) {
    // Calculate dot product using int32_t to avoid overflow given high numbers from multiplication
    int32_t tempSum = 0;
    tempSum += ((int32_t)(vecA.x << FRAC_BITS_8) * (vecB.x << FRAC_BITS_8)) >> FRAC_BITS_8;
    tempSum += ((int32_t)(vecA.y << FRAC_BITS_8) * (vecB.y << FRAC_BITS_8)) >> FRAC_BITS_8;
    tempSum += ((int32_t)(vecA.z << FRAC_BITS_8) * (vecB.z << FRAC_BITS_8)) >> FRAC_BITS_8;
    // Cast to int16_t after we have shifted right
    return (int16_t)tempSum;
}


/**
 * @brief Multiplies a vector by a scalar
 *
 * @param vec The vector
 * @param scalar The scale factor
 * @return The scaled vector
 */
vec3_t vec3_scale(vec3_t vec, int16_t scalar) {
    vec3_t result = {
        .x = vec.x * scalar,
        .y = vec.y * scalar,
        .z = vec.z * scalar,
    };
    return result;
}
