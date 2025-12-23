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

	// Pre-game assets inits: SDL, windows, renderers, level and entities
	init_sdl();

	SDL_Window *window = init_window(TITLE, SCREEN_WIDTH, SCREEN_HEIGHT);
	SDL_Renderer *renderer = init_renderer(window, SCREEN_WIDTH, SCREEN_HEIGHT);
	SDL_Texture* tileset = init_texture("assets/brawler_tileset.bmp", renderer);
	SDL_Texture* charset = init_texture("assets/cs8x8.bmp", renderer);
	SDL_Texture* knight = init_texture("assets/knight_spritesheet.bmp", renderer);
	SDL_Texture* zombie = init_texture("assets/zombie_spritesheet.bmp", renderer);
	SDL_Texture* ghost = init_texture("assets/ghost_spritesheet.bmp", renderer);
	Level* current_level = load_level("assets/level/level1.txt");

	Player player
	{
		.texture = knight,
		.global_x = COLUMNS_PER_SCREEN * TARGET_TILE_SIZE / 2,
		.global_y = VISIBLE_ROWS * TARGET_TILE_SIZE / 2,
		.z = Z_GROUND_LEVEL,
		.z_velocity = NO_Z_VELOCITY,
		.screen_x = SCREEN_BEGINNING,
		.player_speed = PLAYER_SPEED,
		.is_moving = FALSE,
		.action_type = IDLE_PLAYER,
		.action_timer = TIMER_ZERO,
		.facing_right = TRUE,
		.buffer = {},
		.current_action = {0},
		.debug_mode = FALSE,
		.attack_box = {0, 0, 0, 0},
		.score_multiplier = 1,
		.last_score_time = 0,
		.score = 0,
		.hurt_timer = TIMER_ZERO,
		.health_points = PLAYER_MAX_HEALTH,
		.max_health_points = PLAYER_MAX_HEALTH
	};

	Camera camera
	{
		.camera_x = SCREEN_BEGINNING,
		.camera_y = SCREEN_BEGINNING,
		.camera_width = SCREEN_WIDTH,
		.camera_height = SCREEN_HEIGHT,
	};

	// Holder for all enemies entities
	Enemy enemies[5];

	// TO MODIFY AND FUNCTIONALIZE
	init_enemy(&enemies[0], zombie, ENEMY_TYPE_CHASER, 600, 800);
	init_enemy(&enemies[1], ghost, ENEMY_TYPE_CHARGER, 800, 400);

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
				player.texture = knight;
				player.global_x = COLUMNS_PER_SCREEN * TARGET_TILE_SIZE / 2;
				player.global_y = VISIBLE_ROWS * TARGET_TILE_SIZE / 2;
				player.z = Z_GROUND_LEVEL;
				player.z_velocity = NO_Z_VELOCITY;
				player.screen_x = SCREEN_BEGINNING;
				player.player_speed = PLAYER_SPEED;
				player.is_moving = FALSE;
				player.action_type = IDLE_PLAYER;
				player.action_timer = TIMER_ZERO;
				player.facing_right = TRUE;
				clear_buffer(&player);
				strcpy(player.current_action, "");
				player.debug_mode = FALSE;
				player.attack_box = {0, 0, 0, 0};
				player.score_multiplier = 1;
				player.last_score_time = 0;
				player.score = 0;
				player.hurt_timer = TIMER_ZERO;
				player.health_points = PLAYER_MAX_HEALTH;
				player.max_health_points = PLAYER_MAX_HEALTH;

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

		// Hitboxes and collisions
		update_hitboxes(&player);
		update_enemies(enemies, 5, &player);
		for (int i = 0; i < 5; i++)
		{
			// Check if Player hits Enemy
			check_if_enemy_hit(&player, &enemies[i], frame_start);
			// Check if Enemy hits Player
			check_if_player_hit(&player, &enemies[i]);
		}

		// End-tick renderings
		SDL_RenderClear(renderer);
		render_background(renderer, tileset, current_level, &camera);
		render_statbar(renderer, charset, &player, (frame_start - start_time) / 1000);

		// FOR TEST
		for(int i=0; i<5; i++) {
			// Use the new function instead of SDL_RenderFillRect
			render_enemy(renderer, &enemies[i], &camera);

			// Decrease enemy hurt timer manually here or in update_enemies
			if (enemies[i].hurt_timer > 0) enemies[i].hurt_timer--;

		}
		// FOR TEST

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
	SDL_Quit();

	return 0;

}
