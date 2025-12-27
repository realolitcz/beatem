#include <SDL2/SDL.h>
#include "window.h"
#include "logic.h"
#include "defines.h"

// Initializer function for game session struct
void init_game_session(GameSession* game_session)
{

    game_session->state = STATE_MENU;
    game_session->current_level_index = 0;
    strcpy(game_session->player_name, "");
    game_session->menu_selection = 0;
    game_session->high_scores = nullptr;
    game_session->total_scores = 0;
    game_session->current_page = 0;

}


// Function used to initialize Player entity
void init_player(Player* player, SDL_Texture* texture)
{

    player->texture = texture;
    // DRY-ed logic from resetting
    reset_player_state(player);

}


// Function used to reset Player entity (and as a helper to init_player)
void reset_player_state(Player* player)
{

    player->global_x = COLUMNS_PER_SCREEN * TARGET_TILE_SIZE / 2;
    player->global_y = VISIBLE_ROWS * TARGET_TILE_SIZE / 2;
    player->z = Z_GROUND_LEVEL;
    player->z_velocity = NO_Z_VELOCITY;
    player->is_moving = false;
    player->action_type = IDLE_PLAYER;
    player->action_timer = TIMER_ZERO;
    player->facing_right = true;
    clear_buffer(player);
    player->debug_mode = false;
    player->attack_box = {0, 0, 0, 0};
    player->score_multiplier = 1;
    player->multiplier_scale = INITIAL_SCALE;
    player->last_score_time = TIMER_ZERO;
    player->score = 0;
    player->hurt_timer = TIMER_ZERO;
    player->health_points = PLAYER_MAX_HEALTH;

}


// Helper function used to conduct preparatory actions for level change
void prepare_player_for_next_level(Player* player)
{

    // Backup Score
    const int kept_score = player->score;

    // Perform standard reset (clears buffers, position, etc.)
    reset_player_state(player);

    // Restore Score
    player->score = kept_score;

    // Ensure full health for next level
    player->health_points = PLAYER_MAX_HEALTH;

}


// Handler-wrapper function for UI guys
void handle_ui_input(GameSession* session, const SDL_Event* event)
{

    switch (session->state)
    {
        case STATE_MENU:
            handle_menu_input(session, event);
            break;
        case STATE_NAME_INPUT:
            handle_name_input(session, event);
            break;
        case STATE_GAME_OVER:
            handle_game_over_input(session, event);
            break;
        case STATE_SCORES:
            handle_score_input(session, event);
            break;
        default:
            break;
    }

}


// Little helper function to handle resetting game to menu
void reset_game_to_menu(Level** level, const char* path, const TextureAssets* assets, Player* player, Camera* camera,
                        GameSession* session)
{

    change_level(level, path, assets, player, camera, true);
    session->current_level_index = 0;

}


// Function used to handle single click player keyboard events
void handle_input_event(Player* player, const SDL_Event* event, const Level* current_level)
{

    // If key was pressed
    if (event->type == SDL_KEYDOWN && event->key.repeat == 0 )
    {
        const int key {event->key.keysym.sym};
        bool combo_triggered {false};

        if (key == SDLK_F1)
        {
            player->debug_mode = !player->debug_mode;
        }

        // Always push key to the buffer
        if (key == SDLK_k || key == SDLK_l || key == SDLK_a || key == SDLK_d)
        {
            push_input(player, key, SDL_GetTicks());

            const int previous_time {player->action_timer};

            check_combos(player, current_level,SDL_GetTicks());

            if (player->action_timer > previous_time)
            {
                combo_triggered = true;
            }
        }

        // If combo was not triggered we perform normal actions
        if (combo_triggered == false)
        {
            if (key == SDLK_SPACE)
            {
                player_jump(player);
            }
            if (player->action_timer == 0)
            {
                if (key == SDLK_k || key == SDLK_l)
                {
                    // Are we in the air?
                    if (player->z > Z_GROUND_LEVEL)
                    {
                        // Pressing K or L in air triggers specific move
                        player_try_attack(player, AERIAL_ATTACK_PLAYER);
                    }
                    else
                    {
                        // Ground attacks
                        if (key == SDLK_k) player_try_attack(player, LIGHT_ATTACK_PLAYER);
                        if (key == SDLK_l) player_try_attack(player, HEAVY_ATTACK_PLAYER);
                    }
                }
            }
        }
    }

}


// Function to check whether certain tile on the level is walkable
bool is_walkable(const Level* level, const int x, const int y)
{

    // Convert pixel coordinates to map indices
    const int column {x / TARGET_TILE_SIZE};
    const int row {y / TARGET_TILE_SIZE};

    // Safety check (Bounds)
    if (column < 0 || column >= level->width_in_tiles || row < 0 || row >= level->height_in_tiles)
    {
        // Is not walkable
        return false;
    }

    // Get the tile character
    const int index {(row * level->width_in_tiles) + column};
    const char tile {level->map_layout[index]};

    // Return true only for walkable tiles
    // 'F' = Floor, 'E' = Exit. 'W', 'D', 'B' are obstacles.
    return (tile == 'F' || tile == 'E');

}


// Function to handle movement of the player on basis of the current PRESSED AND HOLD key
void handle_player_movement(Player* player, const Level* current_level, const Uint8* currentKeyStates)
{

    player->is_moving = false;

    // Check the "feet" (z = 0) of the player for collision => center X of player, bottom Y of player.
    const int player_center_x {player->global_x + (TARGET_TILE_SIZE / 2)};
    const int player_bottom_y {player->global_y + TARGET_TILE_SIZE - 4};   // -4 gives a tiny bit of leeway
    constexpr int speed = PLAYER_SPEED;

    // Check "Future" position i.e. where we want to walk
    if (currentKeyStates[SDL_SCANCODE_W])
    {
        // If we could go up... (y - speed)
        if (is_walkable(current_level, player_center_x, player_bottom_y - speed))
        {
            player->global_y -= speed;
            player->is_moving = true;
        }
    }
    if (currentKeyStates[SDL_SCANCODE_S])
    {
        // If we could go down... (y + speed)
        if (is_walkable(current_level, player_center_x, player_bottom_y + speed))
        {
            player->global_y += speed;
            player->is_moving = true;
        }
    }
    if (currentKeyStates[SDL_SCANCODE_A])
    {
        // For horizontal, we might want to check the left edge of the sprite, not just center
        const int left_edge {player->global_x - speed + 4}; // +4 padding
        if (is_walkable(current_level, left_edge, player_bottom_y))
        {
            player->global_x -= speed;
            player->facing_right = false;
            player->is_moving = true;
        }
    }
    if (currentKeyStates[SDL_SCANCODE_D])
    {
        // Check right edge
        const int right_edge = player->global_x + TARGET_TILE_SIZE + speed - 4; // -4 padding
        if (is_walkable(current_level, right_edge, player_bottom_y))
        {
            player->global_x += speed;
            player->facing_right = true;
            player->is_moving = true;
        }
    }

}


// Function to change camera placement i.e. the part of visible area basing on current player position
void handle_camera_movement(const Player* player, const Level* current_level, Camera* camera)
{

    const int screen_x = player->global_x - camera->camera_x;

    if (screen_x > GRACE_ZONE_END)
    {
        camera->camera_x = player->global_x - GRACE_ZONE_END;
    }
    if (screen_x < GRACE_ZONE_START)
    {
        camera->camera_x = player->global_x - GRACE_ZONE_START;
    }

    if (camera->camera_x < 0)
    {
        camera->camera_x = 0;
    }
    if (camera->camera_x > current_level->width_in_tiles * TARGET_TILE_SIZE - SCREEN_WIDTH)
    {
        camera->camera_x = current_level->width_in_tiles * TARGET_TILE_SIZE - SCREEN_WIDTH;
    }

}


// Helper function for player attacks
void player_try_attack(Player* player, const int attack_type)
{

    // Cannot attack if already busy
    if (player->action_timer > TIMER_ZERO)
    {
        return;
    }

    // Aerial logic
    if (attack_type == AERIAL_ATTACK_PLAYER && player->z <= Z_GROUND_LEVEL)
    {
        return;
    }
    if (attack_type != AERIAL_ATTACK_PLAYER && player->z > Z_GROUND_LEVEL)
    {
        return;
    }

    // Execution
    player->action_type = attack_type;

    switch (attack_type)
    {
        case LIGHT_ATTACK_PLAYER:
            player->action_timer = ATTACK_LIGHT_FRAMES;
            break;
        case HEAVY_ATTACK_PLAYER:
            player->action_timer = ATTACK_HEAVY_FRAMES;
            break;
        case AERIAL_ATTACK_PLAYER:
            player->action_timer = AERIAL_ATTACK_FRAMES;
            break;
        default:
            ;
    }

}


// Function used to wrap tick-end attack-blockades
void handle_attack(Player* player)
{

    if (player->action_timer > TIMER_ZERO)
    {
        player->action_timer--;
    }
    else
    {
        player->action_type = IDLE_PLAYER;
    }

    if (player->hurt_timer > TIMER_ZERO)
    {
        player->hurt_timer--;
    }

    if (player->multiplier_scale > INITIAL_SCALE)
    {
        player->multiplier_scale -= INITIAL_SCALE_DECR;
        if (player->multiplier_scale < INITIAL_SCALE)
        {
            player->multiplier_scale = INITIAL_SCALE;
        }
    }

    if (player->score_multiplier > 1)
    {
        if (SDL_GetTicks() - player->last_score_time > MULTIPLIER_TIMEOUT)
        {
            player->score_multiplier = 1;             // Reset the count
            player->multiplier_scale = INITIAL_SCALE; // Reset the visual size
        }
    }

    if (player->multiplier_scale > INITIAL_SCALE)
    {
        player->multiplier_scale -= INITIAL_SCALE_DECR;
    }

}


// Function used to handle player damage
void player_take_damage(Player *player, const int damage)
{

    // Ensure we do not hurt player multiple times
    if (player->hurt_timer > TIMER_ZERO)
    {
        return;
    }

    player->hurt_timer = HURT_DURATION;
    player->health_points -= damage;

    if (player->health_points < 0)
    {
        // HANDLE GAME OVER
    }

}


// Function to conduct player jump i.e. add z-axis velocity
void player_jump(Player* player)
{

    if (player->z == Z_GROUND_LEVEL)
    {
        player->z_velocity = JUMP_FORCE;
    }

}


// Function use to wrap tick-end gravity apply
void handle_gravity(Player* player)
{

    if (player->z > Z_GROUND_LEVEL || player->z_velocity != NO_Z_VELOCITY)
    {
        player->z += player->z_velocity;
        player->z_velocity -= GRAVITY;
    }

    if (player->z < Z_GROUND_LEVEL)
    {
        player->z = Z_GROUND_LEVEL;
        player->z_velocity = NO_Z_VELOCITY;
    }

}


// Function to push input key to the combo queue
void push_input(Player* player, const int key, const Uint32 time)
{

    // Add key to the buffer <=> shift all elements down
    for (int i {9}; i > 0; i--)
    {
        player->buffer[i] = player->buffer[i - 1];
    }

    // New input goes to the front
    player->buffer[0].key = key;
    player->buffer[0].time = time;

}


// Function to clear the buffer after conducting certain combo
void clear_buffer(Player* player)
{

    for (int i {0}; i < 10; i++)
    {
        player->buffer[i].key = 0;
        player->buffer[i].time = 0;
    }

}


// Function to check whether certain combo should be executed
void check_combos(Player* player, const Level* current_level, const Uint32 current_time)
{

    // Safety Timeout
    if (current_time - player->buffer[0].time > COMBO_TIMEOUT)
    {
        return;
    }

    // Combo 1: Triple Light (K - K - K)
    if (player->buffer[0].key == SDLK_k && player->buffer[1].key == SDLK_k && player->buffer[2].key == SDLK_k)
    {
        if (player->buffer[0].time - player->buffer[2].time < COMBO_TIMEOUT)
        {
            player->action_type = COMBO_FIRST_PLAYER;
            player->action_timer = COMBO1_FRAMES;
            clear_buffer(player);
            return;
        }
    }

    // Combo 2: Light - Heavy - Light (K - L - K)
    if (player->buffer[0].key == SDLK_k && player->buffer[1].key == SDLK_l && player->buffer[2].key == SDLK_k)
    {
        if (player->buffer[0].time - player->buffer[1].time < COMBO_TIMEOUT)
        {
            player->action_type = COMBO_SECOND_PLAYER;
            player->action_timer = COMBO2_FRAMES;
            clear_buffer(player);
            return;
        }
    }

    // Check for Double Tap (Same key twice, fast)
    if (player->buffer[0].key == player->buffer[1].key &&
        player->buffer[0].time - player->buffer[1].time < DASH_TIMEOUT)
    {
        int direction {0};
        // Right
        if (player->buffer[0].key == SDLK_d)
        {
            direction = 1;
        }
        // Left
        else if (player->buffer[0].key == SDLK_a)
        {
            direction = -1;
        }
        if (direction != 0)
        {
            const int feet_y = player->global_y + TARGET_TILE_SIZE - 4;
            constexpr int step_size = TARGET_TILE_SIZE / 2;

            for (int step {0}; step < DASH_DISTANCE; step += step_size)
            {
                const int next_x = player->global_x + (direction * step_size);

                // Collision Edge: Right side (+Width) or Left side (+0) + Padding
                const int collision_edge = (direction > 0)
                    ? (next_x + TARGET_TILE_SIZE - 4)
                    : (next_x + 4);

                // Boundary Checks
                const bool out_of_bounds = (direction > 0)
                    ? (next_x >= current_level->width_in_tiles * TARGET_TILE_SIZE - 2 * TARGET_TILE_SIZE)
                    : (next_x <= FLOOR_LEFT_SIDE);

                if (out_of_bounds || !is_walkable(current_level, collision_edge, feet_y))
                {
                    break; // Hit wall or border
                }

                // Move
                player->global_x = next_x;
            }
            clear_buffer(player);
            return;
        }
    }
}


// Function to initialize enemy entity on a given position
void init_enemy(Enemy* enemy, SDL_Texture* texture, const int type, const int x, const int y)
{

    enemy->type = type;                   // Type: CHASER or CHARGER
    enemy->texture = texture;             // Spritesheet
    enemy->x = x;                         // Absolute x position in the level
    enemy->y = y;                         // Absolute y position in the level
    enemy->is_alive = true;               // Is enemy still alive?
    enemy->state = ENEMY_STATE_MOVING;    // State e.g. whether is attacking, etc.
    enemy->timer = TIMER_ZERO;            // General purpose timer for charging/cooldowns
    enemy->stun_timer = TIMER_ZERO;       // Frames remaining in stun
    enemy->hurt_timer = TIMER_ZERO;       // Counts down when enemy takes damage
    enemy->facing_right = true;           // Is enemy facing right?
    enemy->health_points = ENEMY_BASE_HP; // Health points of the enemy entity
    enemy->last_hit_time = TIMER_ZERO;    // Time that passed since last hit received

}


// Function used to constantly handle enemies lifetime
void update_enemies(Enemy* enemies, const Player* player, const Level* current_level)
{

    for (int i {0}; i < current_level->enemy_count; i++)
    {
        Enemy* enemy = &enemies[i];

        // Skip dead enemy
        if (!enemy->is_alive)
        {
            continue;
        }

        // Timers
        if (enemy->stun_timer > TIMER_ZERO)
        {
            // Decrease stun timer of stunned enemy and SKIP
            enemy->stun_timer--;
            continue;
        }
        if (enemy->hurt_timer > TIMER_ZERO)
        {
            // Decrease hurt timer of hurt enemy
            enemy->hurt_timer--;
        }

        // Calculate distance to the Player
        const int dx {player->global_x - enemy->x};
        const int dy {player->global_y - enemy->y};
        enemy->facing_right = (dx > 0);

        // Define "Feet" (z = 0) for collision (Bottom Center of the sprite)
        const int feet_x {enemy->x + (TARGET_TILE_SIZE / 2)};
        const int feet_y {enemy->y + TARGET_TILE_SIZE - 2};

        if (enemy->type == ENEMY_TYPE_CHASER)
        {
            if (abs(dx) > PLAYER_PRIVACY_ZONE)
            {
                const int move_x {(dx > 0 ? ENEMY_CHASE_SPEED : -ENEMY_CHASE_SPEED)};
                // Check if the NEXT X position is walkable
                if (is_walkable(current_level, feet_x + move_x, feet_y))
                {
                    enemy->x += move_x;
                }
            }
            if (abs(dy) > PLAYER_PRIVACY_ZONE)
            {
                const int move_y {(dy > 0 ? ENEMY_CHASE_SPEED : -ENEMY_CHASE_SPEED)};
                // Check if NEXT Y position is walkable
                if (is_walkable(current_level, feet_x, feet_y + move_y))
                {
                    enemy->y += move_y;
                }
            }
        }
        else if (enemy->type == ENEMY_TYPE_CHARGER)
        {
            switch (enemy->state)
            {
                case ENEMY_STATE_MOVING:
                {
                    // Align Y
                    if (abs(dy) > TARGET_TILE_SIZE)
                    {
                        const int move_y {(dy > 0 ? ENEMY_ALIGN_SPEED : -ENEMY_ALIGN_SPEED)};
                        if (is_walkable(current_level, feet_x, feet_y + move_y))
                        {
                            enemy->y += move_y;
                        }
                    }
                    // Align X (Maintenance distance)
                    const int target_x {player->global_x + (dx > 0 ? -CHARGE_TRIGGER_RANGE : CHARGE_TRIGGER_RANGE)};
                    const int move_x_diff {target_x - enemy->x};
                    if (abs(move_x_diff) > TARGET_TILE_SIZE)
                    {
                        const int move_x {(move_x_diff > 0 ? ENEMY_ALIGN_SPEED : -ENEMY_ALIGN_SPEED)};
                        if (is_walkable(current_level, feet_x + move_x, feet_y))
                        {
                            enemy->x += move_x;
                        }
                    }
                    // Trigger Charge
                    if (abs(dy) < TARGET_TILE_SIZE * 2 && enemy->timer == TIMER_ZERO)
                    {
                        enemy->state = ENEMY_STATE_CHARGING;
                        enemy->timer = BASE_TIMER;
                    }
                    break;
                }
                case ENEMY_STATE_CHARGING:
                {
                    const int charge_speed {(enemy->facing_right ? ENEMY_CHARGE_SPEED : -ENEMY_CHARGE_SPEED)};
                    // If hitting a wall, STOP charging immediately
                    if (!is_walkable(current_level, feet_x + charge_speed, feet_y))
                    {
                        enemy->state = ENEMY_STATE_MOVING;
                        enemy->timer = BASE_TIMER * 2; // Long cooldown penalty
                    }
                    else
                    {
                        enemy->x += charge_speed;
                        enemy->timer--;
                        if (enemy->timer <= TIMER_ZERO)
                        {
                            enemy->state = ENEMY_STATE_MOVING;
                            enemy->timer = BASE_TIMER * 2;
                        }
                    }
                    break;
                }
                default:
                    ;
            }

            if (enemy->state != ENEMY_STATE_CHARGING && enemy->timer > TIMER_ZERO)
            {
                enemy->timer--;
            }
        }
    }
}


// Function to properly handle size of hitboxes of player attacks
void update_hitboxes(Player* player)
{

    // Hitbox initially is not present
    player->attack_box =
    {
        .x = 0,
        .y = 0,
        .h = 0,
        .w = 0
    };

    // Create hitbox <=> player is currently not idle
    if (player->action_type != IDLE_PLAYER)
    {
        switch (player->action_type)
        {
            case LIGHT_ATTACK_PLAYER:
                player->attack_box.w = LIGHT_ATTACK_WIDTH;
                player->attack_box.h = LIGHT_ATTACK_HEIGHT;
                player->attack_box.x = player->facing_right ?
                                       player->global_x + LIGHT_ATTACK_X_OFFSET :
                                       player->global_x - player->attack_box.w + (TARGET_TILE_SIZE - LIGHT_ATTACK_X_OFFSET);
                player->attack_box.y = player->global_y + LIGHT_ATTACK_Y_OFFSET;
                break;
            case HEAVY_ATTACK_PLAYER:
                player->attack_box.w = HEAVY_ATTACK_WIDTH;
                player->attack_box.h = HEAVY_ATTACK_HEIGHT;
                player->attack_box.x = player->facing_right ?
                                       player->global_x + HEAVY_ATTACK_X_OFFSET :
                                       player->global_x - player->attack_box.w + (TARGET_TILE_SIZE - HEAVY_ATTACK_X_OFFSET);
                player->attack_box.y = player->global_y - HEAVY_ATTACK_Y_OFFSET;
                break;
            case COMBO_FIRST_PLAYER:
                player->attack_box.w = COMBO_FIRST_WIDTH;
                player->attack_box.h = COMBO_FIRST_HEIGHT;
                player->attack_box.x = player->facing_right ?
                                       player->global_x + COMBO_FIRST_X_OFFSET :
                                       player->global_x - player->attack_box.w + (TARGET_TILE_SIZE - COMBO_FIRST_X_OFFSET);
                player->attack_box.y = player->global_y;
                break;
            case COMBO_SECOND_PLAYER:
                player->attack_box.w = COMBO_FIRST_WIDTH;
                player->attack_box.h = COMBO_SECOND_HEIGHT;
                player->attack_box.x = player->facing_right ?
                                       player->global_x + COMBO_SECOND_X_OFFSET :
                                       player->global_x - player->attack_box.w + (TARGET_TILE_SIZE - COMBO_SECOND_X_OFFSET);
                player->attack_box.y = player->global_y;
                break;
            case AERIAL_ATTACK_PLAYER:
                player->attack_box.w = AERIAL_ATTACK_WIDTH;
                player->attack_box.h = AERIAL_ATTACK_HEIGHT;
                player->attack_box.x = player->facing_right ?
                                       player->global_x + AERIAL_ATTACK_X_OFFSET :
                                       player->global_x - player->attack_box.w + (TARGET_TILE_SIZE - AERIAL_ATTACK_X_OFFSET);
                player->attack_box.y = player->global_y;
                break;
            default:
                break;
        }
    }

}


// Function to check whether PLAYER has hit the ENEMY and handle it properly
void check_if_enemy_hit(Player* player, Enemy* enemy, const Uint32 current_time)
{

    // Terminate if enemy is not alive or player is not attacking
    if (enemy->is_alive == false || player->attack_box.w == 0)
    {
        return;
    }

    // Check whether an enemy is within the attack hitbox (AABB)
    const int collision =
    (
        player->attack_box.x < enemy->x + ENEMY_SIZE &&
        player->attack_box.x + player->attack_box.w > enemy->x &&
        player->attack_box.y < enemy->y + ENEMY_SIZE &&
        player->attack_box.y + player->attack_box.h > enemy->y
    );

    if (collision)
    {

        if (current_time - enemy->last_hit_time > ATTACK_TIME_OFFSET)
        {
            enemy->last_hit_time = current_time;
            enemy->health_points -= player->action_type * BASE_DAMAGE_MULTIPLIER;
            enemy->stun_timer = STUN_DURATION;
            enemy->hurt_timer = 20;
            if (enemy->health_points <= 0)
            {
                enemy->is_alive = false;
            }
            if (current_time - player->last_score_time < MULTIPLIER_TIMEOUT)
            {
                player->score_multiplier++;
                player->multiplier_scale = INITIAL_SCALE * 4.5f;
            }
            else
            {
                player->score_multiplier = 1;
                player->multiplier_scale = INITIAL_SCALE;
            }
            player->last_score_time = current_time;
            player->score += BASE_SCORE_MULTIPLIER * player->score_multiplier;
        }

    }

}


// Function to check whether ENEMY has hit the PLAYER and handle it correctly
void check_if_player_hit(Player* player, const Enemy* enemy)
{

    if (!enemy->is_alive || player->hurt_timer > TIMER_ZERO || enemy->stun_timer > TIMER_ZERO || player->z > Z_GROUND_LEVEL)
    {
        return;
    }

    // Player body hitbox
    const SDL_Rect player_rect
    {
        .x = player->global_x,
        .y = player->global_y,
        .w = TARGET_TILE_SIZE,
        .h = TARGET_TILE_SIZE
    };

    // Enemy body hitbox
    const SDL_Rect enemy_rect
    {
        .x = enemy->x,
        .y = enemy->y,
        .w = ENEMY_SIZE,
        .h = ENEMY_SIZE
    };

    // AABB collision check
    const int collision =
    (
        player_rect.x < enemy_rect.x + enemy_rect.w &&
        player_rect.x + player_rect.w > enemy_rect.x &&
        player_rect.y < enemy_rect.y + enemy_rect.h &&
        player_rect.y + player_rect.h > enemy_rect.y
    );

    if (collision)
    {
        player_take_damage(player, ENEMY_CONTACT_DAMAGE);
    }

}


// Function used to check whether level-end conditions has been met
bool check_stage_completion(const Player* player, const Level* level)
{

    // Check if all enemies are dead
    for (int i {0}; i < level->enemy_count; i++)
    {
        if (level->enemies[i].is_alive)
        {
            // Found a survivor, stage not done
            return false;
        }
    }

    // Calculate Player's center tile
    const int center_x {player->global_x + TARGET_TILE_SIZE / 2};
    const int center_y {player->global_y + TARGET_TILE_SIZE / 2};
    const int col {center_x / TARGET_TILE_SIZE};
    const int row {center_y / TARGET_TILE_SIZE};
    const int index {row * level->width_in_tiles + col};

    // Validation: index bounds
    if (index < 0 || index >= level->width_in_tiles * level->height_in_tiles)
    {
        return false;
    }

    // Check for Exit Tile
    if (level->map_layout[index] == 'E')
    {
        return true;
    }

    return false;

}


// Function used to handle main menu input
void handle_menu_input(GameSession *session, const SDL_Event *event)
{

    if (event->type == SDL_KEYDOWN)
    {
        switch (event->key.keysym.sym)
        {
            case SDLK_UP:
                session->menu_selection--;
                if (session->menu_selection < 0)
                {
                    session->menu_selection = 2;
                }
                break;
            case SDLK_DOWN:
                session->menu_selection++;
                if (session->menu_selection > 2)
                {
                    session->menu_selection = 0;
                }
                break;
            case SDLK_RETURN:
            case SDLK_SPACE:
                // Start
                if (session->menu_selection == 0)
                {
                    // Go to name entry first
                    session->state = STATE_GAMEPLAY;
                }
                // Scores
                else if (session->menu_selection == 1)
                {
                    session->high_scores = load_scores(&session->total_scores);
                    session->state = STATE_SCORES;
                }
                // Exit
                else if (session->menu_selection == 2)
                {
                    SDL_Event quit_event;
                    quit_event.type = SDL_QUIT;
                    SDL_PushEvent(&quit_event);
                }
                break;
            default:
                ;
        }
    }

}


// Function used to handle Player's name input
void handle_name_input(GameSession *session, const SDL_Event *event)
{

    if (event->type == SDL_KEYDOWN)
    {
        const int key = event->key.keysym.sym;
        size_t len = strlen(session->player_name);

        // Backspace handling
        if (key == SDLK_BACKSPACE && len > 0)
        {
            session->player_name[--len] = '\0';
        }
        // Enter to confirm
        else if (key == SDLK_RETURN && len > 0)
        {
            // Start the game
            session->state = STATE_SCORES;
        }
        // Handle a-z letters
        else if (key >= SDLK_a && key <= SDLK_z && len < MAX_NAME_LENGTH - 1)
        {
            session->player_name[len++] = static_cast<char>(key);
            session->player_name[len + 1] = '\0';
        }
    }

}


// Function used to handle losing => prompt menu or continue
void handle_game_over_input(GameSession *session, const SDL_Event *event)
{

    if (event->type == SDL_KEYDOWN)
    {
        if (event->key.keysym.sym == SDLK_y)
        {
            // Go back to the menu
            session->state = STATE_MENU;
        }
        else if (event->key.keysym.sym == SDLK_n)
        {
            session->state = STATE_GAMEPLAY;
        }
    }

}

void update_gameplay(GameSession *session, Player *player, Level **current_level,
                     Camera *camera, TextureAssets *assets,
                     const char *level_files[], int max_levels, Uint32 current_time)
{
    // 1. Handle Input & Movement
    const Uint8* currentKeyStates = SDL_GetKeyboardState(nullptr);
    handle_player_movement(player, *current_level, currentKeyStates);
    handle_camera_movement(player, *current_level, camera);

    // 2. Update Combat Logic
    update_hitboxes(player);

    // Note: Dereference *current_level to get the actual pointer
    update_enemies((*current_level)->enemies, player, *current_level);

    for (int i = 0; i < (*current_level)->enemy_count; i++)
    {
        check_if_enemy_hit(player, &(*current_level)->enemies[i], current_time);
        check_if_player_hit(player, &(*current_level)->enemies[i]);
    }

    // 3. Update Physics
    handle_gravity(player);
    handle_attack(player);

    // 4. Check Game Over
    if (player->health_points <= 0)
    {
        session->state = STATE_GAME_OVER;
    }

    // 5. Check Stage Completion
    if (check_stage_completion(player, *current_level))
    {
        session->current_level_index++;

        if (session->current_level_index < max_levels)
        {
            // Load Next Stage (Keep stats)
            // Pass the pointer to the pointer
            change_level(current_level, level_files[session->current_level_index], assets, player, camera, false);
        }
        else
        {
            // Game Finished -> Victory Screen
            session->state = STATE_GAME_OVER;
            session->current_level_index = 0;
        }
    }
}



// Helper function used as comparator for qsort (descending order)
int compare_scores(const void* a, const void* b)
{

    const auto* entryA {static_cast<const ScoreEntry *>(a)};
    const auto* entryB {static_cast<const ScoreEntry *>(b)};
    // Higher score comes first
    return (entryB->score - entryA->score);

}


// Helper function used to save given score to scores file
void save_score(const char* name, const int score)
{

    FILE* file {fopen(SCORES_FILE, "a")};

    if (file == nullptr)
    {
        fprintf(stderr, "Could not open scores file\n");
        exit(1);
    }

    fprintf(file, "%s %d\n", name, score);
    fclose(file);

}


// Function used to load scores from the file
ScoreEntry* load_scores(int* count)
{

    *count = 0;
    FILE* file {fopen(SCORES_FILE, "r")};
    if (file == nullptr)
    {
        // No scores yet
        return nullptr;
    }

    // Count lines to determine memory size
    int character {0};
    int lines {0};
    while(!feof(file))
    {
        character = fgetc(file);
        if(character == '\n')
        {
            lines++;
        }
    }

    // Allocate exact memory needed
    if (lines == 0)
    {
        fclose(file);
        return nullptr;
    }

    auto* scores {new ScoreEntry[lines + 1]}; // +1 safety padding

    // Read Data
    rewind(file);
    int i {0};
    // Limit buffer width to prevent overflow
    while (i < lines && fscanf(file, "%31s %d", scores[i].name, &scores[i].score) == 2)
    {
        i++;
    }
    fclose(file);
    *count = i;

    // Sorting
    qsort(scores, *count, sizeof(ScoreEntry), compare_scores);

    return scores;

}


void free_scores(GameSession* session)
{
    if (session->high_scores != nullptr)
    {
        delete[] session->high_scores;
        session->high_scores = nullptr;
    }
    session->total_scores = 0;
    session->current_page = 0;
}

void handle_score_input(GameSession* session, const SDL_Event* event)
{
    if (event->type == SDL_KEYDOWN)
    {
        int max_page = (session->total_scores - 1) / SCORES_PER_PAGE;

        switch (event->key.keysym.sym)
        {
            case SDLK_ESCAPE:
            case SDLK_RETURN:
                session->state = STATE_MENU;
                free_scores(session); // Clean up memory when leaving screen
                break;
            case SDLK_RIGHT: // Next Page
                if (session->current_page < max_page)
                    session->current_page++;
                break;
            case SDLK_LEFT: // Prev Page
                if (session->current_page > 0)
                    session->current_page--;
                break;
        }
    }
}