#define _USE_MATH_DEFINES
#include <SDL2/SDL.h>
#include "window.h"
#include "logic.h"
#include "defines.h"

int main()
{

	// Pre-game assets inits: SDL, windows, renderers, level and entities
	init_sdl();

	SDL_Window *window = init_window(TITLE, SCREEN_WIDTH, SCREEN_HEIGHT);
	SDL_Renderer *renderer = init_renderer(window, SCREEN_WIDTH, SCREEN_HEIGHT);
	SDL_Texture* tileset = init_texture("assets/brawler_tileset.bmp", renderer);
	SDL_Texture* charset = init_texture("assets/cs8x8.bmp", renderer);
	SDL_Texture* knight = init_texture("assets/knight_spritesheet.bmp", renderer);
	SDL_Texture* zombie = init_texture("assets/zombie_spritesheet.bmp", renderer);
	SDL_Texture* ghost = init_texture("assets/ghost_spritesheet.bmp", renderer);
	const Level* current_level = load_level("assets/level/level1.txt", zombie, ghost);

	Player player {};
	init_player(&player, knight);

	Camera camera {};
	init_camera(&camera);

	SDL_Event event;
	int quit = false;
	Uint32 start_time = SDL_GetTicks();

	// Main game loop
	while (!quit)
	{
		const Uint32 frame_start = SDL_GetTicks();

		// Every-tick event polling
		while (SDL_PollEvent(&event) != 0)
		{
			if (event.type == SDL_QUIT || event.key.keysym.sym == SDLK_ESCAPE)
			{
				quit = true;
			}
			if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_n)
			{
				// Deload and reload current level
				free_level(current_level);
				current_level = load_level("assets/level/level1.txt", zombie, ghost);

				// Reset player entity
				reset_player_state(&player);

				// Reset camera entity
				init_camera(&camera);

				start_time = SDL_GetTicks();
			}
			handle_input_event(&player, &event, current_level);
		}
		// Player movement and camera handling
		const Uint8* currentKeyStates = SDL_GetKeyboardState(nullptr);
		handle_player_movement(&player, current_level, currentKeyStates);
		player.screen_x = player.global_x - camera.camera_x;
		handle_camera_movement(&player, current_level, &camera);

		// Hitboxes and collisions
		update_hitboxes(&player);
		update_enemies(current_level->enemies, &player, current_level);
		for (int i {0}; i < current_level->enemy_count; i++)
		{
			// Check if Player hits Enemy
			check_if_enemy_hit(&player, &current_level->enemies[i], frame_start);
			// Check if Enemy hits Player
			check_if_player_hit(&player, &current_level->enemies[i]);
		}

		// Stage completion check
		if (check_stage_completion(&player, current_level))
		{
			printf("STAGE CLEARED!\n");
			quit = true;
		}

		// End-tick renderings
		SDL_RenderClear(renderer);
		render_background(renderer, tileset, current_level, &camera);
		render_statusbar(renderer, charset, &player, (frame_start - start_time) / 1000);

		for(int i=0; i<current_level->enemy_count; i++)
		{
			render_enemy(renderer, &current_level->enemies[i], &camera);
		}

		if (player.debug_mode)
		{
			render_debug(renderer, charset, &player, &camera);
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
	SDL_DestroyTexture(knight);
	SDL_DestroyTexture(zombie);
	SDL_DestroyTexture(ghost);
	SDL_Quit();

	return 0;

}
