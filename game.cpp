// File: game.cpp
// Header: N/A
// Author: Oliwer A. Gużewski (s208004)

// Support for legacy SDL versions provided by Dr Ostrowski
// extern "C" {
// #include"./SDL2-2.0.10/include/SDL.h"
// #include"./SDL2-2.0.10/include/SDL_main.h"

#include <SDL2/SDL.h>
#include "window.h"
#include "logic.h"
#include "defines.h"

// Main wrapper and orchestrator for the game
int main()
{

	// Initialize main SDL subsystem
	init_sdl();

	// Declare and initialize window and associated renderer for the game
	SDL_Window *window {init_window(TITLE, SCREEN_WIDTH, SCREEN_HEIGHT)};
	SDL_Renderer *renderer {init_renderer(window, SCREEN_WIDTH, SCREEN_HEIGHT)};

	// Declare struct for game assets textures and populate it with appropriate helper
	TextureAssets assets {};
	init_texture_assets(renderer, &assets);

	// Declare struct for game session information holder and populate it with appropriate helper
	GameSession session {};
	init_game_session(&session);
	// Load the game rules
	load_game_actions(&session);

	// Declare struct for player information populate it with appropriate helper
	Player player {};
	init_player(&player, assets.knight, session.actions);

	// Prepare array with level-config files
	const char* level_files[] {LVL1_FILE, LVL2_FILE};

	// Declare pointer for currently played level
	Level* current_level {nullptr};
	current_level = load_level(level_files[0], &assets);

	// Declare struct for camera information populate it with appropriate helper
	Camera camera {};
	init_camera(&camera);

	SDL_Event event;
	int quit = false;
	const Uint32 start_time = SDL_GetTicks();

	// Main game loop
	while (!quit)
	{
		const Uint32 frame_start = SDL_GetTicks();

		// Every-tick event polling
		while (SDL_PollEvent(&event) != 0)
		{
			if (event.type == SDL_QUIT || (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE))
			{
				quit = true;
			}

			// Indicator of state of game before any input processing occurs this frame
			const GameState prev_state = session.state;

            if (session.state == STATE_GAMEPLAY)
            {
                if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_n)
                {
                     change_level(&current_level, level_files[session.current_level_index], &assets, &player, &camera, true);
                }
                handle_input_event(&player, &event, current_level);
            }
            else
            {
                handle_ui_input(&session, &event);
            }

			// If game state changed
			if (session.state != prev_state)
			{
				// Finished entering name -> View Scores
				if (prev_state == STATE_NAME_INPUT && session.state == STATE_SCORES)
				{
					save_score(session.player_name, player.score);

					// Reload scores
					delete[] session.high_scores;
					session.high_scores = load_scores(&session.total_scores);

					// Full Reset to Menu background
					reset_game_to_menu(&current_level, level_files[0], &assets, &player, &camera, &session);
				}
				// Game Over -> Retry OR Menu
				else if (prev_state == STATE_GAME_OVER)
				{
					if (session.state == STATE_GAMEPLAY)
					{
						// Reload current level
						change_level(&current_level, level_files[session.current_level_index], &assets, &player, &camera, true);
					}
					else if (session.state == STATE_MENU)
					{
						// Reset to Menu background
						reset_game_to_menu(&current_level, level_files[0], &assets, &player, &camera, &session);
					}
				}
			}
        }

		// Whilst in gameplay
		if (session.state == STATE_GAMEPLAY)
		{
			update_gameplay(&session, &player, &current_level, &camera, &assets, level_files, MAX_LEVELS, frame_start);
		}

		SDL_RenderClear(renderer);

		// Every-tick rendering
		render_game(renderer, &session, &player, current_level, &camera, &assets, (frame_start - start_time) / 1000);

		SDL_RenderPresent(renderer);

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
	SDL_DestroyTexture(assets.tileset);
	SDL_DestroyTexture(assets.charset);
	SDL_DestroyTexture(assets.knight);
	SDL_DestroyTexture(assets.zombie);
	SDL_DestroyTexture(assets.ghost);
	SDL_Quit();

	return 0;

}
