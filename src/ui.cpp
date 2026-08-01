#include <cstring>
#include "raylib.h"
#include "ui.h"

// Default value definitions  
#define TEXT_SIZE_DEF 		64.0f 
#define TEXT_SPACING_DEF	1.0f
#define LINE_THICK_DEF		2.0f;

UiImplementation ui = (UiImplementation) {0};

void ui_Init(Font font) {
	ui.state = TITLE;
	ui.font = font;

	// Set default values for text rendering
	ui.text_size = TEXT_SIZE_DEF;
	ui.text_spacing = TEXT_SPACING_DEF;
	ui.line_thick = LINE_THICK_DEF;

	// Set/copy colors
	Color colors[3] = { RAYWHITE, GRAY, DARKGRAY };
	memcpy(ui.colors, colors, sizeof(colors));
}

// Free memory allocated for UI
void ui_Close()	{
	UnloadFont(ui.font);
}

// Find where center of text should be
Vector2 ui_TextCenter(Rectangle rect, const char *text, float size, float spacing) {
	Vector2 rec_mid = { rect.x + rect.width * 0.5f, rect.y + rect.height * 0.5f };
	Vector2 text_bounds = MeasureTextEx(ui.font, text, size, spacing);

	return (Vector2) { rec_mid.x - text_bounds.x * 0.5f, rec_mid.y - text_bounds.y * 0.5f };
}

void ui_DrawText(Rectangle rect, const char *text, Color color) {
	Vector2 center = ui_TextCenter(rect, text, ui.text_size, ui.text_spacing);
	DrawTextEx(ui.font, text, center, ui.text_size, ui.text_spacing, color);
}

// Display a button, returns true if clicked
bool ui_Button(Rectangle rect, const char *text) {
	// Button state begins at 0 (default) on each frame
	u8 state = WG_DEFAULT;

	// Get state
	bool hover = CheckCollisionPointRec(GetMousePosition(), rect); 
	if(hover) { 
		// Increment state on hover, WG_DEFAULT -> WG_FOCUSED
		++state;			

		// Increment state on click, WG_FOCUSED -> WG_PRESSED
		if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
			++state;		
	}

	// Infer color from button state
	Color text_color = ui.colors[state];
	
	// Render element
	DrawRectangleLinesEx(rect, ui.line_thick, text_color);
	ui_DrawText(rect, text, text_color);

	// Return true if pressed
	return (state >= WG_PRESSED);
}

// Clickable box that toggles some boolean value
void ui_CheckBox(Rectangle rect, const char *text, bool *val) {
	// Infer color from boolean value
	Color box_color = (*val == true) ? ui.colors[WG_DEFAULT] : ui.colors[WG_PRESSED];	

	// Create rectangle for checkbox
	Rectangle box = (Rectangle) {
		.x = rect.x + rect.width,		// Set to the right of text 
		.y = rect.y,				  
		.width = rect.height,			// Width and height are uniform 
		.height = rect.height		
	};

	// Render elements
	DrawRectangleLinesEx(box, ui.line_thick, box_color);
	ui_DrawText(rect, text, ui.colors[WG_DEFAULT]);

	// Toggle check value on click
	if(CheckCollisionPointRec(GetMousePosition(), box) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
		*val = !(*val);
}

// Non-interactive text element
void ui_Label(Rectangle rect, const char *text) {
	// Background
	DrawRectangleRec(rect, ColorAlpha(BLACK, 0.5f));
	// Outline
	DrawRectangleLinesEx(rect, ui.line_thick, ui.colors[WG_DEFAULT]);
	// Text
	ui_DrawText(rect, text, ui.colors[WG_DEFAULT]);	
}

