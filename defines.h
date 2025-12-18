#ifndef BEATEM_DEFINES_H
#define BEATEM_DEFINES_H

#define TITLE                  "KNIGHT VS MONSTERS - THE BRAWLER"

// Basic game-screen resolution in px
#define SCREEN_WIDTH           1080
#define SCREEN_HEIGHT          720

// Game shows 12 rows of tiles, independent of game base resolution
#define VISIBLE_ROWS           12
#define TARGET_TILE_SIZE       (SCREEN_HEIGHT / VISIBLE_ROWS)

// Assets properties
#define SOURCE_CHAR_SIZE       8
#define SOURCE_TILE_SIZE       16
#define SHEET_COLUMNS          16

// Text scales also relative to the tile size
#define TARGET_CHAR_SIZE       (TARGET_TILE_SIZE / 5)

// Margins of floor depend on calculated tile size
#define FLOOR_HORIZON          (TARGET_TILE_SIZE * 2)                    // Corresponds to two rows of upper-margin
#define FLOOR_BOTTOM           (SCREEN_HEIGHT - (2 * TARGET_TILE_SIZE))  // Corresponds to two lines of lower-margin
#define FLOOR_LEFT_SIDE        TARGET_TILE_SIZE                          // Corresponds to one column left-margin

// Camera grace zone ratios
#define GRACE_ZONE_START       (SCREEN_WIDTH * 1/4)
#define GRACE_ZONE_END         (SCREEN_WIDTH * 3/4)

// Player constants -> speed scales with size, player also runs faster to keep up
#define PLAYER_SPEED           (TARGET_TILE_SIZE / 8)
#define DASH_DISTANCE          (TARGET_TILE_SIZE / 8 * 30)
#define IDLE_PLAYER            0
#define LIGHT_ATTACK_PLAYER    1
#define HEAVY_ATTACK_PLAYER    2
#define COMBO_FIRST_PLAYER     3
#define COMBO_SECOND_PLAYER    4


// Helpers
#define ROWS_PER_SCREEN        VISIBLE_ROWS
#define COLUMNS_PER_SCREEN     (SCREEN_WIDTH / TARGET_TILE_SIZE)
#define TRUE                   1
#define FALSE                  0
#define BASE_BUFFER_SIZE       128
#define SCREEN_BEGINNING       0

// Physics
#define GRAVITY                0.5f
#define JUMP_FORCE             8.0f
#define Z_GROUND_LEVEL         0.0f
#define NO_Z_VELOCITY          0.0f

// Combat timings
#define ATTACK_LIGHT_FRAMES    15
#define ATTACK_HEAVY_FRAMES    35
#define COMBO_TIMEOUT          500
#define DASH_TIMEOUT           200

#endif //BEATEM_DEFINES_H