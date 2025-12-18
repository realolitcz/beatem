#ifndef BEATEM_LOGIC_H
#define BEATEM_LOGIC_H
#include <SDL2/SDL_render.h>

#include "window.h"

struct InputEvent
{
    int key;     // The SDK key code
    Uint32 time; // Time when given key was pressed (SDL_GetTicks)
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
    int action_type;         // Marker of currently conducted action
    int action_timer;        // Time of currently performed action
    int facing_right;        // Is player facing right?
    InputEvent buffer[10];   // History of last 10 inputs
    char current_action[32]; // Name of current action
    int debug_mode;          // Debug overlay toggle
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
void player_jump(Player* player);
void handle_gravity(Player* player);
void push_input(Player* player, int key, Uint32 time);
void check_combos(Player* player, const Level* current_level,Uint32 current_time);
void clear_buffer(Player* player);

#endif //BEATEM_LOGIC_H