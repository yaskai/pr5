#include <unordered_map>
#include "config.h"
#include "raylib.h"
#include "raymath.h"
#include "input.h"

InputHandler *ptr_input_self = NULL;
std::unordered_map<u8, u8> action_map;

int mouse_lock = 0;

void input_Init(InputHandler *input) {
	ptr_input_self = input;

	input->mouse_sensitivity = (float)conf_GetOptionValue("input:ms") * 0.000001f;

	input_CreateAction(ACT_MOV_FWD, 	KEY_W, 0);
	input_CreateAction(ACT_MOV_LEFT,	KEY_A, 0);
	input_CreateAction(ACT_MOV_BACK, 	KEY_S, 0);
	input_CreateAction(ACT_MOV_RIGHT, 	KEY_D, 0);
	input_CreateAction(ACT_JUMP,	KEY_SPACE, 0);	

	action_map.reserve(input->num_actions);

	for(u8 i = 0; i < input->num_actions; i++) {
		InputAction *action = &input->actions[i];
		action_map[action->id] = i;
	}
}

// Create a new input action and push it to actions stack
void input_CreateAction(uint8_t action_id, int key, int mouse_button) {
	InputHandler *input = input_GetPointer();
	InputAction action = (InputAction) { key, mouse_button, 0, action_id };

	input->actions[input->num_actions++] = action;
}

void input_Poll(InputHandler *input) {
	input->cursor_pos = GetMousePosition();
	
	if(mouse_lock > 0) {
		GetMouseDelta();
		input->cursor_delta = Vector2Zero();
	} else {
		input->cursor_delta = Vector2Scale(GetMouseDelta(), input->mouse_sensitivity);
	}

	for(u8 i = 0; i < input->num_actions; i++) {
		InputAction *action = &input->actions[i];
		if(!action->id) continue;

		if(IsKeyUp(action->key)) 			action->state = INPUT_UP; 
		if(IsKeyDown(action->key))			action->state = INPUT_DOWN;
		if(IsKeyReleased(action->key))		action->state = INPUT_RELEASED;
		if(IsKeyPressed(action->key))		action->state = INPUT_PRESSED;
	}
}

void input_Tick(InputHandler *input) {
	if(mouse_lock-- <= 0) mouse_lock = 0;
}

u8 input_GetActionState(u8 action_id) {
	if(action_map.find(action_id) == action_map.end())
		return 0;

	InputAction *action = &ptr_input_self->actions[action_map[action_id]];
	return action->state;
}

Vector2 input_MousePos() {
	return ptr_input_self->cursor_pos;
}

Vector2 input_MouseDelta() {
	return ptr_input_self->cursor_delta;
}

// Lock mouse movement for a number of frames
void input_LockMouse(int frames) {
	mouse_lock = frames;
}

// Turn off and lock optionally lock mouse movement for some frames
// * NOTE: 
// Sometimes causes erratic movement in first-person look...  
// Maybe forcing to center would help??
// To be tested.
void input_DisableMouse(int frames) {
	DisableCursor();
	GetMouseDelta();
	ptr_input_self->cursor_delta = (Vector2) { 0, 0 };
	GetMouseDelta();
	input_LockMouse(frames);
}

InputHandler *input_GetPointer() {
	return ptr_input_self;
}

