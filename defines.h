#ifndef BEATEM_DEFINES_H
#define BEATEM_DEFINES_H

constexpr const char* TITLE          {"KNIGHT VS MONSTERS - THE BRAWLER"};

// Basic game-screen resolution in px
constexpr int SCREEN_WIDTH           {1600};
constexpr int SCREEN_HEIGHT          {900};

// Game shows 12 rows of tiles, independent of game base resolution
constexpr int VISIBLE_ROWS           {12};
constexpr int TARGET_TILE_SIZE       {SCREEN_HEIGHT / VISIBLE_ROWS};

// Assets properties
constexpr int SOURCE_CHAR_SIZE       {8};
constexpr int SOURCE_TILE_SIZE       {16};
constexpr int SHEET_COLUMNS          {16};

// Text scales also relative to the tile size
constexpr int TARGET_CHAR_SIZE       {TARGET_TILE_SIZE / 5};

// Knight spritesheet layout (Y-offsets in pixels, assuming 16px tile size)
constexpr int ROW_WALK_OFFSET        {0};
constexpr int ROW_LIGHT_ATK_OFFSET   {16};
constexpr int ROW_HEAVY_ATK_OFFSET   {32};
constexpr int ROW_COMBO1_OFFSET      {48};
constexpr int ROW_COMBO2_OFFSET      {64};
constexpr int ROW_HURT_OFFSET        {80};

// Knight animation frame counts
constexpr int FRAMES_WALK            {2};
constexpr int FRAMES_LIGHT           {4};
constexpr int FRAMES_HEAVY           {4};
constexpr int FRAMES_COMBO1          {9};
constexpr int FRAMES_COMBO2          {9};
constexpr int FRAMES_HURT            {4};
constexpr int HURT_DURATION          {20};

// Margins of floor depend on calculated tile size
constexpr int FLOOR_HORIZON          {TARGET_TILE_SIZE * 2};                 // Corresponds to two rows of upper-margin
constexpr int FLOOR_BOTTOM           {SCREEN_HEIGHT - 2 * TARGET_TILE_SIZE}; // Corresponds to two lines of lower-margin
constexpr int FLOOR_LEFT_SIDE        {TARGET_TILE_SIZE};                     // Corresponds to one column left-margin

// Camera grace zone ratios
constexpr int GRACE_ZONE_START       {SCREEN_WIDTH * 1 / 4};
constexpr int GRACE_ZONE_END         {SCREEN_WIDTH * 3 / 4};

// Player constants -> speed scales with size, player also runs faster to keep up
constexpr int PLAYER_SPEED           {TARGET_TILE_SIZE / 8};
constexpr int DASH_DISTANCE          {TARGET_TILE_SIZE / 8 * 30};
constexpr int IDLE_PLAYER            {0};
constexpr int LIGHT_ATTACK_PLAYER    {1};
constexpr int HEAVY_ATTACK_PLAYER    {2};
constexpr int COMBO_FIRST_PLAYER     {3};
constexpr int COMBO_SECOND_PLAYER    {4};
constexpr int PLAYER_MAX_HEALTH      {100};
constexpr int PLAYER_PRIVACY_ZONE    {TARGET_TILE_SIZE / 2};

// Helpers
constexpr int ROWS_PER_SCREEN        {VISIBLE_ROWS};
constexpr int COLUMNS_PER_SCREEN     {SCREEN_WIDTH / TARGET_TILE_SIZE};
constexpr int BASE_BUFFER_SIZE       {128};
constexpr int SCREEN_BEGINNING       {0};
constexpr int TIMER_ZERO             {0};
constexpr float INITIAL_SCALE        {1.0f};
constexpr float INITIAL_SCALE_DECR   {0.05f};

// Physics
constexpr float GRAVITY              {0.5f};
constexpr float JUMP_FORCE           {8.0f};
constexpr float Z_GROUND_LEVEL       {0.0f};
constexpr float NO_Z_VELOCITY        {0.0f};

// Combat properties
constexpr int ATTACK_LIGHT_FRAMES    {15};
constexpr int ATTACK_HEAVY_FRAMES    {35};
constexpr int COMBO1_FRAMES          {45};
constexpr int COMBO2_FRAMES          {105};
constexpr int COMBO_TIMEOUT          {500};
constexpr int DASH_TIMEOUT           {200};
constexpr int ATTACK_TIME_OFFSET     {1000}; // Prevents machine-gun hits
constexpr int MULTIPLIER_TIMEOUT     {2000};

constexpr int BASE_DAMAGE_MULTIPLIER {10};
constexpr int BASE_SCORE_MULTIPLIER  {100};

// Attack hitbox properties
constexpr int LIGHT_ATTACK_WIDTH     {TARGET_TILE_SIZE};
constexpr int LIGHT_ATTACK_HEIGHT    {TARGET_TILE_SIZE * 1 / 2};
constexpr int LIGHT_ATTACK_X_OFFSET  {TARGET_TILE_SIZE * 5 / 6};
constexpr int LIGHT_ATTACK_Y_OFFSET  {TARGET_TILE_SIZE * 1 / 6};

constexpr int HEAVY_ATTACK_WIDTH     {TARGET_TILE_SIZE * 4 / 3};
constexpr int HEAVY_ATTACK_HEIGHT    {TARGET_TILE_SIZE * 2 / 3};
constexpr int HEAVY_ATTACK_X_OFFSET  {TARGET_TILE_SIZE * 2 / 3};
constexpr int HEAVY_ATTACK_Y_OFFSET  {TARGET_TILE_SIZE * 1 / 3};

constexpr int COMBO_FIRST_WIDTH      {TARGET_TILE_SIZE * 2};
constexpr int COMBO_FIRST_HEIGHT     {TARGET_TILE_SIZE};
constexpr int COMBO_FIRST_X_OFFSET   {TARGET_TILE_SIZE * 5 / 6};

constexpr int COMBO_SECOND_WIDTH     {TARGET_TILE_SIZE * 3 / 2};
constexpr int COMBO_SECOND_HEIGHT    {TARGET_TILE_SIZE * 4 / 3};
constexpr int COMBO_SECOND_X_OFFSET  {TARGET_TILE_SIZE * 5 / 6};

// Enemies
constexpr int ENEMY_TYPE_CHASER      {0};
constexpr int ENEMY_TYPE_CHARGER     {1};
constexpr int ENEMY_STATE_IDLE       {0};
constexpr int ENEMY_STATE_MOVING     {1};
constexpr int ENEMY_STATE_ATTACKING  {2};
constexpr int ENEMY_STATE_CHARGING   {3};
constexpr int ENEMY_STATE_STUNNED    {4};
constexpr int ENEMY_BASE_HP          {100};
constexpr int ENEMY_CONTACT_DAMAGE   {10};
constexpr int ENEMY_CHASE_SPEED      {PLAYER_SPEED * 1 / 2};
constexpr int ENEMY_CHARGE_SPEED     {PLAYER_SPEED * 2};
constexpr int ENEMY_ALIGN_SPEED      {PLAYER_SPEED * 2 / 5};
constexpr int CHARGE_TRIGGER_RANGE   {TARGET_TILE_SIZE * 4}; // Distance to start charging
constexpr int STUN_DURATION          {30};                   // In frames

// Enemies spritesheet layout (Y-offsets in pixels, assuming 16px tile size)
constexpr int ROW_ENEMY_WALK_OFFSET  {0};
constexpr int ROW_ENEMY_HURT_OFFSET  {16};
constexpr int ROW_ENEMY_STUN_OFFSET  {32};

// Enemies animation frame counts
constexpr int FRAMES_ENEMY_WALK      {2};
constexpr int FRAMES_ENEMY_HURT      {3};
constexpr int FRAMES_ENEMY_STUN      {3};

#endif //BEATEM_DEFINES_H