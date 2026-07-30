#include <string.h>
#include "geo.h"
#include "raylib.h"
#include "raymath.h"
#include "bsp_trace.h"

i32 bsp_PointContents(Bsp_Hull *hull, int num, Vector3 point) {
	float d;
	Bsp_ClipNode *node;
	Bsp_Plane *plane;

	while(num >= 0) {
		// Bad node number, shouldn't happen
		if(num < hull->first_clipnode || num > hull->last_clipnode)
			return 0;

		node = &hull->clipnodes[num];
		plane = &hull->planes[node->plane];
		d = PlaneDistance(point, *(Plane*)plane);

		if(d < 0)
			num = node->children[1];
		else 
			num = node->children[0];
	}

	return num;
}

#define	DIST_EPSILON	(0.03125)
Bsp_TraceData Bsp_TraceDataEmpty() {
	Bsp_TraceData data = {0};
	data.all_solid = true;
	data.fraction = 1;
	return data;
}

bool bsp_RecursiveTraceEx(Bsp_Hull *hull, i32 node_num, float p1_frac, float p2_frac, Vector3 p1, Vector3 p2, Bsp_TraceData *trace) {
	Bsp_ClipNode *node;
	Bsp_Plane *plane;
	float t1, t2;
	Vector3 mid;
	int side;
	float mid_frac;
	float frac;

	// Check for empty
	if(node_num < 0) {
		if(node_num != CONTENTS_SOLID) {
			trace->all_solid = false;

			if(node_num == CONTENTS_EMPTY)
				trace->in_open = true;
			else 	
				trace->in_water = true;
		} else 
			trace->start_solid = true;

		return true;	// Empty
	}

	if(node_num < hull->first_clipnode || node_num > hull->last_clipnode) {
		// Bad node number	
		return true;
	}

	node = &hull->clipnodes[node_num];
	plane = &hull->planes[node->plane];

	Vector3 norm = *(Vector3 *) plane->normal;
	if(plane->type < 3) {
		float3 p1_f3 = Vector3ToFloatV(p1);
		float3 p2_f3 = Vector3ToFloatV(p2);

		t1 =  p1_f3.v[plane->type] - plane->dist;
		t2 =  p2_f3.v[plane->type] - plane->dist;

		p1 = *(Vector3 *) p1_f3.v;
		p2 = *(Vector3 *) p2_f3.v;

	} else {

		t1 = Vector3DotProduct(norm, p1) - plane->dist;
		t2 = Vector3DotProduct(norm, p2) - plane->dist;
	}

	if(t1 >= 0 && t2 >= 0)
		return bsp_RecursiveTraceEx(hull, node->children[0], p1_frac, p2_frac, p1, p2, trace);
	if(t1 < 0 && t2 < 0)
		return bsp_RecursiveTraceEx(hull, node->children[1], p1_frac, p2_frac, p1, p2, trace);

	if(t1 < 0)
		frac = (t1 + DIST_EPSILON) / (t1 - t2);
	else 
		frac = (t1 - DIST_EPSILON) / (t1 - t2);

	frac = Clamp(frac, 0, 1);

	mid_frac = p1_frac + (p2_frac - p1_frac) * frac; 

	float3 m = {0};
	float3 p1_f3 = Vector3ToFloatV(p1);
	float3 p2_f3 = Vector3ToFloatV(p2);
	for(short i = 0; i < 3; i++) 
		m.v[i] = p1_f3.v[i] + frac*(p2_f3.v[i] - p1_f3.v[i]);

	mid = *(Vector3 *) m.v;
	side = (t1 < 0);

	// Move up to node
	if(!bsp_RecursiveTraceEx(hull, node->children[side], p1_frac, mid_frac, p1, mid, trace))
		return false;

	// Go past node
	if(bsp_PointContents(hull, node->children[side^1], mid) != CONTENTS_SOLID)
		return bsp_RecursiveTraceEx(hull, node->children[side^1], mid_frac, p2_frac, mid, p2, trace);

	// Never go out of solid area
	if(trace->all_solid)
		return false;

	if(!side) {
		memcpy(trace->plane.normal, plane->normal, sizeof(float) * 3);
		trace->plane.dist = plane->dist;

	} else {
		for(short i = 0; i < 3; i++)
			trace->plane.normal[i] = -plane->normal[i];
		
		trace->plane.dist = -plane->dist;
	}

	// Shouldn't happen but does sometimes
	while(bsp_PointContents(hull, node_num, mid) == CONTENTS_SOLID) {
		frac -= 0.1f;

		if(frac < 0) {
			trace->fraction = mid_frac;
			trace->point = mid;
			return false;
		}

		mid_frac = p1_frac + (p2_frac - p1_frac) * frac;

		float3 m = {0};
		float3 p1_f3 = Vector3ToFloatV(p1);
		float3 p2_f3 = Vector3ToFloatV(p2);
		for(u8 i = 0; i < 3; i++) m.v[i] = p1_f3.v[i] + frac*(p2_f3.v[i] - p1_f3.v[i]);

		mid = *(Vector3 *) m.v;
	}

	trace->fraction = mid_frac;
	trace->point = mid;

	return false;
}

