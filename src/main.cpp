#include "raylib.h"
#include "config.h"
#include "game.h"
#include "common/nums.h"
#include "message.h"
#include "window.h"

void main_Loop(Game *game, bool *exit);

int main() {
	SetTraceLogLevel(LOG_ERROR);
	msg_SetLogState(MSG_ERROR | MSG_INFO);

	Config conf = (Config) {0}; 
	conf_Init(&conf);
	conf_ReadFile(&conf, "options.conf");
	
	conf.ww = conf_GetOptionValue("window:width");
	conf.wh = conf_GetOptionValue("window:height");
	window_Open();
	menus_Init();

	Game game = (Game) {0};
	game_Init(&game, &conf);

	bool exit = false;
	main_Loop(&game, &exit);

	CloseWindow();
	game_Close(&game);

	return 0;
}

void main_Loop(Game *game, bool *exit) {
	while(!*exit) {
		*exit = ( WindowShouldClose() ^ (game->flags & FLAG_EXIT_REQUEST) );	

		float dt = GetFrameTime();
		cl_game_Update(game, dt);
	}
}

