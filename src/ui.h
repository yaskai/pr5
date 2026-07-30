#pragma once

#include "raylib.h"
#include "common/nums.h"

enum UI_STATES : u8 {
	TITLE,
	MAIN
};

enum WIDGET_STATES : u8 {
	WG_DEFAULT,
	WG_FOCUSED,
	WG_PRESSED
};

typedef struct {
	Font font;

} UiStyle;

typedef struct { 
	Font font;

	Color colors[3];

	float text_size, text_spacing;
	float line_thick;

	u8 state;

} UiImplementation;

void ui_Init(Font font);
void ui_Close();

Vector2 ui_TextCenter(Rectangle rect, const char *text, float size, float spacing);

void ui_DrawText(Rectangle rect, const char *text, Color color);

bool ui_Button(Rectangle rect, const char *text);

void ui_CheckBox(Rectangle rect, const char *text, bool *val);

void ui_Label(Rectangle rect, const char *text);

