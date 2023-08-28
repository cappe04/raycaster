#include "SDL2/SDL.h"

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#include "g_globals.h"
#include "g_math.h"
#include "g_level.h"
#include "g_render.h"

#define PLAYER_SPEED            180             // Pixels per sec
#define PLAYER_ROTSPEED         PI              // Rads per sec
#define PLAYER_RADIUS           G_BLOCK_SIZE / 2

#define ASSERT(_e, ...) if (!(_e)) { fprintf(stderr, __VA_ARGS__); exit(1); }

struct {
    SDL_Window* window;
    SDL_Texture* texture;
    SDL_Renderer* renderer;

    uint32_t pixel_buffer[G_SCREEN_WIDTH * G_SCREEN_HEIGHT];
    bool running;
    bool debug;
    double ticker;
} state;

struct {
    clock_t t1;
    clock_t t2;
    double delta;
} timer;

struct {
    vec2 pos, dir;
    vec2_i index;
    double angle, pitch;
    SDL_Rect rect;
} player;

void draw_map(){
    for(int y=0; y<LEVEL_1.height; y++){
        for(int x=0; x<LEVEL_1.width; x++){
            if(material_register[LEVEL_1.map[y*LEVEL_1.width+x]]->surface_type == MAT_SURF_WALL)
                SDL_SetRenderDrawColor(state.renderer, 200, 200, 200, 255);
            else if(LEVEL_1.map[y*LEVEL_1.width+x] == 3)
                SDL_SetRenderDrawColor(state.renderer, 0, 0, 150, 255);
            else
                SDL_SetRenderDrawColor(state.renderer, 0, 0, 0, 255);
            SDL_Rect rect = { x*G_BLOCK_SIZE+1, y*G_BLOCK_SIZE+1, G_BLOCK_SIZE-2, G_BLOCK_SIZE-2 };
            SDL_RenderFillRect(state.renderer, &rect);
        }
    }
    // Draw player on 2d map
    SDL_SetRenderDrawColor(state.renderer, 255, 255, 0, 255);
    SDL_RenderFillRect(state.renderer, &player.rect);

    SDL_RenderDrawLine(
        state.renderer,
        player.pos.x - player.dir.y * G_BLOCK_SIZE * 0.7, 
        player.pos.y + player.dir.x * G_BLOCK_SIZE * 0.7,
        player.pos.x + player.dir.y * G_BLOCK_SIZE * 0.7,
        player.pos.y - player.dir.x * G_BLOCK_SIZE * 0.7
    );
}

void update(){
    double movspeed = PLAYER_SPEED * timer.delta;
    double rotspeed = PLAYER_ROTSPEED * timer.delta;

    int x = (player.pos.x + player.dir.x * PLAYER_RADIUS) / G_BLOCK_SIZE;
    int y = (player.pos.y + player.dir.y * PLAYER_RADIUS) / G_BLOCK_SIZE;
    bool can_walk = true;// !level_is_wall(x, y, &LEVEL_1, material_register);

    enum e_trigger_type trigger = level_get_trigger(player.index.x, player.index.y, &LEVEL_1);
    state.ticker = state.ticker > 1 ? 0: state.ticker + timer.delta;
    if(trigger == TRIGGER_PRINT && state.ticker >= 1){
        printf("TRIGGER_PRINT %d" "\n", timer.t1);
    }

    const uint8_t* keystate = SDL_GetKeyboardState(NULL);
    if(keystate[SDL_SCANCODE_W] && can_walk){
        player.pos.y += player.dir.y * movspeed;
        player.pos.x += player.dir.x * movspeed;
    }
    if(keystate[SDL_SCANCODE_S]){
        player.pos.x += player.dir.x * -movspeed;
        player.pos.y += player.dir.y * -movspeed;
    }
    if(keystate[SDL_SCANCODE_D]){
        player.pos.x +=  player.dir.y * -movspeed;
        player.pos.y += -player.dir.x * -movspeed;
    }
    if(keystate[SDL_SCANCODE_A]){
        player.pos.x +=  player.dir.y * movspeed;
        player.pos.y += -player.dir.x * movspeed;
    }

    if(keystate[SDL_SCANCODE_LEFT]){
        player.angle -= rotspeed;
    }
    if(keystate[SDL_SCANCODE_RIGHT]){
        player.angle += rotspeed;
    }

    if(keystate[SDL_SCANCODE_UP]){
        player.pitch += 10*timer.delta;
    }
    if(keystate[SDL_SCANCODE_DOWN]){
        player.pitch -= 10*timer.delta;
    }

    // TEMP DEBUG
    if(keystate[SDL_SCANCODE_SPACE]){
        state.debug = true;
    } else{
        state.debug = false;
    }
    
    player.index.x = (int)player.pos.x / G_BLOCK_SIZE;
    player.index.y = (int)player.pos.y / G_BLOCK_SIZE;
    // Updating rect drawing rect
    player.rect.x = player.pos.x - 10;
    player.rect.y = player.pos.y - 10;
    // Limit player angles
    limit_angle(&player.angle);
    // Updating direction normal vector
    player.dir.x = cos(player.angle);
    player.dir.y = sin(player.angle);

    if(state.debug) printf("%f\n", player.angle);
}

void render(){
    // Draw 2d map
    draw_map();

    for(int i = 0; i<G_RAY_RES; i++){
        double angle = player.angle - G_FOV/2*DEGREE + i*DEGREE*G_FOV/G_RAY_RES;
        limit_angle(&angle);

        t_rayhit hit = raycast(player.pos, player.index, angle, &LEVEL_1, material_register);

        // draw 2d map rays
        if(hit.subhit.hit){
            SDL_SetRenderDrawColor(state.renderer, 
                255*(!hit.subhit.is_vertical), 
                255*hit.subhit.is_vertical, 
                0, 255);
            SDL_RenderDrawLine(state.renderer, player.pos.x, player.pos.y, 
                               hit.subhit.ray.x, hit.subhit.ray.y);
        } else {
            SDL_SetRenderDrawColor(
                state.renderer, 
                255*(!hit.is_vertical), 
                255*hit.is_vertical, 0, 255);
            SDL_RenderDrawLine(state.renderer, player.pos.x, player.pos.y, hit.ray.x, hit.ray.y);
        }

        // Calculate wall height
        double wall_height = G_SCREEN_HEIGHT*G_BLOCK_SIZE / (hit.dist * cos(player.angle - angle));
        // Calculate line positions
        int x = (int)G_SCREEN_WIDTH/G_RAY_RES*i;
        int y1 = MAX((int)(G_SCREEN_HEIGHT / 2 - (wall_height/2)), 0); 
        int y2 = MIN((int)G_SCREEN_HEIGHT / 2 + (wall_height/2), G_SCREEN_HEIGHT - 1);

        // Draw surfaces
        draw_walls(&state.pixel_buffer[0], x, y1, y2, wall_height, hit);

        if(hit.subhit.hit){
            double sub_wall_height = G_SCREEN_HEIGHT*G_BLOCK_SIZE / (hit.subhit.dist * cos(player.angle - angle));
            int sub_y1 = MAX((int)(G_SCREEN_HEIGHT / 2 - (sub_wall_height/2)), 0); 
            draw_subhit(&state.pixel_buffer[0], x, sub_y1, hit.subhit, sub_wall_height);
        }
        draw_foundation(&state.pixel_buffer[0], x, y2+1, G_SCREEN_HEIGHT, player.pos, angle, 
                        player.angle, &LEVEL_1, material_register);
    }
}

void init(){
    ASSERT(
        !SDL_Init(SDL_INIT_VIDEO), 
        "SDL failed to initialize: %s\n", 
        SDL_GetError()
    );

    state.window = SDL_CreateWindow(
        G_SCREEN_TITLE,             // Title
        SDL_WINDOWPOS_CENTERED,     // X window pos
        SDL_WINDOWPOS_CENTERED,     // Y window pos
        1280, 720,                  // Width and height
        0                           // Flags
    ); 
    ASSERT(
        state.window, "failed to create SDL window: %s\n", SDL_GetError()
    );

    state.renderer = SDL_CreateRenderer(state.window, -1, SDL_RENDERER_ACCELERATED);
    ASSERT(
        state.renderer, "failed to create SDL renderer: %s\n", SDL_GetError()
    );

    // INGEN ANING
    state.texture = SDL_CreateTexture(
        state.renderer,
        SDL_PIXELFORMAT_ABGR8888,
        SDL_TEXTUREACCESS_STREAMING,
        G_SCREEN_WIDTH,
        G_SCREEN_HEIGHT
    );
    ASSERT(
        state.texture, "failed to create SDL texture: %s\n", SDL_GetError()
    );
}

int main(int argc, char* argv[]){

    init();
    
    // load_textures();
    
    state.running = true;
    state.debug = false;
    state.ticker = 0;

    SDL_Rect dstrect = { 1280-G_SCREEN_WIDTH*2, 0, G_SCREEN_WIDTH*2, G_SCREEN_HEIGHT*2 };

    player.pos = (vec2) { 2*G_BLOCK_SIZE+G_BLOCK_SIZE/2, 5*G_BLOCK_SIZE+G_BLOCK_SIZE/2 };
    player.angle = PI;
    player.dir = (vec2) { cos(player.angle), sin(player.angle) };
    player.rect = (SDL_Rect) { player.pos.x, player.pos.y, 20, 20 };
    player.index = (vec2_i){ (int)player.pos.x / G_BLOCK_SIZE, (int)player.pos.y / G_BLOCK_SIZE, };
    player.pitch = 0;
    
    timer.t1 = clock();

    while (state.running){

        timer.t2 = timer.t1;
        timer.t1 = clock();
        timer.delta = (double)(timer.t1 - timer.t2) / CLOCKS_PER_SEC;

        SDL_Event event;
        while (SDL_PollEvent(&event)){
            switch (event.type)
            {
            case SDL_QUIT:
                state.running = false;
                break;
            
            default:
                break;
            }
        }

        update();

        SDL_SetRenderDrawColor(state.renderer, 80, 80, 80, 255);
        SDL_RenderClear(state.renderer);
        memset(state.pixel_buffer, 0, sizeof(state.pixel_buffer));

        render();
        
        SDL_UpdateTexture(state.texture, NULL, state.pixel_buffer, G_SCREEN_WIDTH * 4);
        SDL_RenderCopy(state.renderer, state.texture, NULL, &dstrect);
        SDL_RenderPresent(state.renderer);
    }

    // unload_textures();

    SDL_DestroyTexture(state.texture);
    SDL_DestroyRenderer(state.renderer);
    SDL_DestroyWindow(state.window);
    SDL_Quit();

    return 0;
}