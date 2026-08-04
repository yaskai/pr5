#pragma once

#include "common/nums.h"
#include "bsp.h"

i32 vis_FindLeaf(Bsp_Data *bsp, Vector3 point); 
u8 vis_IsLeafVisible(Bsp_Data *bsp, i32 curr_leaf, i32 test_leaf);
