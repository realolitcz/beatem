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


// Function used to initialize Camera
void init_camera(Camera* camera)
{

    camera->camera_x = SCREEN_BEGINNING;
    camera->camera_y = SCREEN_BEGINNING;
    camera->camera_width = SCREEN_WIDTH;
    camera->camera_height = SCREEN_HEIGHT;

}


// // Function to load level from given level-config file
Level* load_level(const char* path, SDL_Texture* zombie, SDL_Texture* ghost)
{

    // File handle for level config file
    FILE* file {fopen(path, "r")};
    if (file == nullptr)
    {
        fclose(file);
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
            SDL_Texture* texture {type == ENEMY_TYPE_CHARGER ? ghost : zombie};
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
        .x = 0,
        .y = 0,
        .w = SOURCE_TILE_SIZE,
        .h = SOURCE_TILE_SIZE
    };
    constexpr SDL_Rect wall
    {
        .x = 16,
        .y = 0,
        .w = SOURCE_TILE_SIZE,
        .h = SOURCE_TILE_SIZE
    };
    constexpr SDL_Rect border
    {
        .x = 32,
        .y = 0,
        .w = SOURCE_TILE_SIZE,
        .h = SOURCE_TILE_SIZE
    };
    constexpr SDL_Rect darkness
    {
        .x = 32,
        .y = 16,
        .w = SOURCE_TILE_SIZE,
        .h = SOURCE_TILE_SIZE
    };
    constexpr SDL_Rect exit
    {
        .x = 0,
        .y = 16,
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
        .w = enemy->w,
        .h = enemy->h
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

    render_text(renderer, charset, x, y, buffer, 1.0f);

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
        if (player->multiplier_scale > 1.5f)
        {
            // Add random offset between -2 and 2
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
        else if (player->multiplier_scale > 1.0f)
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
        const int current_bar_width = (player->health_points * total_bar_width) / player->max_health_points;

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
void render_debug(SDL_Renderer* renderer, SDL_Texture* charset, Player* player, const Camera* camera)
{

    // Buffer for holding text
    char buffer[BASE_BUFFER_SIZE];
    // Show current action name
    snprintf(buffer, BASE_BUFFER_SIZE, "ACTION: %s", player->current_action);
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