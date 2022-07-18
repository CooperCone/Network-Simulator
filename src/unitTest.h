#pragma once

#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#define TEST_SUITE(name) int main(int argc, char **argv)
#define TEST(name) if (strcmp(name, argv[1]) == 0)

#define assertZero(a) {\
    if (a != 0) {\
        fprintf(stderr, "Assertion failed: %s = %lld, not 0 ", #a, a);\
        fprintf(stderr, "file %s, line %d\n", __FILE__, __LINE__);\
        exit(1);\
    }\
}

#define assertEq(a, b) {\
    if (a != b) {\
        fprintf(stderr, "Assertion failed: %s = %d, not %d ", #a, a, b);\
        fprintf(stderr, "file %s, line %d\n", __FILE__, __LINE__);\
        exit(1);\
    }\
}

#define assertFloatEq(a, b) {\
    if (abs(a - b) > 0.00001f) {\
        fprintf(stderr, "Assertion failed: %s = %f, not %f ", #a, a, b);\
        fprintf(stderr, "file %s, line %d\n", __FILE__, __LINE__);\
        exit(1);\
    }\
}
