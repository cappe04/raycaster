#include "g_level.h"

t_material* level_get_mat(int x, int y, t_level* level, t_material** mat_register){
    int i = y * level->width + x; clip(&i, 0, level->size-1);
    return mat_register[level->map[i]];
};

enum e_trigger_type level_get_trigger(int x, int y, t_level* level){
    int i = y * level->width + x; clip(&i, 0, level->size-1);
    return level->trigger_map[i];
};

int level_get_surface(int x, int y, t_level* level, t_material** mat_register){
    return level_get_mat(x, y, level, mat_register)->surface_type;;
};


static int level_1_map[] = {
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,0,1,4,4,4,4,0,0,0,0,0,0,0,0,1,
    1,0,1,4,4,4,4,2,0,0,2,0,0,0,0,1,
    1,0,3,5,1,1,1,2,0,0,2,0,0,1,1,1,
    1,3,3,3,3,3,3,2,0,0,0,0,0,0,0,1,
    1,3,3,3,3,2,3,2,0,0,0,0,0,1,0,1,
    1,3,3,3,3,3,3,0,0,0,0,0,0,0,0,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,	
};

static int level_1_trigger_map[] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	
};

t_level LEVEL_1 = {
    &level_1_map[0],
    &level_1_trigger_map[0],
    16, 8, 16*8
};


