#pragma once

#include "raylib.h"
#include "common/nums.h"
#include "bsp.h"

typedef struct {
	float min_u, max_u;
	float min_v, max_v;	
	float w, h;

} Lm_FaceInfo;

typedef struct {
	Rectangle *uvs;
	u32 num_uvs;
	
	Texture2D tex;

} Lightmap;

Lightmap lm_Construct(Bsp_Data *bsp);

