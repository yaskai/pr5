#include "raylib.h"
#include "window.h"
#include "config.h"

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

void window_Open() {
	InitWindow(conf_GetOptionValue("window:width"), conf_GetOptionValue("window:height"), "PR5");	
	SetExitKey(KEY_NULL);

	if(conf_GetOptionValue("graphics:vsync")) 		window_flags |= FLAG_VSYNC_HINT;
	if(conf_GetOptionValue("window:fullscreen"))	window_flags |= FLAG_FULLSCREEN_MODE;
	SetWindowState(window_flags);
}

void window_ToggleFlag(u32 flag) {
	window_flags ^= flag;

	if(window_flags & flag) SetWindowState(flag);
	else ClearWindowState(flag);
}

