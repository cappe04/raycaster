#ifndef H_G_GLOBALS
#define H_G_GLOBALS

#define G_SCREEN_WIDTH              384
#define G_SCREEN_HEIGHT             216
#define G_SCREEN_TITLE              "Raycaster"

#define G_BLOCK_SIZE                50
#define G_RAY_DEPTH                 16                  // Must be atleast length of longest sight-line
#define G_RAY_RES                   G_SCREEN_WIDTH      // Same as SCREEN_WIDTH, if not logic must be implemented
#define G_FOV                       60
#define G_ANGLE_THRESHOLD           0.01
#define G_MAX_DIST                  1000000
#define G_FOG_DIST                  350

#endif