#include <stdexcept>
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
        throw std::runtime_error("Initialization failed");
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
        throw std::runtime_error("Cannot create window!");
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
        throw std::runtime_error("Cannot create renderer!");
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
        throw std::runtime_error("Cannot initialize surface!");
    }

    SDL_Texture* texture {nullptr};
    texture = SDL_CreateTextureFromSurface(renderer, surface);

    if (texture == nullptr)
    {
        SDL_FreeSurface(surface);
        throw std::runtime_error("Cannot initialize texture!");
    }

    SDL_FreeSurface(surface);

    return texture;

}


// Function to load level from given level-config file
Level* load_level(const char* path)
{

    // file handle for level-config file
    FILE* file {fopen(path, "r")};

    if (file == nullptr)
    {
        throw std::runtime_error("Cannot open level file!");
    }

    // allocate memory for the level struct per se
    Level* level {static_cast<Level*>(malloc(sizeof(Level)))};

    if (level == nullptr)
    {
        free(level);
        throw std::runtime_error("Cannot allocate level!");
    }

    // read the preamble of the file i.e. the width and height (first line of the file)
    if (fscanf(file, "%d %d", &level->width_in_tiles, &level->height_in_tiles) != 2)
    {
        fclose(file);
        throw std::runtime_error("Invalid level file header");
    }

    // allocate array of arrays to store level's tile map
    level->map_layout = static_cast<char**>(malloc(level->height_in_tiles * sizeof(char*)));

    // allocate each row of the tile map
    for (int row = 0; row < level->height_in_tiles; row++)
    {
        level->map_layout[row] = static_cast<char*>(malloc(level->width_in_tiles * sizeof(char)));
    }

    // read and store every tile info from the level file
    for (int row = 0; row < level->height_in_tiles; row++)
    {
        for (int column = 0; column < level->width_in_tiles; column++)
        {
            fscanf(file, " %c", &level->map_layout[row][column]);
        }
    }

    fclose(file);

    // debug info
    printf("Level loaded: Size: %dx%d", level->width_in_tiles, level->height_in_tiles);

    return level;

}


// Function used to clean-up level memory
void free_level(Level* level)
{

    for (int row = 0; row < level->height_in_tiles; row++)
    {
        free(level->map_layout[row]);
    }
    free(level->map_layout);

    free(level);

}


// Function used to handle rendering of level background
void render_background(SDL_Renderer* renderer, SDL_Texture* texture, const Level* level, const Camera* camera)
{

    SDL_Rect src{0, 0, SOURCE_TILE_SIZE, SOURCE_TILE_SIZE};

    SDL_Rect floor{0, 0, SOURCE_TILE_SIZE, SOURCE_TILE_SIZE};
    SDL_Rect wall{16, 0, SOURCE_TILE_SIZE, SOURCE_TILE_SIZE};
    SDL_Rect border{32, 0, SOURCE_TILE_SIZE, SOURCE_TILE_SIZE};
    SDL_Rect darkness{32, 16, SOURCE_TILE_SIZE, SOURCE_TILE_SIZE};

    SDL_Rect dst{0, 0, TARGET_TILE_SIZE, TARGET_TILE_SIZE};

    int start_column = camera->camera_x / TARGET_TILE_SIZE;
    int end_column = (camera->camera_x + SCREEN_WIDTH) / TARGET_TILE_SIZE + 1;

    if (start_column < 0)
    {
        start_column = 0;
    }

    if (end_column >= level->width_in_tiles)
    {
        end_column = level->width_in_tiles;
    }

    for (int row = 0; row < level->height_in_tiles; row++)
    {
        for (int column = start_column; column < end_column; column++)
        {
            switch (level->map_layout[row][column])
            {
                case 'F':
                    src = floor;
                    break;
                case 'W':
                    src = wall;
                    break;
                case 'B':
                    src = border;
                    break;
                case 'D':
                    src = darkness;
                    break;
                case ' ':
                default:
                    continue;
            }

            dst =
            {
                column * TARGET_TILE_SIZE - camera->camera_x,
                row * TARGET_TILE_SIZE,
                TARGET_TILE_SIZE,
                TARGET_TILE_SIZE,
            };

            SDL_RenderCopy(renderer, texture, &src, &dst);
        }
    }

}


// Function used to render the player
void render_player(SDL_Renderer* renderer, const Player* player, const Camera* camera)
{

	SDL_Rect src;

    // Player is set default to walk-sprite
    int row_offset = ROW_WALK_OFFSET;
    int total_duration = 0;
    int total_frames = 0;
    int current_frame = 0;

    src = {row_offset, current_frame, SOURCE_TILE_SIZE, SOURCE_TILE_SIZE};

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
    src.y = row_offset;
    src.x = current_frame * 16; // Shift X by 16px per frame

    // Calculate Destination
    const SDL_Rect dst
    {
        player->global_x - camera->camera_x,
        player->global_y - static_cast<int>(player->z),
        TARGET_TILE_SIZE,
        TARGET_TILE_SIZE
    };

    // The spritesheet faces RIGHT by default <=> if facing LEFT, flip horizontally.
    const SDL_RendererFlip flip = player->facing_right ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;

    SDL_RenderCopyEx(renderer, player->texture, &src, &dst, 0, nullptr, flip);

}


// Function used to render the enemies
void render_enemy(SDL_Renderer* renderer, const Enemy* enemy, const Camera* camera)
{

    if (!enemy->is_alive)
    {
        return;
    }

    int row_offset = ROW_ENEMY_WALK_OFFSET;
    int current_frame = 0;

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
        .x = row_offset,
        .y = current_frame * SOURCE_TILE_SIZE,
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

    sprintf(buffer, "%s: %d", label, value);

    render_text(renderer, charset, x, y, buffer);

}


// Function to render all necessary status information
void render_statbar(SDL_Renderer* renderer, SDL_Texture* charset, const Player* player, const Uint32 current_time)
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
    render_stat(renderer, charset, TARGET_CHAR_SIZE * 20, SCREEN_BEGINNING + TARGET_CHAR_SIZE, "MULTIPLIER",
           player->score_multiplier);
    // Player health bar
    render_health_bar(renderer, player, TARGET_CHAR_SIZE * 20, SCREEN_BEGINNING + 2 * TARGET_CHAR_SIZE);

}


// Helper function to prepare and handle Player health bar
void render_health_bar(SDL_Renderer* renderer, const Player* player, int x, int y)
{

    const SDL_Rect background
    {
        .x = x,
        .y = y,
        .w = 100,
        .h = 10,
    };

    SDL_SetRenderDrawColor(renderer, 50, 0, 0, 255);
    SDL_RenderFillRect(renderer, &background);

    if (player->health_points > 0)
    {
        const int bar_width {(player->health_points * 100) / player->max_health_points};
        const SDL_Rect foreground
        {
        .x = x,
        .y = y,
        .w = bar_width,
        .h = 10,
        };
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        SDL_RenderFillRect(renderer, &foreground);
    }

}


// Function used to render given text on given position
void render_text(SDL_Renderer* renderer, SDL_Texture* texture, const int x, const int y, const char* text)
{

    SDL_Rect src{0, 0, SOURCE_CHAR_SIZE, SOURCE_CHAR_SIZE};
    SDL_Rect dst{x, y, TARGET_CHAR_SIZE, TARGET_CHAR_SIZE};

    const int length = strlen(text);

    for (int i = 0; i < length; i++)
    {
        const unsigned char character = text[i];

        src.x = (character % SHEET_COLUMNS) * SOURCE_CHAR_SIZE;
        src.y = (character / SHEET_COLUMNS) * SOURCE_CHAR_SIZE;

        SDL_RenderCopy(renderer, texture, &src, &dst);

        dst.x += TARGET_CHAR_SIZE;
    }

}


// Function to render info associated with the developer mode
void render_debug(SDL_Renderer* renderer, SDL_Texture* charset, Player* player, Camera* camera)
{

    char buffer[BASE_BUFFER_SIZE];
    // Show current action name
    sprintf(buffer, "ACTION: %s", player->current_action);
    render_text(renderer, charset, 0, SCREEN_HEIGHT - 2 * TARGET_CHAR_SIZE, buffer);

    // Visualize the Buffer (show last 5 keys)
    render_text(renderer, charset, 0, SCREEN_HEIGHT - TARGET_CHAR_SIZE, "BUFFER: ");
    for(int i = 0; i < 5; i++)
    {
        const char* keyname = SDL_GetKeyName(player->buffer[i].key);
        // If buffer is empty, print "-"
        if(player->buffer[i].key == 0)
        {
                keyname = "-";
        }
        sprintf(buffer, "%s", keyname);
        render_text(renderer, charset, (strlen("BUFFER: ") * TARGET_CHAR_SIZE) + (i * 16),
                  SCREEN_HEIGHT - TARGET_CHAR_SIZE, buffer);
    }

    if (player->attack_box.w > 0)
    {
        const SDL_Rect attack_box = {player->attack_box.x - camera->camera_x, player->attack_box.y, player->attack_box.w, player->attack_box.h};
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 100); // Green
        SDL_RenderDrawRect(renderer, &attack_box);
    }

}