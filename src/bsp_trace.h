#pragma once

#include "common/nums.h"
#include "bsp.h"

i32 bsp_PointContents(Bsp_Hull *hull, int num, Vector3 point);

typedef struct {
	Bsp_Plane plane;

	Vector3 point;
	Vector3 normal;

	float distance;
	float fraction;

	bool start_solid;
	bool all_solid;
	bool in_open;
	bool in_water;

	bool hit;

} Bsp_TraceData;

// Returns empty Bsp_TraceData struct instance
Bsp_TraceData Bsp_TraceDataEmpty();

// Recursively trace segment through a bsp hull's clip nodes
bool bsp_RecursiveTraceEx(Bsp_Hull *hull, i32 node_num, float p1_frac, float p2_frac, Vector3 p1, Vector3 p2, Bsp_TraceData *trace);

