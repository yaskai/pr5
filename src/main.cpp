#include "raylib.h"
#include "config.h"
#include "game.h"
#include "common/nums.h"
#include "message.h"

enum PLATFORMS : u8 {
	LINUX	= 0,
	WIN64	= 1,
	MACOS	= 2,
	EMWEB	= 4			// Web through emscripten/wasm
};

u32 platform_flags[] = {
	(0), 
	(FLAG_BORDERLESS_WINDOWED_MODE | FLAG_WINDOW_MAXIMIZED),
	(FLAG_FULLSCREEN_MODE), 
	(FLAG_VSYNC_HINT),
};
static const u8 platform = LINUX;
u32 window_flags = platform_flags[platform];	

void window_Open(Config *conf);
void main_Loop(Game *game, bool *exit);

int main() {
	SetTraceLogLevel(LOG_ERROR);
	msg_SetLogState(MSG_ERROR | MSG_INFO);

	Config conf = (Config) {0}; 
	conf_Init(&conf);
	conf_ReadFile(&conf, "options.conf");
	
	conf.ww = conf_GetOptionValue("window:width");
	conf.wh = conf_GetOptionValue("window:height");
	window_Open(&conf);

	Game game = (Game) {0};
	game_Init(&game, &conf);

	bool exit = false;
	main_Loop(&game, &exit);

	CloseWindow();
	game_Close(&game);

	return 0;
}

void window_Open(Config *conf) {
	InitWindow(conf->ww, conf->wh, "PR5");	
	SetExitKey(KEY_NULL);

	if(conf_GetOptionValue("graphics:vsync")) 		window_flags |= FLAG_VSYNC_HINT;
	if(conf_GetOptionValue("window:fullscreen"))	window_flags |= FLAG_FULLSCREEN_MODE;
	SetWindowState(window_flags);
}

void main_Loop(Game *game, bool *exit) {
	while(!*exit) {
		*exit = ( WindowShouldClose() ^ (game->flags & FLAG_EXIT_REQUEST) );	

		if(IsKeyPressed(KEY_V)) {
			window_flags ^= FLAG_VSYNC_HINT;
			if(window_flags & FLAG_VSYNC_HINT) SetWindowState(FLAG_VSYNC_HINT);
			else ClearWindowState(FLAG_VSYNC_HINT);
		}

		float dt = GetFrameTime();
		cl_game_Update(game, dt);
	}
}

