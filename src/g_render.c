#include <stdio.h>

#include "g_globals.h"
#include "g_render.h"

#define PIXEL(x, y) (y * G_SCREEN_WIDTH) + x

typedef struct s_shader { 
    struct { int r, g, b, a; } color; 
    double scale; 
} t_shader;

typedef struct s_texture_map{
    double x, y;
    double y_offset;
    double y_step;
} t_texture_map;

static void draw_pixel(int* pixel_buffer, int x, int y, t_shader shader, 
                       t_texture_map* tex_map, t_loaded_texture* tex_ptr);

static void draw_vline(int* pixel_buffer, int x, int y1, int y2, t_shader shader, 
                       t_texture_map* texmap, t_loaded_texture* tex_ptr);

static t_texture_map gen_texmap(t_loaded_texture* tex_ptr, vec2 ray, t_rayface facing, 
                                bool is_vertical, double wall_height, double y_offset);

static t_shader shade(t_material* mat, double dist, bool is_wall, bool is_vertical);


uint32_t rgba(int r, int g, int b, int a){
    // x & 255 limit to 255
    return ((a & 255) << 24) + ((b & 255) << 16) + ((g & 255) << 8) + (r & 255);
}

t_rayhit raycast(vec2 sender, vec2_i index, double angle, 
                 t_level* level, t_material** mat_register){
    t_raysubit subhit_h; t_raysubit subhit_v;
    subhit_h.hit = false; subhit_v.hit = false;
    subhit_h.ray = sender; subhit_v.ray = sender;
    subhit_h.dist = G_MAX_DIST; subhit_v.dist = G_MAX_DIST;

    t_material* mat_h; t_material* mat_v;

    vec2 ray1 = sender; vec2 ray2 = sender;
    double dist_v = G_MAX_DIST; double dist_h = G_MAX_DIST;

    double _tan = tan(angle);
    double invtan = 1 / _tan;

    t_rayface facing = {
        angle > PI      &&  angle < 2*PI,
        angle > 0       &&  angle < PI,
        angle > PI/2    &&  angle < 3*PI/2,
        angle > 3*PI/2  ||  angle < PI/2
    };

    // Vertical ray check
    for(int depth = 0; depth<G_RAY_DEPTH; depth++){

        // Set ray vec2 to next vertical line
        ray1.x = facing.right ?
            (index.x+1 + depth) * G_BLOCK_SIZE:
            (index.x   - depth) * G_BLOCK_SIZE;
        ray1.y = sender.y + _tan * (ray1.x - sender.x);
        
        // Map index coords
        int x = (ray1.x / G_BLOCK_SIZE) - facing.left;
        int y = (ray1.y / G_BLOCK_SIZE);

        int surface = level_get_surface(x, y, level, mat_register);

        if(surface & MAT_SURF_WALL){
            mat_v = level_get_mat(x, y, level, mat_register);
            dist_v = vec_length(ray1.x - sender.x, ray1.y - sender.y);
            break;
        };

        if((surface & MAT_SURF_SPOS) && !subhit_v.hit){
            subhit_v.hit = true;
            subhit_v.dist = vec_length(ray1.x - sender.x, ray1.y - sender.y);
            subhit_v.ray = ray1;
            subhit_v.material = level_get_mat(x, y, level, mat_register);
            subhit_v.is_vertical = true;
        }
    };

    // Horizontal ray check
    for(int depth = 0; depth<G_RAY_DEPTH; depth++){

        // Set ray vec2 to next horizontal line
        ray2.y = facing.down ?
            (index.y+1 + depth) * G_BLOCK_SIZE:
            (index.y   - depth) * G_BLOCK_SIZE;
        ray2.x = sender.x + invtan * (ray2.y - sender.y);

        // Map index coords
        int x = ray2.x / G_BLOCK_SIZE;
        int y = ray2.y / G_BLOCK_SIZE - facing.up;

        int surface = level_get_surface(x, y, level, mat_register);

        if(surface & MAT_SURF_WALL){
            mat_h = level_get_mat(x, y, level, mat_register);
            dist_h = vec_length(ray2.x - sender.x, ray2.y - sender.y);
            break;
        }

        if((surface & MAT_SURF_SPOS) && !subhit_h.hit){
            subhit_h.hit = true;
            subhit_h.dist = vec_length(ray2.x - sender.x, ray2.y - sender.y);
            subhit_h.ray = ray2;
            subhit_h.material = level_get_mat(x, y, level, mat_register);
            subhit_h.is_vertical = false;
        }
    }

    t_raysubit subhit = subhit_v.dist < subhit_h.dist ? subhit_v: subhit_h;
    subhit.facing = facing;

    if(dist_v < dist_h){
        // Beam hits horizontal first
        
        if(dist_v < subhit.dist)
            subhit.hit = false;
        
        // dist, ray, material, is_vertical
        return (t_rayhit) { dist_v, ray1, mat_v, true, facing, subhit };
    } else{
        // Beam hits horizontal first

        if(dist_h < subhit.dist)
            subhit.hit = false;

        // dist, ray, material, is_vertical
        return (t_rayhit) { dist_h, ray2, mat_h, false, facing, subhit };
    }
}

void draw_walls(int* pixel_buffer, int x, int y1, int y2, int wall_height, t_rayhit hit){

    // Set calculate rgb values
    t_shader shader = shade(hit.material, hit.dist, true, hit.is_vertical);

    // Calculate texture mappings
    double y_offset = wall_height > G_SCREEN_HEIGHT ? (wall_height - G_SCREEN_HEIGHT) / 2: 0;
    t_loaded_texture* tex_ptr = hit.is_vertical ? hit.material->tex_ptr1: hit.material->tex_ptr2;
    t_texture_map texmap = gen_texmap(tex_ptr, hit.ray, hit.facing, hit.is_vertical, wall_height, 
                                      y_offset);
    draw_vline(pixel_buffer, x, y1, y2, shader, &texmap, tex_ptr);

    // Same but for second wall
    if(!(hit.material->surface_type & MAT_SURF_WALL2)) return;
    y_offset = fabs(MIN(y1-wall_height, 0));
    tex_ptr = hit.is_vertical ? hit.material->tex_ptr3: hit.material->tex_ptr4;
    texmap = gen_texmap(tex_ptr, hit.ray, hit.facing, hit.is_vertical, wall_height, y_offset);
    draw_vline(pixel_buffer, x, MAX(y1-wall_height, 0), y1, shader, &texmap, tex_ptr);
}

void draw_subhit(int* pixel_buffer, int x, int y1, t_raysubit subhit, double wall_height){
    if(!(subhit.material->surface_type & MAT_SURF_WALL2)) return;

    t_shader shader = shade(subhit.material, subhit.dist, true, subhit.is_vertical);

    double y_offset = fabs(MIN(y1-wall_height, 0));
    t_loaded_texture* tex_ptr = subhit.is_vertical ? subhit.material->tex_ptr3: subhit.material->tex_ptr4;
    t_texture_map texmap = gen_texmap(tex_ptr, subhit.ray, subhit.facing, subhit.is_vertical, 
                                      wall_height, y_offset);
    draw_vline(pixel_buffer, x, MAX(y1-wall_height, 0), y1, shader, &texmap, tex_ptr);
}

void draw_foundation(int* pixel_buffer, int x, int y1, int y2, vec2 sender, double angle, 
                     double s_angle, t_level* level, t_material** material_register){

    double _cos = cos(angle);
    double _cos_d = cos(s_angle - angle);
    double _sin = sin(angle);
    t_texture_map texmap; int i;  

    for(int y = y1; y<y2; y++){
        double dist = G_BLOCK_SIZE * y2 / ((2*y-y2) * _cos_d);

        double map_x = sender.x + _cos * dist;
        double map_y = sender.y + _sin * dist;

        int i = (int)(map_y/G_BLOCK_SIZE) * level->width + (int)(map_x/G_BLOCK_SIZE);
        clip(&i, 0, level->width*level->height-1);
        t_material* mat = material_register[level->map[i]];
        t_shader shader = shade(mat, dist, false, false);

        if(mat->surface_type & MAT_SURF_FLOOR){
            texmap.x = (int)(mat->tex_ptr1->width  * ((int)map_x % G_BLOCK_SIZE) / G_BLOCK_SIZE);
            texmap.y = (int)(mat->tex_ptr1->height * ((int)map_y % G_BLOCK_SIZE) / G_BLOCK_SIZE);
            draw_pixel(pixel_buffer, x, y, shader, &texmap, mat->tex_ptr1);
        }

        if(mat->surface_type & MAT_SURF_CEIL){
            texmap.x = (int)(mat->tex_ptr2->width  * ((int)map_x % G_BLOCK_SIZE) / G_BLOCK_SIZE);
            texmap.y = (int)(mat->tex_ptr2->height * ((int)map_y % G_BLOCK_SIZE) / G_BLOCK_SIZE);
            draw_pixel(pixel_buffer, x, G_SCREEN_HEIGHT - y - 1, shader, &texmap, mat->tex_ptr2);
        }
    }
}

static void draw_pixel(int* pixel_buffer, int x, int y, t_shader shader, 
                       t_texture_map* tex_map, t_loaded_texture* tex_ptr){
    // Index for pixelbuffer
    int i = (int)tex_map->y * tex_ptr->width + (int)tex_map->x;
    clip(&i, 0, tex_ptr->height * tex_ptr->width-1);  

    int r = tex_ptr->texture[4*i  ] + shader.color.r;
    int g = tex_ptr->texture[4*i+1] + shader.color.g;
    int b = tex_ptr->texture[4*i+2] + shader.color.b;
    int a = tex_ptr->texture[4*i+3] + shader.color.a;

    // Set pixel value in pixelbuffer
    pixel_buffer[PIXEL(x, y)] = rgba(
        clip(&r, 0, 255) * shader.scale, 
        clip(&g, 0, 255) * shader.scale, 
        clip(&b, 0, 255) * shader.scale, 
        a
    );
};

static void draw_vline(int* pixel_buffer, int x, int y1, int y2, t_shader shader, 
                       t_texture_map* texmap, t_loaded_texture* tex_ptr){
    for(int y = y1; y <= y2; y++){
        draw_pixel(pixel_buffer, x, y, shader, texmap, tex_ptr);
        texmap->y += texmap->y_step;
    }
}

static t_texture_map gen_texmap(t_loaded_texture* tex_ptr, vec2 ray, t_rayface facing, 
                                bool is_vertical, double wall_height, double y_offset){
    t_texture_map texmap;
    texmap.x = (int)(tex_ptr->width * ((int)(is_vertical ? ray.y: ray.x) % G_BLOCK_SIZE) / G_BLOCK_SIZE);
    
    if((facing.down && !is_vertical) || (facing.left && is_vertical))
        texmap.x = tex_ptr->width-1 - texmap.x;
    
    texmap.y_step = (double)tex_ptr->height / wall_height;
    texmap.y = y_offset*texmap.y_step;
    
    return texmap;
};

static t_shader shade(t_material* mat, double dist, bool is_wall, bool is_vertical){
    // double fog_effect = 1 - (MIN(dist*dist/G_BLOCK_SIZE, G_FOG_DIST)) / G_FOG_DIST;
    
    t_shader shader = { { 0, 0, 0, 255 }, 1 };
    // shader.scale = fog_effect;
    
    if(is_wall){
        shader.color.r = mat->highlight*is_vertical;
        shader.color.g = mat->highlight*is_vertical;
        shader.color.b = mat->highlight*is_vertical;
    }
    return shader;
};
