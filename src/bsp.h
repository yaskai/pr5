#pragma once

#include <stdio.h>
#include <unordered_map>
#include <vector>
#include "raylib.h"
#include "common/nums.h"

#define BSP_USE_VERSION	 29 
#define BSP_NUM_LUMPS	 15 
enum LUMP_TYPES {
	LUMP_ENTITIES		= 0,
	LUMP_PLANES 		= 1,
	LUMP_MIPTEX			= 2,
	LUMP_VERTICES		= 3,
	LUMP_VIS			= 4,
	LUMP_NODES			= 5,
	LUMP_TEXINFO		= 6,
	LUMP_FACES			= 7,
	LUMP_LIGHTMAPS		= 8,
	LUMP_CLIPNODES 		= 9,
	LUMP_LEAVES			= 10,
	LUMP_LIST_FACES		= 11,
	LUMP_EDGES			= 12,
	LUMP_LIST_EDGES		= 13,
	LUMP_MODELS			= 14,
	LUMP_BSPX			= 15	// Extra lump from ericw-tools
};

typedef struct {
	char key[64];
	char val[64];

} Bsp_EntProperty;

typedef struct {
	Bsp_EntProperty props[64];	
	i32 num_props;

} Bsp_Entity;

// AABB bounding box
typedef struct {
	i16 min[3]; 
	i16 max[3];

} Bsp_AABB;

// Edge 
typedef struct {
	u16 v[2];

} Bsp_Edge;

// Plane
typedef struct {
	float normal[3];
	float dist;
	i32 type;

} Bsp_Plane;

// Bsp Lump
typedef struct {
	i32 f_offset;
	i32 f_length;

} Bsp_Lump;

// BSP File Header
typedef struct {
	i32 version; 
	Bsp_Lump lumps[BSP_NUM_LUMPS];

} Bsp_Header;

// Mip header
typedef struct {
	i32 *offset;
	i32 num_tex;

} Bsp_MipHeader;

// Miptex
typedef struct {
	char name[16];		// Texture name

	u32 width;
	u32 height;

	u32 offset1;
	u32 offset2;
	u32 offset4;
	u32 offset8;

} Bsp_Miptex;

// Texinfo
typedef struct {
	Vector3 vec_s;
	float dist_s;

	Vector3 vec_t;
	float dist_t;

	u32 miptex_id;
	u32 animated;

} Bsp_Texinfo;

// Clip Node
typedef struct {
	u32 plane;
	i16 children[2];

} Bsp_ClipNode;

// BSP Node
typedef struct {
	i32 plane;
	i16 children[2];

	Bsp_AABB aabb;

	u16 first_face;
	u16 num_faces;

} Bsp_Node;

// Leaf
typedef struct {
	i32 type;
	i32 vis_ofs;

	Bsp_AABB aabb;

	u16 first_face;
	u16 num_faces;

	u8 ambient[4];

} Bsp_Leaf;

// Face
typedef struct {
	u16 plane;
	u16 side;

	i32 first_edge;
	u16 num_edges;

	u16 texinfo;

	u8 type_light;
	u8 base_light;
	u8 light[2];
	i32 lightmap;

} Bsp_Face;

// List Faces
typedef struct {
	u16 *faces;
	u32 num_list_faces;

} Bsp_ListFaces;

// List Edges
typedef struct {
	i32 *edges;
	u32 num_list_edges;

} Bsp_ListEdges;

// BSP Model
typedef struct {
	float mins[3];
	float maxs[3];

	float origin[3];

	i32 head_nodes[4];
	i32 num_leaves;

	i32 first_face;
	i32 num_faces;

} Bsp_Model;

// BSP Hull
#define BSP_NUM_HULLS	4
typedef struct {
	Bsp_Plane *planes;
	Bsp_ClipNode *clipnodes;

	i32 first_clipnode;	
	i32 last_clipnode;
	i32 num_planes;

} Bsp_Hull;

typedef struct {
	Bsp_Hull hulls[BSP_NUM_HULLS];	
	Vector3 origin;
	u16 ent_id;
	
} Bsp_HullGroup;

typedef struct {
	char name[24];
	i32 offset;
	i32 length;

} Bspx_Lump;

typedef struct {
	char id[4];
	i32 num_lumps;
	Bspx_Lump lumps[16];

} Bspx_Header;

// Ericw-Tools BSPX decoupled lightmap
typedef struct {
	u16 w, h;
	u32 lm_offset;

	Vector3 vec_s;
	float dist_s;

	Vector3 vec_t;
	float dist_t;

} Lm_DecoupledEntry;

#define CONTENTS_EMPTY -1
#define CONTENTS_SOLID -2
#define CONTENTS_WATER -3
#define CONTENTS_SLIME -4
#define CONTENTS_LAVA  -5
#define CONTENTS_SKY   -6	

typedef struct {
	BoundingBox aabb;
	i16 contents;

	u16 num_planes;
	Bsp_Plane *planes;

} Brush;

typedef struct {
	i32 version;
	i32 model_id;
	i32 num_brushes;

	Brush *brushes;

	i32 num_planes;

} BrushList;

typedef struct {
	Lm_DecoupledEntry *entries;
	i32 *offsets;
	u8 *rgb;

} Bspx_Lightmap;

typedef struct {
	//i32 *brushes;	
	//i32 num_brushes;
	std::vector<i32> brushes;

} BrushRef; 

typedef struct {
	// header 
	Bsp_Header header;
	// **

	// entities
	Bsp_Entity *entities;
	u32 num_entities;
	// **

	// planes
	Bsp_Plane *planes;	
	u32 num_planes;

	// miptexs
	Bsp_Miptex *miptexs;
	u32 num_miptexs;
	// **
	
	// vertices
	Vector3 *vertices;
	u32 num_vertices;
	// **

	// pvs
	u8 *vis;
	u32 num_vis;
	// **
	
	// bsp nodes
	Bsp_Node *nodes;
	u32 num_nodes;
	// **
	
	// texinfo
	Bsp_Texinfo *texinfos;
	u32 num_texinfos;
	// **

	// faces
	Bsp_Face *faces;
	u32 num_faces;
	// **

	// lightmaps
	// **

	// clipnodes
	Bsp_ClipNode *clipnodes;
	u32 num_clipnodes;
	//**

	// leaves
	Bsp_Leaf *leaves;
	u32 num_leaves;
	// **
	
	// list faces
	Bsp_ListFaces list_faces;
	// **
	
	// edges
	Bsp_Edge *edges;
	u32 num_edges;
	// **

	// list edges
	Bsp_ListEdges list_edges;
	// **

	// bsp models
	Bsp_Model *models;
	u32 num_models;
	// **

	// hulls
	Bsp_Hull *hulls;
	//**

	// BSPX header
	Bspx_Header bspx_header;

	// BSPX data
	Bspx_Lightmap bspx_lm;
	BrushList *brush_lists;

	BrushRef *brush_refs;

} Bsp_Data;

Bsp_Data bsp_LoadFile(const char *path);
void bsp_Unload(Bsp_Data *bsp);

void bsp_ParseEntities(Bsp_Data *bsp, FILE *pF);
void bsp_ParsePlanes(Bsp_Data *bsp, FILE *pF);
void bsp_ParseMiptexs(Bsp_Data *bsp, FILE *pF);
void bsp_ParseVertices(Bsp_Data *bsp, FILE *pF);
void bsp_ParseVis(Bsp_Data *bsp, FILE *pF);
void bsp_ParseNodes(Bsp_Data *bsp, FILE *pF);
void bsp_ParseTexInfos(Bsp_Data *bsp, FILE *pF);
void bsp_ParseFaces(Bsp_Data *bsp, FILE *pF);
void bsp_ParseLightmaps(Bsp_Data *bsp, FILE *pF);
void bsp_ParseClipNodes(Bsp_Data *bsp, FILE *pF);
void bsp_ParseLeaves(Bsp_Data *bsp, FILE *pF);
void bsp_ParseListFaces(Bsp_Data *bsp, FILE *pF);
void bsp_ParseEdges(Bsp_Data *bsp, FILE *pF);
void bsp_ParseListEdges(Bsp_Data *bsp, FILE *pF);
void bsp_ParseModels(Bsp_Data *bsp, FILE *pF);

void bsp_LoadExtra(Bsp_Data *bsp, FILE *pF);
void bsp_UnloadExtra(Bsp_Data *bsp);

void bspx_ParseBrushList(Bsp_Data *bsp, FILE *pF, Bspx_Lump *lump);
void bspx_ParseLightRGB(Bsp_Data *bsp, FILE *pF, Bspx_Lump *lump);
void bspx_ParseDecoupledLM(Bsp_Data *bsp, FILE *pF, Bspx_Lump *lump);

// Convert Bsp_ABBB to Raylib bounding box
BoundingBox B32_ToBox(Bsp_AABB b32);

// Create a bsp hull
Bsp_Hull bsp_BuildHull(Bsp_Data *bsp, i32 hull_id);

void bspx_MakeBrushRefs(Bsp_Data *bsp, u32 submodel);

