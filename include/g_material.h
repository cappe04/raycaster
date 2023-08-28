#ifndef H_G_MATERIAL
#define H_G_MATERIAL

#include <stdbool.h>

#include "g_math.h"

/*
wall
2nd wall
3rd wall
floor
ceil
save position
draw on saved pos
collideable
*/

#define MAT_SURF_WALL           0x01
#define MAT_SURF_WALL2          0x02
// #define MAT_SURF_WALL3          0x04
#define MAT_SURF_FLOOR          0x08
#define MAT_SURF_CEIL           0x10
#define MAT_SURF_SPOS           0x20
#define MAT_SURF_DSPOS          0x40
#define MAT_SURF_COLIDE         0x80

// enum e_mat_surface{
//     MAT_SURF_NONE,
//     MAT_SURF_WALL,
//     MAT_SURF_DOUBLE_WALL,
//     MAT_SURF_FLOOR,
//     MAT_SURF_CEIL,
//     MAT_SURF_BOTH,
// };

typedef struct s_loaded_texture{
    uint8_t* texture;
    int width, height;
} t_loaded_texture;

typedef struct s_material{
    int surface_type;
    int highlight;
    t_loaded_texture* tex_ptr1;
    t_loaded_texture* tex_ptr2;
    t_loaded_texture* tex_ptr3;
    t_loaded_texture* tex_ptr4;
    //t_loaded_texture* tex_ptr_top_ns;
    //t_loaded_texture* tex_ptr_top_ew;
} t_material;

extern void unload_textures();
extern void load_textures();

extern t_material MAT_AIR;
extern t_material MAT_BASIC_WALL;
extern t_material MAT_SECOND_WALL;
extern t_material MAT_BASIC_FLOOR;
extern t_material MAT_BASIC_CEIL;
extern t_material MAT_BASIC_ENTRANCE;

extern t_material* material_register[10];

// extern t_loaded_texture TEX_NULL;
// extern t_loaded_texture TEX_WHITE;
// extern t_loaded_texture TEX_BRICK_WALL;
// extern t_loaded_texture TEX_ROCKY_ROAD;
// extern t_loaded_texture TEX_WOODEN_FLOOR;
// extern t_loaded_texture TEX_METAL_FLOOR;


#endif