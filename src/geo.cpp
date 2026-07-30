#include <string.h>
#include "geo.h"

// Signed plane distance 
float PlaneDistance(Vector3 point, Plane plane) {
	return Vector3DotProduct(plane.normal, point) - plane.distance;
}

u8 IntersectSegmentPlane(Vector3 a, Vector3 b, Plane plane, float *t, Vector3 *point) {
	Vector3 ab = Vector3Subtract(b, a);
	*t = (plane.distance - (Vector3DotProduct(plane.normal, a))) / (Vector3DotProduct(plane.normal, ab));

	if(*t >= 0.0f && *t <= 1.0f) {
		Vector3 vt; 
		memset(&vt, *t, sizeof(Vector3));
		*point = Vector3Multiply(Vector3Add(vt, a), ab);
		return 1;
	}

	return 0;
}

// Return a set of six planes from a bounding box
Plane *BoxToPlanes(BoundingBox aabb) {
	Plane *planes = (Plane*)malloc(sizeof(Plane) * 6);

	BoxNormals normals = BoxGetNormals();

	float distances[6];
	memcpy(distances, &aabb, sizeof(BoundingBox));
	
	for(u8 i = 0; i < 6; i++) {
		if(i >= 3) distances[i] *= -1;
		planes[i] = (Plane) { .normal = normals.n[i], .distance = distances[i] };
	}

	return planes;
} 

// Vector representing the length of each axis of given bounding box
Vector3 BoxExtents(BoundingBox aabb) {
	return Vector3Subtract(aabb.max, aabb.min);
}

// Center of given box
Vector3 BoxCenter(BoundingBox aabb) {
	return Vector3Subtract(aabb.max, Vector3Scale(BoxExtents(aabb), 0.5f));
}

// "Empty" bounding box, impossible to intersect with
BoundingBox BoxEmpty() {
	return (BoundingBox) { .min = Vector3Scale(Vector3One(), FLT_MAX), .max = Vector3Scale(Vector3One(), -FLT_MAX) };
}

// Move a box by setting it's origion to a given point
BoundingBox BoxRecenter(BoundingBox aabb, Vector3 point) {
	Vector3 ext = BoxExtents(aabb);
	return (BoundingBox) { .min = Vector3Add(point, Vector3Scale(ext, -0.5f)), .max = Vector3Add(point, Vector3Scale(ext, 0.5f)) };
}

// Box translated/moved by vector 
BoundingBox BoxTranslate(BoundingBox aabb, Vector3 v) {
	Vector3 point = Vector3Add(BoxCenter(aabb), v);	
	return BoxRecenter(aabb, point);	
}

// Expand bounding box to fit a point in space
BoundingBox BoxExpandToPoint(BoundingBox aabb, Vector3 point) {
	return (BoundingBox) { .min = Vector3Min(aabb.min, point), .max = Vector3Max(aabb.max, point) };
}

float BoxSurfaceArea(BoundingBox aabb) {
	Vector3 extent = BoxExtents(aabb); 
	return (extent.x * extent.y + extent.y * extent.z + extent.z * extent.x);
}

// Get vertices of given box
BoxPoints BoxGetPoints(BoundingBox aabb) {
	BoxPoints points = (BoxPoints) {0};

	for(u8 i = 0; i < 8; i++) {
		points.v[i].x = (i & 0x01) ? aabb.min.x : aabb.max.x;
		points.v[i].y = (i & 0x02) ? aabb.min.y : aabb.max.y;
		points.v[i].z = (i & 0x04) ? aabb.min.z : aabb.max.z;
	}

	return points;
}

// Get face normals for (any) AABB
BoxNormals BoxGetNormals() {
	BoxNormals normals = (BoxNormals) {0};

	for(u8 i = 0; i < 3; i++) {
		normals.n[i].x = (1 << i & 0x01) ? -1 : 0;
		normals.n[i].y = (1 << i & 0x02) ? -1 : 0;
		normals.n[i].z = (1 << i & 0x04) ? -1 : 0;
		normals.n[i+3] = Vector3Negate(normals.n[i]);
	}

	return normals;
}

