#pragma once

#include "raylib.h"
#include "raymath.h"
#include "common/nums.h"
#include "geo.h"
#include "bsp.h"

typedef struct {
	BoundingBox bounds;

	u16 children[2];

	u16 first_brush;
	u16 num_brushes;
	
} Clip_Node;

typedef struct {
	BrushList *brush_list;
	Clip_Node *clip_nodes;
	i32 *brush_ids;

	u16 node_count;
	u16 node_cap;
	
} Clip_Tree; 

Clip_Tree clip_tree_Construct(Bsp_Data *bsp, i32 submodel);
void clip_tree_Destroy(Clip_Tree *tree);
void clip_tree_Subdivide(Clip_Tree *tree, u16 node_id);

float clip_node_Cost(Clip_Node *node);

