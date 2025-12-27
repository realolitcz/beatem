#ifndef BEATEM_WINDOW_H
#define BEATEM_WINDOW_H

#include <SDL2/SDL.h>
#include "logic.h"

struct Camera
{
    int camera_x;
    int camera_y;
    int camera_width;
    int camera_height;
};

struct TextureAssets
{
    SDL_Texture* knight;
    SDL_Texture* zombie;
    SDL_Texture* ghost;
    SDL_Texture* tileset;
    SDL_Texture* charset;
};

void init_sdl();
SDL_Window* init_window(const char* title, int width, int height);
SDL_Renderer* init_renderer(SDL_Window* window, int width, int height);
SDL_Texture* init_texture(const char* path, SDL_Renderer* renderer);
void init_texture_assets(SDL_Renderer* renderer, TextureAssets* assets);
void init_camera(Camera* camera);
Level* load_level(const char* path, const TextureAssets* assets);
void free_level(const Level* level);
void change_level(Level** current_level_ptr, const char* path, const TextureAssets* assets, Player* player,
                  Camera* camera, bool reset_stats);
void render_menu_background(SDL_Renderer* renderer, SDL_Texture* tileset);
void render_menu(SDL_Renderer* renderer, SDL_Texture* charset, const GameSession* session);
void render_name_input(SDL_Renderer* renderer, SDL_Texture* charset, const GameSession* session);
void render_game_over(SDL_Renderer* renderer, SDL_Texture* charset, const Player* player);
void render_background(SDL_Renderer* renderer, SDL_Texture* texture, const Level* level, const Camera* camera);
void render_player(SDL_Renderer* renderer, const Player* player, const Camera* camera);
void render_enemy(SDL_Renderer* renderer, const Enemy* enemy, const Camera* camera);
void render_stat(SDL_Renderer* renderer, SDL_Texture* charset, int x, int y, const char* label, int value);
void render_statusbar(SDL_Renderer* renderer, SDL_Texture* charset, const Player* player, Uint32 current_time);
void render_multiplier(SDL_Renderer* renderer, SDL_Texture* charset, const Player* player);
void render_health_bar(SDL_Renderer* renderer, const Player* player);
void render_text(SDL_Renderer* renderer, SDL_Texture* texture, int x, int y, const char* text, float scale);
void render_debug(SDL_Renderer* renderer, SDL_Texture* charset, const Player* player, const Camera* camera);
void render_highscores(SDL_Renderer* renderer, SDL_Texture* charset, const GameSession* session);
void render_game(SDL_Renderer* renderer, const GameSession* session, const Player* player,
                 const Level* level, const Camera* camera, const TextureAssets* assets, Uint32 game_time);

#endif //BEATEM_WINDOW_H