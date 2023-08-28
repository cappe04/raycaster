#include "g_math.h"

double vec_length(double x, double y){ 
    return sqrt(x*x + y*y); 
}

void limit_angle(double* angle) {
    if(*angle > 2*PI)
        *angle -= 2*PI;
    else if (*angle < 0)
        *angle += 2*PI;
}

int clip(int* value, int lower, int upper){
    *value = MIN(*value, upper);
    *value = MAX(*value, lower);
    return *value;
}