#ifndef H_G_MATH
#define H_G_MATH

#include <stdint.h>
#include <math.h>

#define PI                      3.14159265359
#define DEGREE                  0.0174532925

#define MAX(a, b) (a > b ? a: b)
#define MIN(a, b) (a < b ? a: b)

typedef struct s_vec2 { double x, y; } vec2;
typedef struct s_vec2_i { int32_t x, y; } vec2_i;

extern double vec_length(double x, double y);
extern void limit_angle(double* angle);
extern int clip(int* value, int lower, int upper);

#endif