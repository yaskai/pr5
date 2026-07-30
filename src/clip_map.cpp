#include <cstdlib>
#include <string.h>
#include <stdlib.h>
#include "message.h"
#include "raylib.h"
#include "raymath.h"
#include "clip_map.h"
#include "bsp.h"
#include "geo.h"

#define CLIP_TREE_START_CAPACITY 1024
Clip_Tree clip_tree_Construct(Bsp_Data *bsp, int32_t submodel) {
	Clip_Tree tree = (Clip_Tree) {0};

	tree.brush_list = &bsp->brush_lists[submodel];

	tree.node_cap = CLIP_TREE_START_CAPACITY;
	tree.clip_nodes = (Clip_Node*)calloc(tree.node_cap, sizeof(Clip_Node));
	tree.brush_ids = (i32*)calloc(tree.brush_list->num_brushes, sizeof(i32));

	// Initialize root node
	Clip_Node root = (Clip_Node) {
		.bounds = BoxEmpty(),
		.children = { 0, 0 },
		.first_brush = 0,
		.num_brushes = (u16)tree.brush_list->num_brushes
	};

	for(i32 i = 0; i < tree.brush_list->num_brushes; i++) {
		Brush *brush = &tree.brush_list->brushes[i];
		root.bounds.min = Vector3Min(root.bounds.min, brush->aabb.min); 
		root.bounds.max = Vector3Max(root.bounds.max, brush->aabb.max); 

		tree.brush_ids[i] = i;
	}

	// Insert root
	tree.clip_nodes[tree.node_count++] = root;

	// Start recursive subdivision at node 0 (root)
	clip_tree_Subdivide(&tree, 0);
	
	return tree;
}

void clip_tree_Destroy(Clip_Tree *tree) {
	if(tree->brush_ids)		free(tree->brush_ids);
	if(tree->clip_nodes)	free(tree->clip_nodes);
	*tree = (Clip_Tree) {0};
}

void clip_tree_Subdivide(Clip_Tree *tree, u16 node_id) {
	msg_Print("clip_tree_Subdivide()", ANSI_BLUE);

	if(node_id < 0 || node_id >= tree->node_count) return;

	Clip_Node *node = &tree->clip_nodes[node_id];

	// Base case:
	// only one brush is left inside node
	if(node->num_brushes == 1) {
		return;
	}

	printf("num_brushes: %d\n", node->num_brushes);

	Vector3 e = BoxExtents(node->bounds);
	printf("extents: { %f, %f, %f }\n", e.x, e.y, e.z);

	// Find splitting axis
	float3 center = Vector3ToFloatV(BoxCenter(node->bounds));
	float ext_max = 0.0f;
	float3 extents = Vector3ToFloatV(BoxExtents(node->bounds));
	float3 maxs;
	for(u8 i = 0; i < 3; i++) {
		// Skip if smaller than largest extent on axis 
		if(extents.v[i] < ext_max) continue; 
		// If longest axis:
		// Update longest axis value
		ext_max = extents.v[i];
		// Also clamp left box to center on component axis
		maxs = Vector3ToFloatV(node->bounds.max);
		maxs.v[i] = center.v[i]; 
	}

	BoundingBox l_box = (BoundingBox) {
		.min = node->bounds.min,
		.max = *(Vector3*)maxs.v
	};

	BoundingBox r_box = (BoundingBox) {
		.min = l_box.max,
		.max = node->bounds.max
	};

	// Partition
	u16 i = node->first_brush;
	u16 j = i + node->num_brushes - 1;
	while(i <= j) {
		i32 brush_idx = tree->brush_ids[i];
		Brush *brush = &tree->brush_list->brushes[brush_idx];
		Vector3 brush_center = BoxCenter(brush->aabb);

		if(CheckCollisionBoxes(l_box, brush->aabb)) {
			i++;
		} else { 
			i32 t = tree->brush_ids[i];
			tree->brush_ids[i] = tree->brush_ids[j];
			tree->brush_ids[j] = t;
			j--;
		}
	}

	u16 l_count = i - node->first_brush;
	u16 r_count = node->num_brushes - l_count;

	// Base case:
	// Either side is empty
	if(l_count <= 0 || l_count == node->num_brushes) return;

	printf("l_count: %d\n", l_count);
	printf("r_count: %d\n", r_count);

	if(tree->node_count + 2 >= tree->node_cap) {
		tree->node_cap *= 2;
		tree->clip_nodes = (Clip_Node*)realloc(tree->clip_nodes, sizeof(Clip_Node) * tree->node_cap);
		node = &tree->clip_nodes[node_id];
	}

	u16 l_child = tree->node_count++;
	u16 r_child = tree->node_count++;

	// Initialize left child node
	tree->clip_nodes[l_child] = (Clip_Node) {
		//.bounds = BoxEmpty(),
		.bounds = l_box,
		.children = { 0, 0 },
		.first_brush = node->first_brush,
		.num_brushes = l_count
	};

	// Initialize right child node
	tree->clip_nodes[r_child] = (Clip_Node) {
		//.bounds = BoxEmpty(),
		.bounds = r_box,
		.children = { 0, 0 },
		.first_brush = i, 
		.num_brushes = r_count
	};

	// Set child indices and leave this node empty of brushes marking it as a branch
	node->children[0] = l_child;
	node->children[1] = r_child;

	// Construct bounding boxes for left and right child nodes
	/*
	for(u16 i = 0; i < 2; i++) {
		Clip_Node *child_node = &tree->clip_nodes[node->children[i]];

		for(u16 j = 0; j < child_node->num_brushes; j++) {
			i32 brush_idx = tree->brush_ids[child_node->first_brush + j];
			Brush *brush = &tree->brush_list->brushes[brush_idx];

			child_node->bounds.min = Vector3Min(child_node->bounds.min, brush->aabb.min);
			child_node->bounds.max = Vector3Max(child_node->bounds.max, brush->aabb.max);
		}
	}
	*/

	node->first_brush = 0;
	node->num_brushes = 0;

	// Recurse:
	clip_tree_Subdivide(tree, l_child);
	clip_tree_Subdivide(tree, r_child);
}

float clip_node_Cost(Clip_Node *node) {
	return BoxSurfaceArea(node->bounds) * node->num_brushes;
}

