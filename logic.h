#ifndef BEATEM_LOGIC_H
#define BEATEM_LOGIC_H
#include <SDL2/SDL_render.h>

#include "window.h"

struct InputEvent
{
    int key;     // The SDK key code
    Uint32 time; // Time when given key was pressed (SDL_GetTicks)
};

// This struct is fully compatible with SDL_Rect format
struct Hitbox
{
    int x;
    int y;
    int h;
    int w;
};

struct Player
{
    SDL_Texture* texture;    // Spritesheet
    int global_x;            // Absolute x position in the level
    int global_y;            // Absolute y position in the level
    float z;                 // Height above "ground level"
    float z_velocity;        // Change-rate of player height
    int screen_x;            // X position within given screen
    int player_speed;        // Current player velocity
    int is_moving;           // Is player moving right now?
    int action_type;         // Marker of currently conducted action
    int action_timer;        // Time of currently performed action
    int facing_right;        // Is player facing right?
    InputEvent buffer[10];   // History of last 10 inputs
    char current_action[32]; // Name of current action
    int debug_mode;          // Debug overlay toggle
    Hitbox attack_box;       // The area of current attack
    int score_multiplier;    // Current attack score multiplayer
    Uint32 last_score_time;  // Time that passed since last hit
    int score;               // Current points for the player
    int hurt_timer;          // Counts down when player takes damage
    int health_points;       // Current HP
    int max_health_points;   // Maximum player HP
};

struct Enemy
{
    int type;             // Type: CHASER or CHARGER
    SDL_Texture* texture; // Spritesheet
    int x;                // Absolute x position in the level
    int y;                // Absolute y position in the level
    int h;                // Height of the enemy entity
    int w;                // Width of the enemy entity
    int is_alive;         // Is enemy still alive?
    int state;            // State e.g. whether is attacking, etc.
    int timer;            // General purpose timer for charging/cooldowns
    int stun_timer;       // Frames remaining in stun
    int hurt_timer;       // Counts down when enemy takes damage
    int facing_right;     // Is enemy facing right?
    int health_points;    // Health points of the enemy entity
    Hitbox attack_box;    // The area of current attack TO the player
    Uint32 last_hit_time; // Time that passed since last hit received
};

struct Level
{
    int width_in_tiles;
    int height_in_tiles;
    char** map_layout;
    //int enemy_count;
    //EnemySpawn* enemies;
};

struct Camera;

void handle_input_event(Player* player, const SDL_Event* event, const Level* current_level);
void handle_player_movement(Player* player, const Level* current_level,const Uint8* currentKeyStates);
void handle_camera_movement(const Player* player, const Level* current_level, Camera* camera);
void player_light_attack(Player* player);
void player_heavy_attack(Player* player);
void handle_attack(Player* player);
void player_take_damage(Player* player, int damage);
void player_jump(Player* player);
void handle_gravity(Player* player);
void push_input(Player* player, int key, Uint32 time);
void check_combos(Player* player, const Level* current_level,Uint32 current_time);
void clear_buffer(Player* player);
void init_enemy(Enemy* enemy, SDL_Texture* texture, int type, int x, int y);
void update_enemies(Enemy* enemies, int count, const Player* player);
void update_hitboxes(Player* player);
void check_if_enemy_hit(Player* player, Enemy* enemy, Uint32 current_time);
void check_if_player_hit(Player* player, const Enemy* enemy);

#endif //BEATEM_LOGIC_H