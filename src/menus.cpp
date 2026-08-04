#include <cstdio>
#include "raylib.h"
#include "raymath.h"
#include "game.h"
#include "ui.h"

bool val = false;
bool full_screen = true;

void game_MainMenu(Game *game) {
	if(ui_Button( (Rectangle) { 100, game->conf->wh * 0.5f, 300, 64 }, "Start")) {
		game_StartNew(game);
		return;
	}

	ui_CheckBox( (Rectangle) { 0, 0, 300, 64 }, "checkbox", &val);	
}

void game_PauseMenu(Game *game) {
	if(ui_Button( (Rectangle) { 100, game->conf->wh * 0.5f, 300, 100 }, "Resume") )		
		game->pause = false;

	if(!game->pause) {
		input_DisableMouse(24);
		return;
	}
	
	if(ui_Button( (Rectangle) { 100, game->conf->wh * 0.5f + 100, 300, 100 }, "Quit") )		
		game->flags |= FLAG_EXIT_REQUEST;

	game_OptionsMenu(game);
}

static const char* resolutions_16x9[] = {
	"1280x720",			// 0
	"1600x900",			// 1
	"1920x1080",		// 2
	//"2560x1440",		// 3
	//"3840x2160"			// 4
};
u8 res_id = 2;
//u8 res_count = 5;
u8 res_count = 3;


void game_OptionsMenu(Game *game) {
	if(ui_Button( (Rectangle) { game->conf->wh*0.75f, game->conf->wh*0.5f, 300, 100 }, resolutions_16x9[res_id])) {
		res_id = (res_id + 1) % res_count;
	}

	ui_CheckBox( (Rectangle) { game->conf->wh*0.75f, game->conf->wh*0.75f, 300, 100 }, "fullscreen", &full_screen);	

	if(ui_Button( (Rectangle) { game->conf->wh*0.75f, game->conf->wh*0.85f, 300, 100 }, "apply")) {
		int w, h;
		sscanf(resolutions_16x9[res_id], "%dx%d", &w, &h); 
		game_OpenDrawBuffer(game, w, h);

		game_TogglePause(game);
		game_QueuePause(game, 16);
	}
}

