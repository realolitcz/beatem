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

    switch (player->action_type)
	{
		case 0:
			src = {0, 0, SOURCE_TILE_SIZE, SOURCE_TILE_SIZE};
			break;
		case 1:
			src ={0, 16, SOURCE_TILE_SIZE, SOURCE_TILE_SIZE};
			break;
		case 2:
			src = {0, 32, SOURCE_TILE_SIZE, SOURCE_TILE_SIZE};
			break;
		case 3:
			src = {16, 0, SOURCE_TILE_SIZE, SOURCE_TILE_SIZE};
			break;
		case 4:
			src = {16, 16, SOURCE_TILE_SIZE, SOURCE_TILE_SIZE};
        default:
            ;
    }

    const SDL_Rect dst
	{
		player->global_x - camera->camera_x,
		player->global_y - static_cast<int>(player->z),
		TARGET_TILE_SIZE,
		TARGET_TILE_SIZE,
	};

    SDL_RenderCopy(renderer, player->texture, &src, &dst);

}


// Helper function for render_text to prepare label:value lines for stats
void render_stat(SDL_Renderer* renderer, SDL_Texture* charset, const int x, const int y, const char* label,
                 const int value)
{

    char buffer[BASE_BUFFER_SIZE];

    sprintf(buffer, "%s: %d", label, value);

    render_text(renderer, charset, x, y, buffer);

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
void render_debug(SDL_Renderer* renderer, SDL_Texture* charset, Player* player)
{

    char buffer[BASE_BUFFER_SIZE];
    sprintf(buffer, "ACTION: %s", player->current_action);
    // Show current action name
    render_text(renderer, charset, 0, SCREEN_HEIGHT - 2 * TARGET_CHAR_SIZE, buffer);

    // Visualize the Buffer (show last 5 keys)
    render_text(renderer, charset, 0, SCREEN_HEIGHT - TARGET_CHAR_SIZE, "BUFFER: ");
    for(int i = 0; i < 5; i++)
    {
        const char* keyname = SDL_GetKeyName(player->buffer[i].key);
        // If buffer is empty (0), print "-"
        if(player->buffer[i].key == 0)
        {
                keyname = "-";
        }
        sprintf(buffer, "%s", keyname);
        render_text(renderer, charset, (strlen("BUFFER: ") * TARGET_CHAR_SIZE) + (i * 16),
                  SCREEN_HEIGHT - TARGET_CHAR_SIZE, buffer);
    }

}