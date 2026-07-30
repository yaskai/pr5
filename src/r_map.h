#pragma once

#include <stdlib.h>
#include <vector>
#include "raylib.h"
#include "common/nums.h"
#include "bsp.h"
#include "lit.h"

#define RBRUSH_FLAG_SKIP 			0x01
#define RBRUSH_FLAG_TRANSLUCENT		0x02

typedef struct {
	Model model;
	
	BoundingBox aabb;

	i32 leaf;
	
	u8 vis;
	u8 flags;
	
} r_Brush;

class r_Map {
public:
	void Build(Bsp_Data *bsp);
	void Unload();

	void Draw(Bsp_Data *bsp, Vector3 camera_position);

private:
	r_Brush *rbrushes;
	u32 rbrush_count, rbrush_cap;

	std::vector<i32> rbrush_ids;

	Lightmap lm;

	void Push_rBrush(r_Brush rb) {
		if(rbrush_count + 1 >= rbrush_cap) {
			rbrush_cap = (rbrush_cap << 1);
			rbrushes = (r_Brush*)realloc(rbrushes, sizeof(r_Brush) * rbrush_cap);
		}

		rbrushes[rbrush_count++] = rb;
	}
};

Model *r_BspLeafToModels(Bsp_Data *bsp, Lightmap *lm, i32 leaf_id, int *n_out, u8 *flags);
r_Brush *r_BspLeafToRenderBrushes(Bsp_Data *bsp, Lightmap *lm, i32 leaf_id, int *n_out);

