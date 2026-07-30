#pragma once

#include "raylib.h"
#include "common/nums.h"
#include "config.h"

enum ACTION_STATES : u8 {
	INPUT_UP			= 0,
	INPUT_DOWN			= 1,
	INPUT_PRESSED		= 2,
	INPUT_RELEASED		= 3
};

enum ACTION_IDS : u8 {
	ACT_NULL		= 0,
	ACT_MOV_FWD 	= 1,
	ACT_MOV_BACK	= 2,
	ACT_MOV_LEFT	= 3,
	ACT_MOV_RIGHT	= 4,
	ACT_JUMP		= 5,
};

typedef struct {
	int key, mouse_button;
	u8 state;
	u8 id;

} InputAction;

typedef struct {
	Vector2 cursor_pos;
	Vector2 cursor_delta;

	float mouse_sensitivity;
	
	InputAction actions[16];
	u8 num_actions; 

} InputHandler;

void input_Init(InputHandler *input);
void input_CreateAction(u8 action_id, int key, int mouse_button);

void input_Poll(InputHandler *input);
void input_Tick(InputHandler *input);

u8 input_GetActionState(u8 action_id);

Vector2 input_MousePos();
Vector2 input_MouseDelta();

void input_LockMouse(int frames);

InputHandler* input_GetPointer();

