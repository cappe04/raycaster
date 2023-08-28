#ifndef H_G_LEVEL
#define H_G_LEVEL

#include <stdbool.h>

#include "g_math.h"
#include "g_material.h"

enum e_trigger_type {
    TRIGGER_TYPE_NONE,
    TRIGGER_PRINT,
    TRIGGER_TYPE_DOOR,
    TRIGGER_TYPE_TIMED_DOOR,
};

typedef struct s_level {
    int* map;
    int* trigger_map;
    int width, height, size;
} t_level;

extern t_level LEVEL_1;

extern t_material* level_get_mat(int x, int y, t_level* level, t_material** mat_register);
extern int level_get_surface(int x, int y, t_level* level, t_material** mat_register);
enum e_trigger_type level_get_trigger(int x, int y, t_level* level);

#endif