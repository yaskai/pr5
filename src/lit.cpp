#include <cstdlib>
#include <raylib.h>
#include <stdlib.h>
#include "lit.h"
#include "bsp.h"
#include "message.h"

Lightmap lm_Construct(Bsp_Data *bsp) {
	msg_Print("lm_Construct()", ANSI_BLUE);

	Lightmap lm = (Lightmap) {0};

	lm.num_uvs = bsp->num_faces;
	lm.uvs = (Rectangle*)calloc(lm.num_uvs, sizeof(Rectangle));
	
	i32 atlas_w = 1024;
	i32 cX = 0, cY = 0;
	i32 row_h = 0;

	for(u32 i = 0; i < lm.num_uvs; i++) {
		Lm_DecoupledEntry *entry = &bsp->bspx_lm.entries[i];
		if(entry->w == 0 || entry->h == 0) continue;

		if(cX + entry->w > atlas_w) {
			cX = 0;
			cY += row_h;
			row_h = 0;
		}

		lm.uvs[i] = (Rectangle) { .x = (float)cX, .y = (float)cY, .width = (float)entry->w, .height = (float)entry->h };
		cX += entry->w;

		if(entry->h > row_h) row_h = entry->h;
	}

	i32 atlas_h = cY + row_h;
	i32 total_px = atlas_w * atlas_h * 4;
	u8 *px = (u8*)calloc(total_px, sizeof(u8));
	
	for(u32 i = 0; i < lm.num_uvs; i++) {
		Lm_DecoupledEntry *entry = &bsp->bspx_lm.entries[i];
		if(entry->w == 0 || entry->h == 0) continue;

		i32 w = entry->w;
		i32 h = entry->h;
		i32 ax = lm.uvs[i].x;
		i32 ay = lm.uvs[i].y;	

		u8 *src_px = bsp->bspx_lm.rgb + entry->lm_offset * 3;

		for(i32 y = 0; y < h; y++) {
			for(i32 x = 0; x < w; x++) {
				i32 src_id = (y * w + x) * 3;
				i32 dst_id = ((ay + y) * atlas_w + (ax + x)) * 4;

				px[dst_id+0] = src_px[src_id+0]; 
				px[dst_id+1] = src_px[src_id+1]; 
				px[dst_id+2] = src_px[src_id+2]; 
				px[dst_id+3] = 255; 
			}
		}
	}

	Image img = (Image) {
		.data = px,
		.width = atlas_w,
		.height = atlas_h,
		.mipmaps = 1,
		.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
	};

	lm.tex = LoadTextureFromImage(img);
	SetTextureFilter(lm.tex, TEXTURE_FILTER_TRILINEAR);
	UnloadImage(img);
	
	return lm;
}

