#ifndef H_G_RENDER
#define H_G_RENDER

#include <stdbool.h>
#include <stdint.h>

#include "g_math.h"
#include "g_material.h"
#include "g_level.h"

typedef struct s_rayface{
    bool up; bool down; bool left; bool right;
} t_rayface;

typedef struct s_raysubhit{
    bool hit;
    double dist; 
    vec2 ray; 
    t_material* material;
    bool is_vertical;
    t_rayface facing;
} t_raysubit;

typedef struct s_rayhit {
    double dist;
    vec2 ray;
    t_material* material;
    bool is_vertical;
    t_rayface facing;
    t_raysubit subhit;
} t_rayhit;

extern uint32_t rgba(int r, int g, int b, int a);

extern t_rayhit raycast(vec2 sender, vec2_i index, double angle, t_level* level, 
                        t_material** materials);

extern void draw_walls(int* pixel_buffer, int x, int y1, int y2, int wall_height, t_rayhit hit);

extern void draw_subhit(int* pixel_buffer, int x, int y1, t_raysubit subhit, double wall_height);

extern void draw_foundation(int* pixel_buffer, int x, int y1, int y2, vec2 sender, double angle, 
                            double s_angle, t_level* level, t_material** material_register);


#endif