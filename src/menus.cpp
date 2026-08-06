#include <cstdio>
#include <cstdlib>
#include "config.h"
#include "raylib.h"
#include "raymath.h"
#include "game.h"
#include "ui.h"
#include "window.h"

enum MENU_STATES : u8 {
	MENU_HOME,
	MENU_SETTINGS_GFX,
	MENU_SETTINGS_INPUT,
};
u8 menu_state;

// Common 16:9 resolutions
static const char* resolutions_16x9[] = {
	"1280x720",			// 0
	"1600x900",			// 1
	"1920x1080",		// 2
	"2560x1440",		// 3
	"3840x2160"			// 4
};
u8 res_count = 5;

UI_DirButtonData res_button_data;
bool full_screen, vsync;

// Set some initial menu values
void menus_Init() {
	full_screen = conf_GetOptionValue("window:fullscreen");
	vsync = conf_GetOptionValue("graphics:vsync");

	menu_state = MENU_HOME;

	res_button_data = (UI_DirButtonData) {
		.text = resolutions_16x9,
		.count = res_count,
		.idx = 2
	};
}

// Title screen menu loop
void game_MainMenu(Game *game) {
	if(ui_Button( (Rectangle) { 100, game->conf->wh * 0.5f, 300, 64 }, "Start")) {
		game_StartNew(game);
		return;
	}
}

void game_PauseMenu(Game *game) {
	if(ui_Button( (Rectangle) { 100, game->conf->wh * 0.5f, 300, 100 }, "Resume") ) {
		game->pause = false;
		menu_state = 0;
	}

	/*
	if(ui_Button( (Rectangle) { 100, game->conf->wh * 0.65f, 300, 100 }, "Settings") ) {
		++menu_state;
	}
	*/

	if(!game->pause) {
		input_DisableMouse(24);
		return;
	}
	
	if(ui_Button( (Rectangle) { 100, game->conf->wh * 0.5f + 100, 300, 100 }, "Quit") )		
		game->flags |= FLAG_EXIT_REQUEST;

	game_OptionsMenu(game);
}


void game_OptionsMenu(Game *game) {
	// Resolution option button 
	ui_DirectionalButton( (Rectangle) { game->conf->wh*0.75f, game->conf->wh*0.25f, 300, 100 }, &res_button_data);

	// FULLSCREEN checkbox 
	ui_CheckBox( (Rectangle) { game->conf->wh*0.75f, game->conf->wh*0.5f, 300, 100 }, "fullscreen", &full_screen);	
	if(full_screen != window_HasFlag(FLAG_FULLSCREEN_MODE))	window_ToggleFlag(FLAG_FULLSCREEN_MODE);

	// VSYNC checkbox
	ui_CheckBox( (Rectangle) { game->conf->wh*0.75f, game->conf->wh*0.75f, 300, 100 }, "vsync", &vsync);	
	if(vsync != window_HasFlag(FLAG_VSYNC_HINT)) window_ToggleFlag(FLAG_VSYNC_HINT);

	// Apply options button
	if(ui_Button( (Rectangle) { game->conf->wh*0.75f, game->conf->wh*0.85f, 300, 100 }, "apply")) {
		int w, h;
		sscanf(resolutions_16x9[res_button_data.idx], "%dx%d", &w, &h); 

		// Only apply resolution if actually needed
		if(w != game->buffer.texture.width && h != game->buffer.texture.height) {
			game_OpenDrawBuffer(game, w, h);
			// Unpause game to render the resolution change
			game_TogglePause(game);
			// Trigger a pause for later in a few frames to keep ux continuity 
			game_QueuePause(game, 16);
		}
	}
}

