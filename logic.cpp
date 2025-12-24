#include <SDL2/SDL.h>
#include <stdexcept>
#include "window.h"
#include "defines.h"
#include "logic.h"

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
                if (key == SDLK_k)
                {
                    player_light_attack(player);
                }
                if (key == SDLK_l)
                {
                    player_heavy_attack(player);
                }
            }
        }
    }

}


// Function to handle movement of the player on basis of the current PRESSED AND HOLD key
void handle_player_movement(Player* player, const Level* current_level, const Uint8* currentKeyStates)
{

    player->is_moving = false;

    if (currentKeyStates[SDL_SCANCODE_W])
    {
        if (player->global_y > FLOOR_HORIZON)
        {
            player->global_y -= player->player_speed;
            player->is_moving = true;
        }
    }
    if (currentKeyStates[SDL_SCANCODE_S])
    {
        if (player->global_y < FLOOR_BOTTOM)
        {
            player->global_y += player->player_speed;
            player->is_moving = true;
        }
    }
    if (currentKeyStates[SDL_SCANCODE_A])
    {
        if (player->global_x > FLOOR_LEFT_SIDE)
        {
            player->global_x -= player->player_speed;
            player->facing_right = false;
            player->is_moving = true;
        }
    }
    if (currentKeyStates[SDL_SCANCODE_D])
    {
        if (player->global_x < current_level->width_in_tiles * TARGET_TILE_SIZE -  2 * TARGET_TILE_SIZE)
        {
            player->global_x += player->player_speed;
            player->facing_right = true;
            player->is_moving = true;
        }
    }

}


// Function to change camera placement i.e. the part of visible area basing on current player position
void handle_camera_movement(const Player* player, const Level* current_level, Camera* camera)
{

    if (player->screen_x > GRACE_ZONE_END)
    {
        camera->camera_x = player->global_x - GRACE_ZONE_END;
    }
    if (player->screen_x < GRACE_ZONE_START)
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


// Function to handle the light/fast (action X) player attack
void player_light_attack(Player* player)
{

    if (player->action_timer == TIMER_ZERO)
    {
        player->action_type = LIGHT_ATTACK_PLAYER;
        player->action_timer = ATTACK_LIGHT_FRAMES;
    }

}


// Function to handle heavy/slow (action Y) player attack
void player_heavy_attack(Player* player)
{

    if (player->action_timer == TIMER_ZERO)
    {
        player->action_type = HEAVY_ATTACK_PLAYER;
        player->action_timer = ATTACK_HEAVY_FRAMES;
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
void check_combos(Player* player, const Level* current_level,const Uint32 current_time)
{

    // If keys were pressed in too big delay, we do not conduct combo
    if (current_time - player->buffer[0].time > COMBO_TIMEOUT)
    {
        return;
    }

    // First combo: triple light (k - k - k) attack
    if (player->buffer[0].key == SDLK_k &&
        player->buffer[1].key == SDLK_k &&
        player->buffer[2].key == SDLK_k)
    {
        // Only if sequence happened fast enough
        if (player->buffer[0].time - player->buffer[2].time < COMBO_TIMEOUT)
        {
            strcpy(player->current_action, "TRIPLE SLASH!");
            player->action_type = COMBO_FIRST_PLAYER;
            player->action_timer = COMBO1_FRAMES;
            clear_buffer(player);
            return;
        }
    }

    // Second combo: light - heavy - light (l - k - l) attack
    if (player->buffer[0].key == SDLK_k &&
        player->buffer[1].key == SDLK_l &&
        player->buffer[2].key == SDLK_k)
    {
        if (player->buffer[0].time - player->buffer[1].time < COMBO_TIMEOUT)
        {
            strcpy(player->current_action, "ULTIMATE BREAKER!");
            player->action_type = COMBO_SECOND_PLAYER;
            player->action_timer = COMBO2_FRAMES;
            clear_buffer(player);
            return;
        }
    }

    // Right dash (right - right)
    if (player->buffer[0].key == SDLK_d &&
        player->buffer[1].key == SDLK_d)
    {
        // Dash timeout is tighter than combo
        if (player->buffer[0].time - player->buffer[1].time < DASH_TIMEOUT)
        {
            strcpy(player->current_action, "DASH RIGHT >>");
            if (player->global_x + DASH_DISTANCE < current_level->width_in_tiles * TARGET_TILE_SIZE -  2 * TARGET_TILE_SIZE)
            {
                player->global_x += DASH_DISTANCE;
            }
            else
            {
                player->global_x = current_level->width_in_tiles * TARGET_TILE_SIZE - 2 * TARGET_TILE_SIZE;
            }
            clear_buffer(player);
            return;
        }
    }

    // Left dash (left - left)
    if (player->buffer[0].key == SDLK_a &&
        player->buffer[1].key == SDLK_a)
    {
        if (player->buffer[0].time - player->buffer[1].time < DASH_TIMEOUT)
        {
            strcpy(player->current_action, "DASH LEFT <<");
            if (player->global_x - DASH_DISTANCE > FLOOR_LEFT_SIDE)
            {
                player->global_x -= DASH_DISTANCE;
            }
            else
            {
                player->global_x = FLOOR_LEFT_SIDE;
            }
            clear_buffer(player);
            return;
        }
    }

    // DISCLAIMER TO ADDITIONAL B.c. - adding combos for programmer is easy with this construction -> we need only
    // to include additional conditional here and handle it e.g. with some new function

}


// Function to initialize enemy entity on a given position
void init_enemy(Enemy* enemy, SDL_Texture* texture, const int type, const int x, const int y)
{

    enemy->type = type;                   // Type: CHASER or CHARGER
    enemy->texture = texture;             // Spritesheet
    enemy->x = x;                         // Absolute x position in the level
    enemy->y = y;                         // Absolute y position in the level
    enemy->h = TARGET_TILE_SIZE;          // Height of the enemy entity
    enemy->w = TARGET_TILE_SIZE;          // Width of the enemy entity
    enemy->is_alive = true;               // Is enemy still alive?
    enemy->state = ENEMY_STATE_MOVING;    // State e.g. whether is attacking, etc.
    enemy->timer = TIMER_ZERO;            // General purpose timer for charging/cooldowns
    enemy->stun_timer = TIMER_ZERO;       // Frames remaining in stun
    enemy->hurt_timer = TIMER_ZERO;       // Counts down when enemy takes damage
    enemy->facing_right = true;           // Is enemy facing right?
    enemy->health_points = ENEMY_BASE_HP; // Health points of the enemy entity
    enemy->attack_box = {0, 0, 0, 0};  // The area of current attack TO the player
    enemy->last_hit_time = TIMER_ZERO;    // Time that passed since last hit received

}


// Function used to constantly handle enemies lifetime
void update_enemies(Enemy* enemies, const int count, const Player* player)
{

    for (int i {0}; i < count; i++)
    {

        Enemy* enemy {&enemies[i]};

        if (!enemy->is_alive)
        {
            continue;
        }

        // If stunned, count down and DO NOT move or act
        if (enemy->stun_timer > TIMER_ZERO)
        {
            enemy->stun_timer--;
            continue;
        }

        if (enemy->hurt_timer > TIMER_ZERO)
        {
            enemy->hurt_timer--;
        }

        // Calculate distance to the player
        const int dx {player->global_x - enemy->x}; // X distance
        const int dy {player->global_y - enemy->y}; // Y distance

        // Determine facing direction
        enemy->facing_right = (dx > 0);

        if (enemy->type == ENEMY_TYPE_CHASER)
        {
            // Chaser move closer to the player constantly (with some margin)
            if (abs(dx) > PLAYER_PRIVACY_ZONE)
            {
                enemy->x += (dx > 0 ? ENEMY_CHASE_SPEED : -ENEMY_CHASE_SPEED);
            }
            if (abs(dy) > PLAYER_PRIVACY_ZONE)
            {
                enemy->y += (dy > 0 ? ENEMY_CHASE_SPEED : -ENEMY_CHASE_SPEED);
            }
            // ADD COLLISION WITH PLAYER AND HURTING
        }

        if (enemy->type == ENEMY_TYPE_CHARGER)
        {
            switch (enemy->state)
            {
                // Keep distance and line up
                case ENEMY_STATE_MOVING:
                {
                    // Align Y axis => line up with the player
                    if (abs(dy) > TARGET_TILE_SIZE)
                    {
                        enemy->y += (dy > 0 ? ENEMY_ALIGN_SPEED : -ENEMY_ALIGN_SPEED);
                    }
                    // Maintain specific X distance
                    const int target_x = player->global_x + (dx > 0 ? -CHARGE_TRIGGER_RANGE : CHARGE_TRIGGER_RANGE);
                    int move_x = target_x - enemy->x;
                    if (abs(move_x) > TARGET_TILE_SIZE)
                    {
                        enemy->x += (move_x > 0 ? ENEMY_ALIGN_SPEED : - ENEMY_ALIGN_SPEED);
                    }
                    // Trigger charge <=> aligned vertically AND cooldown is ready
                    if (abs(dy) < TARGET_TILE_SIZE * 2 && enemy->timer == TIMER_ZERO)
                    {
                        enemy->state = ENEMY_STATE_CHARGING;
                        enemy->timer = 60;
                    }
                    break;
                }
                // Charging
                case ENEMY_STATE_CHARGING:
                {
                    // Move very fast in facing direction
                    enemy->x += (enemy->facing_right ? ENEMY_CHARGE_SPEED : -ENEMY_CHARGE_SPEED);
                    enemy->timer--;
                    if (enemy->timer <= TIMER_ZERO)
                    {
                        enemy->state = ENEMY_STATE_MOVING;
                        enemy->timer = 120;
                    }
                    break;
                }
                default:
                    ;
            }

            // Cooldown handling if not charging
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
        player->attack_box.x < enemy->x + enemy->w &&
        player->attack_box.x + player->attack_box.w > enemy->x &&
        player->attack_box.y < enemy->y + enemy->h &&
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

    if (!enemy->is_alive || player->hurt_timer > TIMER_ZERO || enemy->stun_timer > TIMER_ZERO)
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
        .w = enemy->w,
        .h = enemy->h
    };

    // AABB collision check
    const int collision =
    (
        player_rect.x < enemy_rect.x + enemy_rect.w &&
        player_rect.x + player_rect.w > enemy_rect.x &&
        player_rect.y < enemy_rect.y + enemy_rect.h &&
        player_rect.y + player_rect.h > enemy_rect.y
    );

    // Z-Axis check (jumping over enemy) <=> if player is high enough, they do not take damage (with a little leeway)
    if (collision && player->z < TARGET_TILE_SIZE)
    {
        player_take_damage(player, ENEMY_CONTACT_DAMAGE);
    }

}


void reset_player_state(Player* player)
{

    player->global_x = COLUMNS_PER_SCREEN * TARGET_TILE_SIZE / 2;
    player->global_y = VISIBLE_ROWS * TARGET_TILE_SIZE / 2;
    player->z = Z_GROUND_LEVEL;
    player->z_velocity = NO_Z_VELOCITY;
    player->screen_x = SCREEN_BEGINNING;
    player->player_speed = PLAYER_SPEED;
    player->is_moving = false;
    player->action_type = IDLE_PLAYER;
    player->action_timer = TIMER_ZERO;
    player->facing_right = true;
    clear_buffer(player);
    strcpy(player->current_action, "");
    player->debug_mode = false;
    player->attack_box = {0, 0, 0, 0};
    player->score_multiplier = 1;
    player->multiplier_scale = INITIAL_SCALE;
    player->last_score_time = TIMER_ZERO;
    player->score = 0;
    player->hurt_timer = TIMER_ZERO;
    player->health_points = PLAYER_MAX_HEALTH;
    player->max_health_points = PLAYER_MAX_HEALTH;

}

