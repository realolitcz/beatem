// File: window.cpp
// Header: window.h
// Author: Oliwer A. Gużewski (s208004)

// Support for legacy SDL versions provided by Dr Ostrowski
// extern "C" {
// #include"./SDL2-2.0.10/include/SDL.h"
// #include"./SDL2-2.0.10/include/SDL_main.h"

#include <SDL2/SDL.h>
#include "window.h"
#include "logic.h"
#include "defines.h"

// Function used to initialize the SDL HAL system and set render method
void init_sdl()
{
    if(SDL_Init(SDL_INIT_VIDEO) != 0 ||
        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest") != SDL_TRUE)
    {
        fprintf(stderr, "Initialization failed: %s\n", SDL_GetError());
        exit(1);
    }
}


// Function used to create SDL window of given parameters
SDL_Window* init_window(const char* title, const int width, const int height)
{

    SDL_Window* window {nullptr};

    window = SDL_CreateWindow
        (
            title,
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            width,
            height,
            SDL_WINDOW_SHOWN
        );

    if (window == nullptr)
    {
        fprintf(stderr, "Window creation failed: %s\n", SDL_GetError());
        exit(1);
    }

    return window;

}


// Function to create renderer for given window
SDL_Renderer* init_renderer(SDL_Window* window, const int width, const int height)
{

    SDL_Renderer* renderer {nullptr};

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    SDL_RenderSetLogicalSize(renderer, width, height);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

    if (renderer == nullptr)
    {
        fprintf(stderr, "Renderer creation failed: %s\n", SDL_GetError());
        exit(1);
    }

    return renderer;

}


// Function to create a texture from given file
SDL_Texture* init_texture(const char* path, SDL_Renderer* renderer)
{

    SDL_Surface* surface {nullptr};
    surface = SDL_LoadBMP(path);

    if(surface == nullptr)
    {
        fprintf(stderr, "Surface creation failed: %s\n", SDL_GetError());
        exit(1);
    }

    SDL_Texture* texture {nullptr};
    texture = SDL_CreateTextureFromSurface(renderer, surface);

    if (texture == nullptr)
    {
        SDL_FreeSurface(surface);
        fprintf(stderr, "Texture creation failed: %s\n", SDL_GetError());
        exit(1);
    }

    SDL_FreeSurface(surface);

    return texture;

}


// Helper function used to populated assets struct with given asset textures
void init_texture_assets(SDL_Renderer* renderer, TextureAssets* assets)
{

    assets->tileset = init_texture("assets/brawler_tileset.bmp", renderer);
    assets->charset = init_texture("assets/cs8x8.bmp", renderer);
    assets->knight  = init_texture("assets/knight_spritesheet.bmp", renderer);
    assets->zombie  = init_texture("assets/zombie_spritesheet.bmp", renderer);
    assets->ghost   = init_texture("assets/ghost_spritesheet.bmp", renderer);

}


// Function used to initialize camera
void init_camera(Camera* camera)
{

    camera->camera_x = SCREEN_BEGINNING;
    camera->camera_y = SCREEN_BEGINNING;
    camera->camera_width = SCREEN_WIDTH;
    camera->camera_height = SCREEN_HEIGHT;

}


// Function to load level from given level-config file
Level* load_level(const char* path, const TextureAssets* assets)
{

    // File handle for level config file
    FILE* file {fopen(path, "r")};
    if (file == nullptr)
    {
        fprintf(stderr, "Level loading failed: %s\n", SDL_GetError());
        exit(1);
    }

    // Allocate new level
    auto* level {new Level()};

    // Preamble read: width, height, enemy count
    if (fscanf(file, "%d %d %d", &level->width_in_tiles, &level->height_in_tiles, &level->enemy_count) != 3)
    {
        fclose(file);
        delete level;
        fprintf(stderr, "Invalid level file header (Preamble mismatch): %s\n", SDL_GetError());
        exit(1);
    }

    // Map allocation
    level->map_layout = new char[level->width_in_tiles * level->height_in_tiles];

    // Read map
    for (int i {0}; i < level->height_in_tiles * level->width_in_tiles; i++)
    {
        // Read char by char into the linear array
        fscanf(file, " %c", &level->map_layout[i]);
    }

    // Enemy allocation
    if (level->enemy_count > 0)
    {
        level->enemies = new Enemy[level->enemy_count];

        for (int i {0}; i < level->enemy_count; i++)
        {
            int type {0}, x {0}, y {0};
            // Read enemy data
            if (fscanf(file, "%d %d %d", &type, &x, &y) != 3)
            {
                // Validation failed: Preamble said there were more enemies than we found
                fclose(file);
                free_level(level);
                fprintf(stderr, "Invalid enemy data in level file\n");
                exit(1);
            }
            SDL_Texture* texture {type == ENEMY_TYPE_CHARGER ? assets->ghost : assets->zombie};
            init_enemy(&level->enemies[i], texture, type, x, y);
        }
    }
    else
    {
        level->enemies = nullptr;
    }

    fclose(file);

    printf("Level loaded: Size: %dx%d, Enemies: %d\n", level->width_in_tiles, level->height_in_tiles,
           level->enemy_count);

    return level;

}


// Function used to clean-up memory after level
void free_level(const Level* level)
{

    if (level != nullptr)
    {
        // Delete level array
        delete[] level->map_layout;
        // Delete enemy array
        delete[] level->enemies;
        // Delete the struct
        delete level;
    }

}


// Helper function used to handle transitions between levels
void change_level(Level** current_level_ptr, const char* path, const TextureAssets* assets, Player* player,
                  Camera* camera, const bool reset_stats)
{

    // Safety Cleanup
    if (*current_level_ptr != nullptr)
    {
        free_level(*current_level_ptr);
    }

    // Load New Data
    *current_level_ptr = load_level(path, assets);

    // Handle Player State
    if (reset_stats)
    {
        // Full wipe (New Game / Retry)
        reset_player_state(player);
    }
    else
    {
        // Keep Score (Next Stage)
        prepare_player_for_next_level(player);
    }

    camera->camera_x = 0;

}


// Helper function used to prepare and render menu background
void render_menu_background(SDL_Renderer* renderer, SDL_Texture* tileset)
{

    // We will be filling the background will pure darkness
    constexpr SDL_Rect source
    {
        .x = DARKNESS_X,
        .y = DARKNESS_Y,
        .w = SOURCE_TILE_SIZE,
        .h = SOURCE_TILE_SIZE
    };

    SDL_Rect destination
    {
        .x = 0,
        .y = 0,
        .w = TARGET_TILE_SIZE,
        .h = TARGET_TILE_SIZE
    };

    // Loop through the entire screen to tile the background
    for (int y {0}; y < SCREEN_HEIGHT; y += TARGET_TILE_SIZE)
    {
        for (int x {0}; x < SCREEN_WIDTH; x += TARGET_TILE_SIZE)
        {
            destination.x = x;
            destination.y = y;
            SDL_RenderCopy(renderer, tileset, &source, &destination);
        }
    }

}


// Function used to render main menu of the game
void render_menu(SDL_Renderer* renderer, SDL_Texture* charset, const GameSession* session)
{

    // Draw Title -> (ScreenCenter) - (Half of the string's unscaled width)
    const int title_x {static_cast<int>(SCREEN_WIDTH / 2 - strlen(TITLE) * TARGET_CHAR_SIZE / 2)};
    render_text(renderer, charset, title_x, MENU_ROW_ONE, TITLE, INITIAL_SCALE * 2.0f);

    // Draw Options
    const char* options[] = { "START GAME", "HIGH SCORES", "EXIT" };

    for (int i {0}; i < 3; i++)
    {
        const int y_pos {MENU_ROW_ONE + ((i + 1) * TARGET_TILE_SIZE)};
        const int text_len {static_cast<int>(strlen(options[i]))};
        const int x_pos {SCREEN_WIDTH / 2 - (text_len * TARGET_CHAR_SIZE) / 2};

        // Highlight selected option
        if (i == session->menu_selection)
        {
            SDL_SetTextureColorMod(charset, 255, 215, 0); // Gold
            render_text(renderer, charset, x_pos - TARGET_TILE_SIZE, y_pos, ">", INITIAL_SCALE * 1.5f);
        }
        else
        {
            SDL_SetTextureColorMod(charset, 255, 255, 255); // White
        }

        render_text(renderer, charset, x_pos, y_pos, options[i], INITIAL_SCALE * 1.5f);
    }

    SDL_SetTextureColorMod(charset, 255, 255, 255); // Reset


}


// Function used to render name input field
void render_name_input(SDL_Renderer* renderer, SDL_Texture* charset, const GameSession* session)
{

    const char* label {"ENTER NICKNAME:"};
    const int label_x {static_cast<int>(SCREEN_WIDTH / 2 - strlen(label) * TARGET_CHAR_SIZE / 2)};
    render_text(renderer, charset, label_x, MENU_ROW_ONE, label, INITIAL_SCALE * 1.5f);

    const char* name_to_render {(strlen(session->player_name) > 0) ? session->player_name : " "};
    const int name_x {static_cast<int>(SCREEN_WIDTH / 2 - strlen(name_to_render) * TARGET_CHAR_SIZE / 2)};

    // Draw text in Gold to show it is active input
    SDL_SetTextureColorMod(charset, 255, 215, 0);
    render_text(renderer, charset, name_x, MENU_ROW_ONE * 2, name_to_render, INITIAL_SCALE * 2.0f);
    SDL_SetTextureColorMod(charset, 255, 255, 255);

    const char* prompt {"[PRESS ENTER]"};
    const int prompt_x {static_cast<int>(SCREEN_WIDTH / 2 - strlen(prompt) * TARGET_CHAR_SIZE / 2)};
    render_text(renderer, charset, prompt_x, MENU_ROW_ONE * 3, prompt, INITIAL_SCALE * 1.5f);

}


// Function used to render game-over screen
void render_game_over(SDL_Renderer* renderer, SDL_Texture* charset, const Player* player)
{

    // If player has health, it's a victory, not a game over
    const char* title {(player->health_points > 0) ? "VICTORY!" : "GAME OVER"};

    const int title_x {static_cast<int>(SCREEN_WIDTH / 2 - strlen(title) * TARGET_CHAR_SIZE / 2)};

    if (player->health_points > 0)
    {
        SDL_SetTextureColorMod(charset, 0, 255, 0);
    }
    render_text(renderer, charset, title_x, MENU_ROW_ONE, title, INITIAL_SCALE * 3.0f);
    SDL_SetTextureColorMod(charset, 255, 255, 255);

    char score_buffer[BASE_BUFFER_SIZE];
    snprintf(score_buffer, sizeof(score_buffer), "FINAL SCORE: %d", player->score);
    const int score_x {static_cast<int>(SCREEN_WIDTH / 2 - strlen(score_buffer) * TARGET_CHAR_SIZE / 2)};
    render_text(renderer, charset, score_x, MENU_ROW_ONE * 2, score_buffer, INITIAL_SCALE * 1.5f);

    const char* prompt {"PRESS 'Y' TO EXIT OR 'N' TO RESET"};
    const int prompt_x {static_cast<int>(SCREEN_WIDTH / 2 - strlen(prompt) * TARGET_CHAR_SIZE / 2)};

    SDL_SetTextureColorMod(charset, 255, 100, 100);
    render_text(renderer, charset, prompt_x, MENU_ROW_ONE * 3, prompt, INITIAL_SCALE * 1.5f);
    SDL_SetTextureColorMod(charset, 255, 255, 255);

}


// Function used to handle rendering of level background
void render_background(SDL_Renderer* renderer, SDL_Texture* texture, const Level* level, const Camera* camera)
{

    // Holder for rendered tile
    SDL_Rect source
    {
        .x = 0,
        .y = 0,
        .w = SOURCE_TILE_SIZE,
        .h = SOURCE_TILE_SIZE
    };

    // Markers of tiles in the tileset
    constexpr SDL_Rect floor
    {
        .x = FLOOR_X,
        .y = FLOOR_Y,
        .w = SOURCE_TILE_SIZE,
        .h = SOURCE_TILE_SIZE
    };
    constexpr SDL_Rect wall
    {
        .x = WALL_X,
        .y = WALL_Y,
        .w = SOURCE_TILE_SIZE,
        .h = SOURCE_TILE_SIZE
    };
    constexpr SDL_Rect border
    {
        .x = BORDER_X,
        .y = BORDER_Y,
        .w = SOURCE_TILE_SIZE,
        .h = SOURCE_TILE_SIZE
    };
    constexpr SDL_Rect darkness
    {
        .x = DARKNESS_X,
        .y = DARKNESS_Y,
        .w = SOURCE_TILE_SIZE,
        .h = SOURCE_TILE_SIZE
    };
    constexpr SDL_Rect exit
    {
        .x = EXIT_X,
        .y = EXIT_X,
        .w = SOURCE_TILE_SIZE,
        .h = SOURCE_TILE_SIZE
    };

    // Holder for render destination
    SDL_Rect destination
    {
        .x = 0,
        .y = 0,
        .w = TARGET_TILE_SIZE,
        .h = TARGET_TILE_SIZE
    };

    // Markers of currently visible level-area
    int start_column {camera->camera_x / TARGET_TILE_SIZE};
    int end_column {(camera->camera_x + SCREEN_WIDTH) / TARGET_TILE_SIZE + 1};

    // Clap start column and end column not to exceed level width
    if (start_column < 0)
    {
        start_column = 0;
    }
    if (end_column >= level->width_in_tiles)
    {
        end_column = level->width_in_tiles;
    }

    for (int row {0}; row < level->height_in_tiles; row++)
    {
        for (int column {start_column}; column < end_column; column++)
        {
            switch (const int index = (row * level->width_in_tiles) + column; level->map_layout[index])
            {
                case 'F':
                    source = floor;
                    break;
                case 'W':
                    source = wall;
                    break;
                case 'B':
                    source = border;
                    break;
                case 'D':
                    source = darkness;
                    break;
                case 'E':
                    source = exit;
                    break;
                case ' ':
                default:
                    continue;
            }

            destination =
            {
                column * TARGET_TILE_SIZE - camera->camera_x,
                row * TARGET_TILE_SIZE,
                TARGET_TILE_SIZE,
                TARGET_TILE_SIZE,
            };

            SDL_RenderCopy(renderer, texture, &source, &destination);
        }
    }

}


// Function used to render the player
void render_player(SDL_Renderer* renderer, const Player* player, const Camera* camera)
{

    // Player is set default to walk-sprite
    int row_offset {ROW_WALK_OFFSET};
    int total_duration {0};
    int total_frames {0};
    int current_frame {0};

    // Source sprite for the player
    SDL_Rect source
    {
        .x = row_offset,
        .y = current_frame,
        .w = SOURCE_TILE_SIZE,
        .h = SOURCE_TILE_SIZE
    };

    // Priority 0 -> hurt animation
    if (player->hurt_timer > 0)
    {
        row_offset = ROW_HURT_OFFSET;
        total_duration = HURT_DURATION;
        total_frames = FRAMES_HURT;
        // As timer goes 20->0, frame goes 0->3
        current_frame = (total_duration - player->hurt_timer) * total_frames / total_duration;
    }

    // Priority 1 -> attack animation
    else if (player->action_type != IDLE_PLAYER)
    {
        switch (player->action_type)
        {
            case LIGHT_ATTACK_PLAYER:
                row_offset = ROW_LIGHT_ATK_OFFSET;
                total_frames = FRAMES_LIGHT;
                total_duration = ATTACK_LIGHT_FRAMES;
                break;
            case HEAVY_ATTACK_PLAYER:
                row_offset = ROW_HEAVY_ATK_OFFSET;
                total_frames = FRAMES_HEAVY;
                total_duration = ATTACK_HEAVY_FRAMES;
                break;
            case COMBO_FIRST_PLAYER:
                row_offset = ROW_COMBO1_OFFSET;
                total_frames = FRAMES_COMBO1;
                total_duration = COMBO1_FRAMES;
                break;
            case COMBO_SECOND_PLAYER:
                row_offset = ROW_COMBO2_OFFSET;
                total_frames = FRAMES_COMBO2;
                total_duration = COMBO2_FRAMES;
                break;
            case AERIAL_ATTACK_PLAYER:
                row_offset = ROW_AERIAL_ATK_OFFSET;
                total_frames = FRAMES_AERIAL;
                total_duration = AERIAL_ATTACK_FRAMES;
                break;
            default:
                ;
        }
        // Calculate frame based on how much time has passed for this action
        const int time_elapsed = total_duration - player->action_timer;
        current_frame = time_elapsed * total_frames / total_duration;
        // Clamp frame to ensure we don't go off-sheet
        if (current_frame >= total_frames)
        {
            current_frame = total_frames - 1;
        }
    }

    // Priority 2 -> movement animation
    else if (player->is_moving)
    {
        row_offset = ROW_WALK_OFFSET;
        // Cycle 2 frames based on global time (speed 150ms)
        current_frame = (SDL_GetTicks() / 150) % FRAMES_WALK;
    }

    // Priority 3 -> just chilling
    else
    {
        row_offset = ROW_WALK_OFFSET;
        // Just stand still (First frame)
        current_frame = 0;
    }

    // Apply Coordinates
    source.y = row_offset;
    source.x = current_frame * 16; // Shift X by 16px per frame

    // Calculate Destination
    const SDL_Rect destination
    {
        .x = player->global_x - camera->camera_x,
        .y = player->global_y - static_cast<int>(player->z),
        .w = TARGET_TILE_SIZE,
        .h = TARGET_TILE_SIZE
    };

    // The spritesheet faces RIGHT by default <=> if facing LEFT, flip horizontally.
    const SDL_RendererFlip flip = player->facing_right ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;

    SDL_RenderCopyEx(renderer, player->texture, &source, &destination, 0, nullptr, flip);

}


// Function used to render the enemies
void render_enemy(SDL_Renderer* renderer, const Enemy* enemy, const Camera* camera)
{

    if (!enemy->is_alive)
    {
        return;
    }

    // Enemy is set default to walk-sprite
    int row_offset {ROW_ENEMY_WALK_OFFSET};
    int current_frame {0};

    // Priority 0 -> stunned animation
    if (enemy->stun_timer > TIMER_ZERO && enemy->hurt_timer == TIMER_ZERO)
    {
        row_offset = ROW_ENEMY_STUN_OFFSET;
        // Cycle 3 frames quickly (speed 100 ms)
        current_frame = (SDL_GetTicks() / 100) % FRAMES_ENEMY_STUN;
    }

    // Priority 1 -> hurt animation
    else if (enemy->hurt_timer > TIMER_ZERO)
    {
        row_offset = ROW_ENEMY_HURT_OFFSET;
        // Map timer 20->0 to frames 0->2
        current_frame = (STUN_DURATION - enemy->hurt_timer) * FRAMES_ENEMY_HURT / STUN_DURATION;
        if (current_frame >= FRAMES_ENEMY_HURT)
        {
            current_frame = FRAMES_ENEMY_HURT - 1;
        }
    }

    // Priority 2 -> charging (only for Ghost)
    else if (enemy->state == ENEMY_STATE_CHARGING)
    {
        // Use walk animation, but faster to look aggressive (speed 50 ms)
        row_offset = ROW_ENEMY_WALK_OFFSET;
        current_frame = (SDL_GetTicks() / 50) % FRAMES_ENEMY_WALK;
    }

    // Priority 3 -> walk animation
    else
    {
        row_offset = ROW_ENEMY_WALK_OFFSET;
        // Cycle 2 frames based on global time (speed 200ms)
        current_frame = (SDL_GetTicks() / 200) % FRAMES_ENEMY_WALK;
    }

    // Calculate source
    const SDL_Rect src
    {
        .x = current_frame * SOURCE_TILE_SIZE,
        .y = row_offset,
        .w = SOURCE_TILE_SIZE,
        .h = SOURCE_TILE_SIZE
    };

    // Calculate destination
    const SDL_Rect dst
    {
        .x = enemy->x - camera->camera_x,
        .y = enemy->y,
        .w = ENEMY_SIZE,
        .h = ENEMY_SIZE
    };

    // The spritesheet faces RIGHT by default <=> if facing LEFT, flip horizontally.
    const SDL_RendererFlip flip = enemy->facing_right ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;

    SDL_RenderCopyEx(renderer, enemy->texture, &src, &dst, 0, nullptr, flip);

}


// Helper function for render_text to prepare label:value lines for stats
void render_stat(SDL_Renderer* renderer, SDL_Texture* charset, const int x, const int y, const char* label,
                 const int value)
{

    char buffer[BASE_BUFFER_SIZE];

    snprintf(buffer, BASE_BUFFER_SIZE, "%s: %d", label, value);

    render_text(renderer, charset, x, y, buffer, INITIAL_SCALE);

}


// Function to render all necessary status information
void render_statusbar(SDL_Renderer* renderer, SDL_Texture* charset, const Player* player, const Uint32 current_time)
{

    // X POSITION:
    render_stat(renderer, charset, SCREEN_BEGINNING, SCREEN_BEGINNING,  "X POSITION",
               player->global_x);
    // Y POSITION:
    render_stat(renderer, charset, SCREEN_BEGINNING, SCREEN_BEGINNING + TARGET_CHAR_SIZE, "Y POSITION",
           player->global_y);
    // CURRENT TIME:
    render_stat(renderer, charset, SCREEN_BEGINNING, SCREEN_BEGINNING + 2 * TARGET_CHAR_SIZE, "ELAPSED TIME",
           current_time);

    // Next "column"

    // SCORE:
    render_stat(renderer, charset, TARGET_CHAR_SIZE * 20, SCREEN_BEGINNING, "SCORE", player->score);
    // MULTIPLIER:
    render_multiplier(renderer, charset, player);
    // Player health bar
    render_health_bar(renderer, player);

}


// Function used to render the flashy and bold multiplier graphic
void render_multiplier(SDL_Renderer* renderer, SDL_Texture* charset, const Player* player)
{

    // Only show if we actually have a combo
    if (player->score_multiplier > 1)
    {
        char buffer[BASE_BUFFER_SIZE];
        // Short, Punchy Text
        snprintf(buffer, BASE_BUFFER_SIZE, "%dx COMBO!", player->score_multiplier);

        // Circa 3/4 of the screen
        int combo_x {SCREEN_WIDTH - (TARGET_TILE_SIZE * 8)};
        int combo_y = {TARGET_TILE_SIZE * 1/2};

        // If the scale is big => shake the text
        if (player->multiplier_scale > INITIAL_SCALE * 1.5f)
        {
            // Add pseudorandom offset between -2 and 2
            combo_x += (rand() % 5) - 2;
            combo_y += (rand() % 5) - 2;
        }

        // Colours: Normal = White/Gold, High Combo (>5) = Rainbow Cycle.
        if (player->score_multiplier >= 5)
        {
            // Cycle colors every 100ms
            int cycle {static_cast<int>(SDL_GetTicks() / 100 % 3)};
            switch (cycle)
            {
                case 0:
                    SDL_SetTextureColorMod(charset, 255, 0, 0);
                    break;
                case 1:
                    SDL_SetTextureColorMod(charset, 0, 255, 0);
                    break;
                default:
                    SDL_SetTextureColorMod(charset, 0, 255, 255);
                    break;
            }
        }
        else if (player->multiplier_scale > INITIAL_SCALE)
        {
            SDL_SetTextureColorMod(charset, 255, 215, 0); // Gold (On Hit)
        }
        else
        {
            SDL_SetTextureColorMod(charset, 200, 200, 200); // Silver (Decaying)
        }

        render_text(renderer, charset, combo_x, combo_y, buffer, player->multiplier_scale);

        SDL_SetTextureColorMod(charset, 255, 255, 255);
    }

}


// Helper function to prepare and handle Player health bar
void render_health_bar(SDL_Renderer* renderer, const Player* player)
{

    constexpr int total_bar_width {TARGET_TILE_SIZE * 4};
    constexpr int bar_height {TARGET_TILE_SIZE * 1/3};
    constexpr int bar_x {(SCREEN_WIDTH - total_bar_width) * 1/2};
    constexpr int bar_y {SCREEN_HEIGHT - (TARGET_TILE_SIZE * 2/3)};

    constexpr SDL_Rect background
    {
        .x = bar_x,
        .y = bar_y,
        .w = total_bar_width,
        .h = bar_height,
    };

    SDL_SetRenderDrawColor(renderer, 50, 0, 0, 255);
    SDL_RenderFillRect(renderer, &background);

    if (player->health_points > 0)
    {
        const int current_bar_width = (player->health_points * total_bar_width) / PLAYER_MAX_HEALTH;

        const SDL_Rect foreground
        {
            .x = bar_x,
            .y = bar_y,
            .w = current_bar_width,
            .h = bar_height,
        };

        if (current_bar_width > total_bar_width * 1/2)
        {
            SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        }
        else if (current_bar_width > total_bar_width * 1/4)
        {
            SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
        }
        else
        {
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        }

        SDL_RenderFillRect(renderer, &foreground);
    }

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &background);

}


// Function used to render given text on given position. Based on Dr Ostrowski's template
void render_text(SDL_Renderer* renderer, SDL_Texture* texture, const int x, const int y, const char* text,
                 const float scale)
{

    const int length {static_cast<int>(strlen(text))};

    // Size of original char
    constexpr int original_size {TARGET_CHAR_SIZE};
    // Size of destination char
    const int scaled_size {static_cast<int>(TARGET_CHAR_SIZE * scale)};

    // Calculate centering offset ->  when scale > 1.0f, shift X/Y back by half, so the text appears to grow from
    // its center, not top-left
    const int x_offset {(length * (scaled_size - original_size)) / 2};
    const int y_offset {(scaled_size - original_size) / 2};

    SDL_Rect source
    {
        .x = SCREEN_BEGINNING,
        .y = SCREEN_BEGINNING,
        .w = SOURCE_CHAR_SIZE,
        .h = SOURCE_CHAR_SIZE
    };

    SDL_Rect destination
    {
        .x = x - x_offset,
        .y = y - y_offset,
        .w = scaled_size,
        .h = scaled_size
    };

    for (int i {0}; i < length; i++)
    {
        const unsigned char character = text[i];

        source.x = (character % SHEET_COLUMNS) * SOURCE_CHAR_SIZE;
        source.y = (character / SHEET_COLUMNS) * SOURCE_CHAR_SIZE;

        SDL_RenderCopy(renderer, texture, &source, &destination);

        destination.x += scaled_size;
    }

}


// Function to render info associated with the developer mode
void render_debug(SDL_Renderer* renderer, SDL_Texture* charset, const Player* player, const Camera* camera)
{

    // Buffer for holding text
    char buffer[BASE_BUFFER_SIZE];
    // Show current action name
    const char* current_action {"IDLE"};

    switch(player->action_type)
    {
        case LIGHT_ATTACK_PLAYER:
            current_action = "LIGHT ATTACK";
            break;
        case HEAVY_ATTACK_PLAYER:
            current_action = "HEAVY ATTACK";
            break;
        case COMBO_FIRST_PLAYER:
            current_action = "TRIPLE SLASH";
            break;
        case COMBO_SECOND_PLAYER:
            current_action = "ULTIMATE BREAKER";
            break;
        case AERIAL_ATTACK_PLAYER:
            current_action = "AERIAL KICK";
            break;
        default:
            current_action = "IDLE";
            break;
    }

    snprintf(buffer, BASE_BUFFER_SIZE, "ACTION: %s", current_action);
    render_text(renderer, charset, SCREEN_BEGINNING, SCREEN_HEIGHT - 2 * TARGET_CHAR_SIZE, buffer,
           INITIAL_SCALE);

    // Visualize the Buffer (show last 5 keys)
    render_text(renderer, charset, SCREEN_BEGINNING, SCREEN_HEIGHT - TARGET_CHAR_SIZE, "BUFFER: ",
           INITIAL_SCALE);

    for(int i {0}; i < 5; i++)
    {
        const char* keyname = SDL_GetKeyName(player->buffer[i].key);
        // If buffer is empty, print "-"
        if(player->buffer[i].key == 0)
        {
                keyname = "-";
        }
        snprintf(buffer, BASE_BUFFER_SIZE, "%s", keyname);
        render_text(renderer, charset, (strlen("BUFFER: ") * TARGET_CHAR_SIZE) + (i * 16),
                  SCREEN_HEIGHT - TARGET_CHAR_SIZE, buffer, INITIAL_SCALE);
    }

    if (player->attack_box.w > 0)
    {
        const SDL_Rect attack_box
        {
            .x = player->attack_box.x - camera->camera_x,
            .y = player->attack_box.y,
            .w = player->attack_box.w,
            .h = player->attack_box.h
        };
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 100);
        SDL_RenderDrawRect(renderer, &attack_box);
    }

}


// Function used to render high scores screen
void render_highscores(SDL_Renderer* renderer, SDL_Texture* charset, const GameSession* session)
{

    // Title
    const char* title {"HALL OF FAME"};
    render_text(renderer, charset, SCREEN_WIDTH/2 - (strlen(title)*TARGET_CHAR_SIZE)/2,
                TARGET_TILE_SIZE * 2, title, INITIAL_SCALE * 2.0f);

    if (session->total_scores == 0)
    {
        const char* msg {"NO RECORDS YET"};
        render_text(renderer, charset, SCREEN_WIDTH/2 - (strlen(msg)*TARGET_CHAR_SIZE)/2,
                    SCREEN_HEIGHT/2, msg, INITIAL_SCALE * 1.5f);
        return;
    }

    // Pagination calculation
    const int start_index {session->current_page * SCORES_PER_PAGE};
    int end_index {start_index + SCORES_PER_PAGE};
    if (end_index > session->total_scores)
    {
        end_index = session->total_scores;
    }

    int y_pos {TARGET_TILE_SIZE * 4};

    // Render List
    for (int i {start_index}; i < end_index; i++)
    {
        char buffer[BASE_BUFFER_SIZE];
        // Format: "1. NAME ...... 1000"
        snprintf(buffer, sizeof(buffer), "%d. %s", i + 1, session->high_scores[i].name);

        // Draw Name (Left aligned-ish)
        render_text(renderer, charset, SCREEN_WIDTH/3, y_pos, buffer, INITIAL_SCALE * 1.5f);

        // Draw Score (Right aligned-ish)
        snprintf(buffer, sizeof(buffer), "%d", session->high_scores[i].score);
        render_text(renderer, charset, SCREEN_WIDTH * 2/3, y_pos, buffer, INITIAL_SCALE * 1.5f);

        y_pos += TARGET_TILE_SIZE;
    }

    // Render pagination controls
    char page_buf[BASE_BUFFER_SIZE];
    const int total_pages {(session->total_scores - 1) / SCORES_PER_PAGE + 1};
    snprintf(page_buf, BASE_BUFFER_SIZE, "< PAGE %d / %d >", session->current_page + 1, total_pages);

    render_text(renderer, charset, SCREEN_WIDTH/2 - (strlen(page_buf)*TARGET_CHAR_SIZE)/2,
              SCREEN_HEIGHT - TARGET_TILE_SIZE * 2, page_buf, INITIAL_SCALE * 1.5f);

}


// Master wrapper for all game rendering guys
void render_game(SDL_Renderer* renderer, const GameSession* session, const Player* player, const Level* level,
                 const Camera* camera, const TextureAssets* assets, const Uint32 game_time)
{

    // Draw backgrounds
    if ((session->state == STATE_GAMEPLAY || session->state == STATE_GAME_OVER) && level != nullptr)
    {
        render_background(renderer, assets->tileset, level, camera);
    }
    else
    {
        render_menu_background(renderer, assets->tileset);
    }

    // Draw UI / Entities
    switch (session->state)
    {
        case STATE_MENU:
            render_menu(renderer, assets->charset, session);
            break;
        case STATE_NAME_INPUT:
            render_name_input(renderer, assets->charset, session);
            break;
        case STATE_GAMEPLAY:
            // Render Entities on top of the level background
            if (player->debug_mode)
            {
                render_debug(renderer, assets->charset, player, camera);
            }
            if (level)
            {
                for (int i {0}; i < level->enemy_count; i++)
                {
                    render_enemy(renderer, &level->enemies[i], camera);
                }
            }
            render_player(renderer, player, camera);
            render_statusbar(renderer, assets->charset, player, game_time);
            break;
        case STATE_GAME_OVER:
            render_menu_background(renderer, assets->tileset);
            render_game_over(renderer, assets->charset, player);
            break;
        case STATE_SCORES:
            render_highscores(renderer, assets->charset, session);
            break;
        default:
            break;
    }

}