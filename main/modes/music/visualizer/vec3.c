#include "vec3.h"

/**
 * @brief Converts a 16-bit vec3_t to a 32-bit fixed-point vec3q_t
 *
 * @param vec A vec3_t
 * @return A new vec3q_t
 */
vec3q_t vec3_toFixed(vec3_t vec, int shift) {
    vec3q_t vecFixed = {
        .x = TO_FIXED(vec.x, shift),
        .y = TO_FIXED(vec.y, shift),
        .z = TO_FIXED(vec.z, shift)
    };
    return vecFixed;
}

/**
 * @brief Converts a 32-bit fixed-point vec3q_t to a 16-bit vec3_t
 *
 * @param vec A vec3q_t
 * @return A new vec3_t
 */
vec3_t vec3q_fromFixed(vec3q_t vec, int shift) {
    vec3_t vecInt = {
        .x = FROM_FIXED(vec.x, shift),
        .y = FROM_FIXED(vec.y, shift),
        .z = FROM_FIXED(vec.z, shift)
    };
    return vecInt;
}

/**
 * @brief Add two vectors and return the resulting vector
 *
 * @param vecA The first vector
 * @param vecB The second vector
 * @return The sum of both vectors
 */
vec3q_t vec3q_add(vec3q_t vecA, vec3q_t vecB) {
    vec3q_t result = {
        .x = vecA.x + vecB.x,
        .y = vecA.y + vecB.y,
        .z = vecA.z + vecB.z
    };
    return result;
}

/**
 * @brief Element-wise multiplication of two vectors
 *
 * @param vecA The first vector
 * @param vecB The second vector
 * @return The element-wise product of both vectors
 */
vec3_t vec3_mult(vec3_t vecA, vec3_t vecB) {
    vec3_t result = {
        .x = vecA.x * vecB.x,
        .y = vecA.y * vecB.y,
        .z = vecA.z * vecB.z
    };
    return result;
}


vec3_t vec3_validateEuler(vec3_t vec) {
    int16_t vec3[3] = {vec.x, vec.y, vec.z};
    for (int i = 0; i < 3; i++) {
        if (vec3[i] > 359) {
            vec3[i] -= 360;
        } else if (vec3[i] < 0) {
            vec3[i] += 360;
        }
    }

    vec3_t eul = {
        .x = vec3[0],
        .y = vec3[1],
        .z = vec3[2]
    };

    return eul;
}

/**
 * @brief Subtract two vectors and return the resulting vector. A-B will yield a vector that goes from the tip of B to the tip of A
 *
 * @param vecA The first vector
 * @param vecB The second vector
 * @return The difference of both vectors
 */
vec3q_t vec3q_sub(vec3q_t vecA, vec3q_t vecB) {
    vec3q_t result = {
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
int32_t vec3q_dot(vec3q_t vecA, vec3q_t vecB) {
    int32_t sum = 0;
    sum += vecA.x * vecB.x;
    sum += vecA.y * vecB.y;
    sum += vecA.z * vecB.z;
    return sum;
}

/**
 * @brief Multiplies a vector by a scalar
 *
 * @param vec The vector
 * @param scalar The scale factor
 * @return The scaled vector
 */
vec3q_t vec3q_scale(vec3q_t vec, int32_t scalar) {
    vec3q_t result = {
        .x = vec.x * scalar,
        .y = vec.y * scalar,
        .z = vec.z * scalar,
    };
    return result;
}
