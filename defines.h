#ifndef BEATEM_DEFINES_H
#define BEATEM_DEFINES_H

#define TITLE                  "KNIGHT VS MONSTERS - THE BRAWLER"

// Basic game-screen resolution in px
#define SCREEN_WIDTH           1600
#define SCREEN_HEIGHT          900

// Game shows 12 rows of tiles, independent of game base resolution
#define VISIBLE_ROWS           12
#define TARGET_TILE_SIZE       (SCREEN_HEIGHT / VISIBLE_ROWS)

// Assets properties
#define SOURCE_CHAR_SIZE       8
#define SOURCE_TILE_SIZE       16
#define SHEET_COLUMNS          16

// Text scales also relative to the tile size
#define TARGET_CHAR_SIZE       (TARGET_TILE_SIZE / 5)

// Knight spritesheet layout (Y-offsets in pixels, assuming 16px tile size)
#define ROW_WALK_OFFSET        0
#define ROW_LIGHT_ATK_OFFSET   16
#define ROW_HEAVY_ATK_OFFSET   32
#define ROW_COMBO1_OFFSET      48
#define ROW_COMBO2_OFFSET      64
#define ROW_HURT_OFFSET        80

// Knight animation frame counts
#define FRAMES_WALK            2
#define FRAMES_LIGHT           4
#define FRAMES_HEAVY           4
#define FRAMES_COMBO1          9
#define FRAMES_COMBO2          9
#define FRAMES_HURT            4
#define HURT_DURATION          20 // 20 frames duration for the hurt state

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
#define PLAYER_MAX_HEALTH      100


// Helpers
#define ROWS_PER_SCREEN        VISIBLE_ROWS
#define COLUMNS_PER_SCREEN     (SCREEN_WIDTH / TARGET_TILE_SIZE)
#define TRUE                   1
#define FALSE                  0
#define BASE_BUFFER_SIZE       128
#define SCREEN_BEGINNING       0
#define TIMER_ZERO             0

// Physics
#define GRAVITY                0.5f
#define JUMP_FORCE             8.0f
#define Z_GROUND_LEVEL         0.0f
#define NO_Z_VELOCITY          0.0f

// Combat properties
#define ATTACK_LIGHT_FRAMES    15
#define ATTACK_HEAVY_FRAMES    35
#define COMBO1_FRAMES          45
#define COMBO2_FRAMES          105
#define COMBO_TIMEOUT          500
#define DASH_TIMEOUT           200
#define ATTACK_TIME_OFFSET     1000 // In order to prevent hitting enemy like a machine-gun
#define BASE_DAMAGE_MULTIPLIER 10
#define MULTIPLIER_TIMEOUT     2000
#define BASE_SCORE_MULTIPLIER  100

// Attack hitbox properties
#define LIGHT_ATTACK_WIDTH     (TARGET_TILE_SIZE)
#define LIGHT_ATTACK_HEIGHT    (TARGET_TILE_SIZE * 1/2)
#define LIGHT_ATTACK_X_OFFSET  (TARGET_TILE_SIZE * 5/6)
#define LIGHT_ATTACK_Y_OFFSET  (TARGET_TILE_SIZE * 1/6)
#define HEAVY_ATTACK_WIDTH     (TARGET_TILE_SIZE * 4/3)
#define HEAVY_ATTACK_HEIGHT    (TARGET_TILE_SIZE * 2/3)
#define HEAVY_ATTACK_X_OFFSET  (TARGET_TILE_SIZE * 2/3)
#define HEAVY_ATTACK_Y_OFFSET  (TARGET_TILE_SIZE * 1/3)
#define COMBO_FIRST_WIDTH      (TARGET_TILE_SIZE * 2)
#define COMBO_FIRST_HEIGHT     (TARGET_TILE_SIZE)
#define COMBO_FIRST_X_OFFSET   (TARGET_TILE_SIZE * 5/6)
#define COMBO_SECOND_WIDTH     (TARGET_TILE_SIZE * 3/2)
#define COMBO_SECOND_HEIGHT    (TARGET_TILE_SIZE * 4/3)
#define COMBO_SECOND_X_OFFSET  (TARGET_TILE_SIZE * 5/6)

// Enemies
#define ENEMY_TYPE_CHASER      0
#define ENEMY_TYPE_CHARGER     1
#define ENEMY_STATE_IDLE       0
#define ENEMY_STATE_MOVING     1
#define ENEMY_STATE_ATTACKING  2
#define ENEMY_STATE_CHARGING   3
#define ENEMY_STATE_STUNNED    4
#define ENEMY_BASE_HP          100
#define ENEMY_CONTACT_DAMAGE   10
#define ENEMY_CHASE_SPEED      (PLAYER_SPEED * 1/2)
#define ENEMY_CHARGE_SPEED     (PLAYER_SPEED * 2)
#define ENEMY_ALIGN_SPEED      (PLAYER_SPEED * 2/5)
#define CHARGE_TRIGGER_RANGE   (TARGET_TILE_SIZE * 4) // Distance to start charging
#define STUN_DURATION          30                     // In frames

// Enemies spritesheet layout (Y-offsets in pixels, assuming 16px tile size)
#define ROW_ENEMY_WALK_OFFSET  0
#define ROW_ENEMY_HURT_OFFSET  16
#define ROW_ENEMY_STUN_OFFSET  32

// Enemies animation frame counts
#define FRAMES_ENEMY_WALK      2
#define FRAMES_ENEMY_HURT      3
#define FRAMES_ENEMY_STUN      3

#endif //BEATEM_DEFINES_H