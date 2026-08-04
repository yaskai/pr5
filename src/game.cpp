#include <cstdio>
#include <stdint.h>
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

Cam_Controller cc;
r_Map rmap;
Skybox skybox;

Font font;

int fps; 
float fps_tick = 0;
int fps_min = INT32_MAX, fps_max = 0;
std::vector<int> fps_vals;
int fps_avg;

int pause_queue = 0;

void log_Fps() {
	int frame_sum = 0;
	for(int n : fps_vals) frame_sum += n;

	fps_avg = frame_sum / fps_vals.size();
	
	printf("fps average: %d\n", fps_avg);
	printf("lowest: %d\n", fps_min);
	printf("highest: %d\n", fps_max);
}

// Game initialization
void game_Init(Game *game, Config *conf) {
	msg_Print("game_Init()", ANSI_BLUE);

	// Set config pointer
	game->conf = conf;

	game_OpenDrawBuffer(game, conf_GetOptionValue("graphics:res_x"), conf_GetOptionValue("graphics:res_y")); 

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

void game_OpenDrawBuffer(Game *game, int width, int height) {
	if(IsTextureValid(game->buffer.texture)) UnloadRenderTexture(game->buffer);	
	game->buffer = LoadRenderTexture(width, height);
	SetTextureFilter(game->buffer.texture, TEXTURE_FILTER_POINT);
}

// Unload game state
void game_Close(Game *game) {
	msg_Print("game_Close()", ANSI_BLUE);

	rmap.Unload();
	bsp_Unload(&game->bsp);
	ent_handler_Close(&game->ent_handler);
	skybox_Close(&skybox);

	UnloadRenderTexture(game->buffer);

	log_Fps();
}

// Per-frame update
void cl_game_Update(Game *game, float dt) {
	if(!game->pause) {
		input_Poll(&game->input_handler);
		ent_Handler_Update(&game->ent_handler, dt);
	}

	if(IsKeyPressed(KEY_ESCAPE)) game_TogglePause(game);

	if(game->state == STATE_TITLE && IsKeyDown(KEY_Y)) {
		game_StartNew(game);
	} 

	if(!game->pause) { 
		game->accumulator += dt;

		while(game->accumulator >= TICK) {
			cl_game_Tick(game, TICK);
			game->accumulator -= TICK;
		}
	}

	float alpha = game->accumulator / TICK;
	game_Render(game, alpha);

	if(game->flags & FLAG_PAUSE_REQUEST && !game->pause) {
		pause_queue--;	
		if(pause_queue <= 0) {
			game->flags &= ~FLAG_PAUSE_REQUEST;
			pause_queue = 0;
			game_TogglePause(game);
		}
	}
}

// Logic update 
void cl_game_Tick(Game *game, float tick_dt) {
	input_Tick(&game->input_handler);

	for(u16 i = 0; i < game->ent_handler.num_ents; i++) {
		Entity *ent = &game->ent_handler.ents[i];
		ent->ct_prev = ent->ct;
	}

	ent_handler_Tick(&game->ent_handler, &game->bsp, tick_dt);
}

// Draw step
void game_Render(Game *game, float alpha) {
	fps_tick += 16.6f * GetFrameTime();
	if(fps_tick >= 1.0f) {
		fps = GetFPS();
		fps_tick = 0;

		if(game->state) {
			fps_vals.push_back(fps);
			if(fps < fps_min) fps_min = fps;
			if(fps > fps_max) fps_max = fps;
		}
	}

	if(game->ent_handler.cl_player_id && !game->pause) { 
		cc_Resolve(&cc, &game->ent_handler, alpha);
	}

	BeginDrawing();
	BeginTextureMode(game->buffer);
	if(!game->pause) ClearBackground(BLACK);

	game_Draw3D(game, alpha);
	//game_Draw2D(game, alpha);

	EndTextureMode();

	DrawTexturePro(
			game->buffer.texture, 
			(Rectangle) { 0, 0, (f32)game->buffer.texture.width, -(f32)game->buffer.texture.height },
			(Rectangle) { 0, 0, game->conf->ww, game->conf->wh }, 
			Vector2Zero(), 
			0.0f, 
			WHITE
			);

	game_Draw2D(game, alpha);

	EndDrawing();
}

void game_Draw3D(Game *game, float alpha) {
	if(!game->state || game->pause) return;

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

	if(game->pause) game_PauseMenu(game);

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

void game_TogglePause(Game *game) {
		game->pause = !game->pause;

		if(game->pause) 
			input_EnableMouse();	
		else 
			input_DisableMouse(0);
}

void game_QueuePause(Game *game, int frames) {
	pause_queue = frames;
	game->flags |= FLAG_PAUSE_REQUEST;
}

