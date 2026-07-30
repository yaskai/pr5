#pragma once

#include "raylib.h"

static TextureCubemap GenTextureCubemap(Shader shader, Texture2D panorama, int size, int format);

typedef struct {
	Model model;
	Shader shader;

	bool useHDR;
	
} Skybox;

Skybox skybox_Init(const char *img_path, int size, int format);
void skybox_Close(Skybox *skybox);

void skybox_Render(Skybox *skybox);

