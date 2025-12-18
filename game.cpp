#define _USE_MATH_DEFINES
#include <math.h>
#include <stdexcept>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <SDL2/SDL.h>
#include "window.h"
#include "logic.h"
#include "defines.h"

int main()
{
	// Pre-game assets inits: sdl, windows, renderers, level and entities
	init_sdl();

	SDL_Window *window = init_window(TITLE, SCREEN_WIDTH, SCREEN_HEIGHT);
	SDL_Renderer *renderer = init_renderer(window, SCREEN_WIDTH, SCREEN_HEIGHT);
	SDL_Texture* tileset = init_texture("assets/brawler_tileset.bmp", renderer);
	SDL_Texture* charset = init_texture("assets/cs8x8.bmp", renderer);
	Level* current_level = load_level("assets/level/level1.txt");

	Player player
	{
		init_texture("assets/knight.bmp", renderer), // Spritesheet
		COLUMNS_PER_SCREEN * TARGET_TILE_SIZE / 2,        // Absolute x position in the level
		VISIBLE_ROWS * TARGET_TILE_SIZE / 2,              // Absolute y position in the level
		Z_GROUND_LEVEL,                                   // Height above "ground level"
		NO_Z_VELOCITY,                                    // Change-rate of player height
		0,                                                // X position within given screen
		PLAYER_SPEED,                                     // Current player velocity
		IDLE_PLAYER,                                      // Marker of currently conducted action
		0,                                                // Time of currently performed action
		TRUE,                                             // Is player facing right?
		NULL,                                             // History of last 10 inputs
		NULL,                                             // Name of current action
		FALSE                                             // Debug overlay toggle
	};

	Camera camera
	{
		SCREEN_BEGINNING,
		SCREEN_BEGINNING,
		SCREEN_WIDTH,
		SCREEN_HEIGHT,
	};

	SDL_Event event;
	int quit = FALSE;
	Uint32 start_time = SDL_GetTicks();

	// Main game loop
	while (quit != TRUE)
	{
		const Uint32 frame_start = SDL_GetTicks();

		// Every-tick event polling
		while (SDL_PollEvent(&event) != 0)
		{
			if (event.type == SDL_QUIT || event.key.keysym.sym == SDLK_ESCAPE)
			{
				quit = 1;
			}
			if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_n)
			{
				// Deload and reload current level
				free_level(current_level);
				current_level = load_level("assets/level/level1.txt");
				// Reset player entity
				player.global_x = COLUMNS_PER_SCREEN * TARGET_TILE_SIZE / 2;
				player.global_y = VISIBLE_ROWS * TARGET_TILE_SIZE / 2;
				player.z = Z_GROUND_LEVEL;
				player.z_velocity = NO_Z_VELOCITY;
				player.action_type = IDLE_PLAYER;
				player.action_timer = 0;
				// Reset camera entity
				camera.camera_x = SCREEN_BEGINNING;
				camera.camera_y = SCREEN_BEGINNING;

				start_time = SDL_GetTicks();
			}
			handle_input_event(&player, &event, current_level);
		}
		// Player movement and camera handling
		const Uint8* currentKeyStates = SDL_GetKeyboardState(nullptr);
		handle_player_movement(&player, current_level, currentKeyStates);
		player.screen_x = player.global_x - camera.camera_x;
		handle_camera_movement(&player, current_level, &camera);

		// End-tick renderings
		SDL_RenderClear(renderer);

		render_background(renderer, tileset, current_level, &camera);

		render_stat(renderer, charset, SCREEN_BEGINNING, SCREEN_BEGINNING,  "X POSITION",
		       player.global_x);
		render_stat(renderer, charset, SCREEN_BEGINNING, SCREEN_BEGINNING + TARGET_CHAR_SIZE, "Y POSITION",
			   player.global_y);
		render_stat(renderer, charset, SCREEN_BEGINNING, SCREEN_BEGINNING + 2 * TARGET_CHAR_SIZE, "TIME",
			   (frame_start - start_time) / 1000);

		if (player.debug_mode)
		{
			render_debug(renderer, charset, &player);
		}

		render_player(renderer, &player, &camera);

		SDL_RenderPresent(renderer);

		// End-tick handlers
		handle_gravity(&player);
		handle_attack(&player);

		// Tick-end
		Uint32 frame_time = SDL_GetTicks() - frame_start;
		if (frame_time < 16)
		{
			// Ensure circa 60 FPS
			SDL_Delay(16 - frame_time);
		}
	}

	// Post-game clean-up
	free_level(current_level);
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyTexture(tileset);
	SDL_DestroyTexture(charset);
	SDL_Quit();

	return 0;

}
