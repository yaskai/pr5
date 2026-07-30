#pragma once

#include "raylib.h"
#include "bsp.h"
#include "common/nums.h"
#include "config.h"
#include "input.h"
#include "ent.h"

enum game_states : u8 {
	STATE_TITLE,
	STATE_MAIN,
};

#define TICK (1.0f / 66.0f)

#define FLAG_EXIT_REQUEST	0x01

typedef struct {
	Bsp_Data bsp;

	Camera3D camera;

	Config *conf;

	InputHandler input_handler;
	Ent_Handler ent_handler;

	float accumulator;

	u8 state, flags;

} Game;

void game_Init(Game *game, Config *conf);
void game_Close(Game *game);

void cl_game_Update(Game *game, float dt);
void cl_game_Tick(Game *game, float tick_dt);

void sv_game_Update(Game *game, float dt);
void sv_game_Tick(Game *game, float tick_dt);

void game_Render(Game *game, float alpha);
void game_Draw3D(Game *game, float alpha);
void game_Draw2D(Game *game, float alpha);

void game_StartNew(Game *game);

void game_MainMenu(Game *game);
void game_PauseMenu(Game *game);

