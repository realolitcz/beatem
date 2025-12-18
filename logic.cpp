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
        const int key = event->key.keysym.sym;
        int combo_triggered = FALSE;

        if (key == SDLK_F1)
        {
            player->debug_mode = !player->debug_mode;
        }

        // Always push key to the buffer
        if (key == SDLK_k || key == SDLK_l || key == SDLK_a || key == SDLK_d)
        {
            push_input(player, key, SDL_GetTicks());

            const int previous_time = player->action_timer;

            check_combos(player, current_level,SDL_GetTicks());

            if (player->action_timer > previous_time)
            {
                combo_triggered = TRUE;
            }
        }

        // If combo was not triggered we perform normal actions
        if (combo_triggered == FALSE)
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


// Function to handle movement of the player on basis of the current PRESSED AND HOLDED key
void handle_player_movement(Player* player, const Level* current_level,const Uint8* currentKeyStates)
{

    if (currentKeyStates[SDL_SCANCODE_W])
    {
        if (player->global_y > FLOOR_HORIZON)
        {
            player->global_y -= player->player_speed;
        }
    }
    if (currentKeyStates[SDL_SCANCODE_S])
    {
        if (player->global_y < FLOOR_BOTTOM)
        {
            player->global_y += player->player_speed;
        }
    }
    if (currentKeyStates[SDL_SCANCODE_A])
    {
        if (player->global_x > FLOOR_LEFT_SIDE)
        {
            player->global_x -= player->player_speed;
        }
    }
    if (currentKeyStates[SDL_SCANCODE_D])
    {
        if (player->global_x < current_level->width_in_tiles * TARGET_TILE_SIZE -  2 * TARGET_TILE_SIZE)
        {
            player->global_x += player->player_speed;
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

    if (player->action_timer == 0)
    {
        player->action_type = LIGHT_ATTACK_PLAYER;
        player->action_timer = ATTACK_LIGHT_FRAMES;
    }

}


// Function to handle heavy/slow (action Y) player attack
void player_heavy_attack(Player* player)
{

    if (player->action_timer == 0)
    {
        player->action_type = HEAVY_ATTACK_PLAYER;
        player->action_timer = ATTACK_HEAVY_FRAMES;
    }

}


// Function used to wrap tick-end attack-blockades
void handle_attack(Player* player)
{

    if (player->action_timer > 0)
    {
        player->action_timer--;
    }
    else
    {
        player->action_type = IDLE_PLAYER;
    }

}


// Function to conduct player jump i.e. add z-axis velocity
void player_jump(Player* player)
{

    if (player->z == 0)
    {
        player->z_velocity = JUMP_FORCE;
    }

}


// Function use to wrap tick-end gravity apply
void handle_gravity(Player* player)
{

    if (player->z > 0 || player->z_velocity != 0)
    {
        player->z += player->z_velocity;
        player->z_velocity -= GRAVITY;
    }

    if (player->z < 0)
    {
        player->z = 0;
        player->z_velocity = 0;
    }

}


// Function to push input key to the combo queue
void push_input(Player* player, const int key, const Uint32 time)
{

    // Add key to the buffer <=> shift all elements down
    for (int i = 9; i > 0; i--)
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

    for (int i = 0; i < 10; i++)
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
            player->action_timer = ATTACK_LIGHT_FRAMES * 3;
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
            player->action_timer = ATTACK_HEAVY_FRAMES * 3;
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

    // DISCLAIMER TO ADDITIONALS B.c. - adding combos for proggramer is easy with this construction -> we need only
    // to include additional conditional here and handle it e.g. with some new function

}