#ifndef BEATEM_LOGIC_H
#define BEATEM_LOGIC_H
#include <SDL2/SDL.h>
#include "defines.h"

struct InputEvent
{
    int key;     // The SDK key code
    Uint32 time; // Time when given key was pressed (from SDL_GetTicks)
};

struct Hitbox
{
    int x; // X position of the hitbox
    int y; // Y position of the hitbox
    int h; // Height of the hitbox
    int w; // Width of the hitbox
};

enum GameState
{
    STATE_MENU = 0,
    STATE_NAME_INPUT = 1,
    STATE_GAMEPLAY = 2,
    STATE_GAME_OVER = 3,
    STATE_SCORES = 4
};

struct ScoreEntry
{
    char name[MAX_NAME_LENGTH]; // Player name for the score entry
    int score;                  // Achieved Player score
};

struct GameSession
{
    GameState state;                    // Indicator of current state of game
    int current_level_index;            // Numerical index of currently played level
    char player_name[MAX_NAME_LENGTH];  // Name entered by the player
    int menu_selection;                 // 0 -> start, 1-> score, 2-> exit
    ScoreEntry* high_scores;            // Array holding players scores
    int total_scores;                   // How many scores loaded
    int current_page;                   // Current view page
};

struct Player
{
    SDL_Texture* texture;    // Spritesheet
    int global_x;            // Absolute x position in the level
    int global_y;            // Absolute y position in the level
    float z;                 // Height above "ground level"
    float z_velocity;        // Change-rate of player height
    bool is_moving;          // Is player moving right now?
    int action_type;         // Marker of currently conducted action
    int action_timer;        // Time of currently performed action
    bool facing_right;       // Is player facing right?
    InputEvent buffer[10];   // History of last 10 inputs
    int debug_mode;          // Debug overlay toggle
    Hitbox attack_box;       // The area of current attack
    int score_multiplier;    // Current attack score multiplayer
    float multiplier_scale;  // Visual scale for the multiplier (1.0 -> normal, 2.5 -> big pop)
    Uint32 last_score_time;  // Time that passed since last hit
    int score;               // Current points for the player
    int hurt_timer;          // Counts down when player takes damage
    int health_points;       // Current HP
};

struct Enemy
{
    int type;             // Type: CHASER or CHARGER
    SDL_Texture* texture; // Spritesheet
    int x;                // Absolute x position in the level
    int y;                // Absolute y position in the level
    bool is_alive;        // Is enemy still alive?
    int state;            // State e.g. whether is attacking, etc.
    int timer;            // General purpose timer for charging/cooldowns
    int stun_timer;       // Frames remaining in stun
    int hurt_timer;       // Counts down when enemy takes damage
    bool facing_right;    // Is enemy facing right?
    int health_points;    // Health points of the enemy entity
    Uint32 last_hit_time; // Time that passed since last hit received
};

struct Level
{
    int width_in_tiles;
    int height_in_tiles;
    char* map_layout;
    int enemy_count;
    Enemy* enemies;
};

struct TextureAssets;
struct Camera;

void init_game_session(GameSession* game_session);
void init_player(Player* player, SDL_Texture* texture);
void reset_player_state(Player* player);
void prepare_player_for_next_level(Player* player);
void handle_ui_input(GameSession* session, const SDL_Event* event);
void reset_game_to_menu(Level** level, const char* path, const TextureAssets* assets, Player* player, Camera* camera,
                        GameSession* session);
void handle_input_event(Player* player, const SDL_Event* event, const Level* current_level);
void handle_player_movement(Player* player, const Level* current_level,const Uint8* currentKeyStates);
void handle_camera_movement(const Player* player, const Level* current_level, Camera* camera);
void player_try_attack(Player* player, int attack_type);
void handle_attack(Player* player);
void player_take_damage(Player* player, int damage);
void player_jump(Player* player);
void handle_gravity(Player* player);
void push_input(Player* player, int key, Uint32 time);
void check_combos(Player* player, const Level* current_level,Uint32 current_time);
void clear_buffer(Player* player);
void init_enemy(Enemy* enemy, SDL_Texture* texture, int type, int x, int y);
void update_enemies(Enemy* enemies, const Player* player, const Level* current_level);
void update_hitboxes(Player* player);
void check_if_enemy_hit(Player* player, Enemy* enemy, Uint32 current_time);
void check_if_player_hit(Player* player, const Enemy* enemy);
bool check_stage_completion(const Player* player, const Level* level);
void handle_menu_input(GameSession* session, const SDL_Event* event);
void handle_name_input(GameSession* session, const SDL_Event* event);
void handle_game_over_input(GameSession* session, const SDL_Event* event);
void update_gameplay(GameSession *session, Player *player, Level **current_level, Camera *camera,
                     TextureAssets *assets, const char *level_files[], int max_levels, Uint32 current_time);
int compare_scores(const void* a, const void* b);
void save_score(const char* name, int score);
ScoreEntry* load_scores(int* count);
void free_scores(GameSession* session);
void handle_score_input(GameSession* session, const SDL_Event* event);

#endif //BEATEM_LOGIC_H