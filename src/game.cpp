#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include "config.h"
#include "ent.h"
#include "input.h"
#include "message.h"
#include "r_map.h"
#include "game.h"
#include "geo.h"
#include "bsp.h"
#include "cam.h"
#include "r_skybox.h"
#include "ui.h"

#define CLIP_NEAR 	2.0
#define CLIP_FAR 	40000.0

RenderTexture2D buffer;
Cam_Controller cc;
r_Map rmap;
Skybox skybox;

Font font;

int fps, fps_tick = 0;

bool val = false;
bool pause = false;

// Game initialization
void game_Init(Game *game, Config *conf) {
	msg_Print("game_Init()", ANSI_BLUE);

	// Set config pointer
	game->conf = conf;

	buffer = LoadRenderTexture(conf_GetOptionValue("graphics:res_x"), conf_GetOptionValue("graphics:res_y"));
	SetTextureFilter(buffer.texture, TEXTURE_FILTER_POINT);

	// Init camera struct 
	game->camera = (Camera3D) {
		.position = (Vector3) { 0, 0, 0 },
		.target = (Vector3) { -1, 0, 0 },
		.up = WORLD_UP,
		.fovy = 90.0f,
		.projection = CAMERA_PERSPECTIVE
	};

	rlSetClipPlanes(CLIP_NEAR, CLIP_FAR);

	// Input setup
	game->input_handler = (InputHandler) {0};
	input_Init(&game->input_handler);

	game->ent_handler = (Ent_Handler) {0};

	const char *font_path = "resources/fonts/zed_mono.ttf";
	font = LoadFontEx(font_path, 128, NULL, 0);
	SetTextureFilter(font.texture, TEXTURE_FILTER_POINT);

	ui_Init(font);

	skybox = skybox_Init("resources/skybox_02.png", 1024, 0);
}

// Unload game state
void game_Close(Game *game) {
	msg_Print("game_Close()", ANSI_BLUE);

	rmap.Unload();
	bsp_Unload(&game->bsp);
	ent_handler_Close(&game->ent_handler);
	skybox_Close(&skybox);

	UnloadRenderTexture(buffer);
}

// Per-frame update
void cl_game_Update(Game *game, float dt) {
	if(!pause) {
		input_Poll(&game->input_handler);
		ent_Handler_Update(&game->ent_handler, dt);
	}

	if(IsKeyPressed(KEY_ESCAPE)) {
		pause = !pause;

		if(pause) {
			input_EnableMouse();	
		} else {
			DisableCursor();
			GetMouseDelta();
			game->input_handler.cursor_delta = (Vector2) { 0, 0 };
			GetMouseDelta();
			input_LockMouse(60);
		}
	}

	if(game->state == STATE_TITLE && IsKeyDown(KEY_Y)) {
		game_StartNew(game);
	} 

	if(!pause) { 
		game->accumulator += dt;

		while(game->accumulator >= TICK) {
			cl_game_Tick(game, TICK);
			game->accumulator -= TICK;
		}
	}

	float alpha = game->accumulator / TICK;
	game_Render(game, alpha);
}

// Logic update 
void cl_game_Tick(Game *game, float tick_dt) {
	fps_tick++;
	if(fps_tick >= 4) {
		fps = GetFPS();
		fps_tick = 0;
	}
	
	input_Tick(&game->input_handler);

	for(u16 i = 0; i < game->ent_handler.num_ents; i++) {
		Entity *ent = &game->ent_handler.ents[i];
		ent->ct_prev = ent->ct;
	}

	ent_handler_Tick(&game->ent_handler, &game->bsp, tick_dt);
}

// Draw step
void game_Render(Game *game, float alpha) {
	if(game->ent_handler.cl_player_id && !pause) { 
		cc_Resolve(&cc, &game->ent_handler, alpha);
	}

	BeginDrawing();
	BeginTextureMode(buffer);
	if(!pause) ClearBackground(BLACK);

	game_Draw3D(game, alpha);
	//game_Draw2D(game, alpha);

	EndTextureMode();

	DrawTexturePro(
			buffer.texture, 
			(Rectangle) { 0, 0, (f32)buffer.texture.width, -(f32)buffer.texture.height },
			(Rectangle) { 0, 0, game->conf->ww, game->conf->wh }, 
			Vector2Zero(), 
			0.0f, 
			WHITE
			);

	game_Draw2D(game, alpha);

	EndDrawing();
}

void game_Draw3D(Game *game, float alpha) {
	if(!game->state || pause) return;

	BeginMode3D(game->camera);

	skybox_Render(&skybox);	

	rmap.Draw(&game->bsp, game->camera.position);
	ent_Handler_Render(&game->ent_handler, alpha);

	EndMode3D();
}

void game_Draw2D(Game *game, float alpha) {
	if(game->state == STATE_TITLE) {
		game_MainMenu(game);
		return;
	}	

	if(pause) game_PauseMenu(game);

	if(conf_GetOptionValue("graphics:show_fps")) { 
		ui_Label( (Rectangle) { 0, 0, 144, 32 }, "");
		DrawTextEx(font, TextFormat("FPS: %d", fps), (Vector2) { 4, 1 }, 32, 1, RAYWHITE);
	}
}

void game_StartNew(Game *game) {
	input_DisableMouse(60);

	game->state = STATE_MAIN;

	game->bsp = bsp_LoadFile("resources/maps/test/gym.bsp");

	ent_handler_Init(&game->ent_handler, &game->bsp);
		
	cc = cc_Init(&game->camera, game->ent_handler.cl_player_id);	

	rmap.Build(&game->bsp);
}

void game_MainMenu(Game *game) {
	if(ui_Button( (Rectangle) { 100, game->conf->wh * 0.5f, 300, 64 }, "Start")) {
		game_StartNew(game);
		return;
	}

	ui_CheckBox( (Rectangle) { 0, 0, 300, 64 }, "checkbox", &val);	
}

void game_PauseMenu(Game *game) {
	if(ui_Button( (Rectangle) { 100, game->conf->wh * 0.5f, 300, 100 }, "Resume") )		
		pause = false;

	if(!pause) {
		input_DisableMouse(24);
		return;
	}
	
	if(ui_Button( (Rectangle) { 100, game->conf->wh * 0.5f + 100, 300, 100 }, "Quit") )		
		game->flags |= FLAG_EXIT_REQUEST;
}

