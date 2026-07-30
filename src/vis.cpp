#include "vis.h"
#include "geo.h"

i32 vis_FindLeaf(Bsp_Data *bsp, Vector3 point) {
	int node_num = bsp->models[0].head_nodes[0];

	while(node_num >= 0) {
		Bsp_Node *node = &bsp->nodes[node_num];
		Plane plane = *(Plane*)&bsp->planes[node->plane];

		node_num = (PlaneDistance(point, plane) >= 0) ? node->children[0] : node->children[1];
	}

	return ~node_num;
}

u8 vis_IsLeafVisible(Bsp_Data *bsp, int32_t curr_leaf, int32_t test_leaf) {
	if(curr_leaf == test_leaf)
		return true;

	Bsp_Leaf *leaf = &bsp->leaves[curr_leaf];

	// No vis data, default to drawing
	if(leaf->vis_ofs < 0) return 1;

	u8 *vis = bsp->vis + leaf->vis_ofs;

	// Decompress and test bitmask
	i32 leafnum = 1;
	while(leafnum < (i32)bsp->num_leaves) {
		if(*vis == 0) {
			// Skip
			vis++;
			leafnum += *vis * 8;
			vis++;

		} else {
			// Test each bit in byte
			for(int bit = 0; bit < 8; bit++) {
				if(leafnum == test_leaf) {
					return (*vis >> bit) & 1;
				}

				leafnum++;
			}

			vis++;
		}
	}

	return 0;
}

