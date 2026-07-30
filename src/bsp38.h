#pragma once

#include "raylib.h"
#include "nums.h"

#define BSP_VERSION	38
#define NUM_LUMPS 	18

enum LUMP_TYPES {
	LUMP_ENTITIES		=  0,
	LUMP_PLANES 		=  1,
	LUMP_VERTICES		=  2,
	LUMP_VIS			=  3,
	LUMP_NODES			=  4,
	LUMP_TEXINFO		=  5,
	LUMP_FACES			=  6,
	LUMP_LIGTHING		=  7,
	LUMP_LEAVES			=  8,
	LUMP_LEAF_FACES		=  9,
	LUMP_LEAF_BRUSHES	= 10,
	LUMP_EDGES			= 11,
	LUMP_SURF_EDGES		= 12,
	LUMP_MODELS			= 13,
	LUMP_BRUSHES		= 14,
	LUMP_BRUSH_SIDES	= 15,
	LUMP_POP			= 16,
	LUMP_AREAS			= 17,
	LUMP_AREA_PORTALS	= 18,
	LUMP_BSPX			= 19
};

typedef struct {
	u32 offset;
	u32 length;

} Bsp_Lump;

typedef struct {
	u32 magic;
	u32 version;

	Bsp_Lump lumps[19];

} Bsp_Header;

typedef struct {
	i16 x;
	i16 y;
	i16 z;

} point3s;

// Plane
typedef struct {
	Vector3 normal;
	float distance;
	u32 type;

} Bsp_Plane;

// Face
typedef struct {
	u16 plane;
	u16 side;

	u32 first_edge;
	u16 num_edges;

	u16 texinfo;

	u8 light_styles[4];
	u32 light_offset;

} Bsp_Face;

