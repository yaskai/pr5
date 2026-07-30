#include <cstdlib>
#include <cstring>
#include <stdio.h>
#include <string>
#include <unordered_map>
#include "raylib.h"
#include "message.h"
#include "bsp.h"

typedef void (*ParseFunc)(Bsp_Data *bsp, FILE *pF);
static const ParseFunc parse_fn[BSP_NUM_LUMPS] = {
	&bsp_ParseEntities,					// Entities
	&bsp_ParsePlanes,					// Planes
	&bsp_ParseMiptexs,					// Mips
    &bsp_ParseVertices,					// Vertices
    &bsp_ParseVis,						// Vis
    &bsp_ParseNodes,					// BSP nodes
    &bsp_ParseTexInfos,					// Texinfos/surfaces
    &bsp_ParseFaces,					// Faces
    NULL, /*&bsp_ParseLightmaps,*/		// Lightmaps
    &bsp_ParseClipNodes,				// Clip nodes
    &bsp_ParseLeaves,					// Leaves
    &bsp_ParseListFaces,				// List faces
    &bsp_ParseEdges,					// Edges
    &bsp_ParseListEdges,				// List edges
    &bsp_ParseModels					// Models
};

Bsp_Data bsp_LoadFile(const char *path) {
	msg_Print("bsp_LoadFile()", ANSI_BLUE);

	Bsp_Data bsp = (Bsp_Data) {0};
	
	FILE *pF = fopen(path, "rb");
	if(!pF) {
		msg_LogError("could not open bsp file", path);
		return bsp;
	}
	msg_Print("file ok", ANSI_GREEN);

	Bsp_Header header = (Bsp_Header) {0};
	fread(&header, sizeof(Bsp_Header), 1, pF);
	
	if(header.version != BSP_USE_VERSION) {
		msg_LogError("bsp version mismatch", NULL);
		return bsp;
	}
	msg_Print("version ok", ANSI_GREEN);

	bsp.header = header;

	for(u8 i = 0; i < BSP_NUM_LUMPS; i++) 
		if(parse_fn[i]) parse_fn[i](&bsp, pF);

	bsp_LoadExtra(&bsp, pF);

	fclose(pF);

	bsp.hulls = (Bsp_Hull*)malloc(sizeof(Bsp_Hull) * BSP_NUM_HULLS);
	for(i32 i = 0; i < BSP_NUM_HULLS; i++) bsp.hulls[i] = bsp_BuildHull(&bsp, i);

	return bsp;
}

void bsp_Unload(Bsp_Data *bsp) {
	msg_Print("bsp_Unload()", ANSI_BLUE);
	
	if(bsp->entities)			free(bsp->entities);
	if(bsp->planes)				free(bsp->planes);
	if(bsp->miptexs)			free(bsp->miptexs);
	if(bsp->vertices)			free(bsp->vertices);
	if(bsp->vis)				free(bsp->vis);
	if(bsp->clipnodes)			free(bsp->clipnodes);
	if(bsp->leaves)				free(bsp->leaves);
	if(bsp->list_faces.faces)	free(bsp->list_faces.faces);
	if(bsp->list_edges.edges)	free(bsp->list_edges.edges);
	if(bsp->models)				free(bsp->models);	

	bsp_UnloadExtra(bsp);

	*bsp = (Bsp_Data) {0};
}

void bsp_ParseEntities(Bsp_Data *bsp, FILE *pF) {
	msg_Print("bsp_ParseEntities()", ANSI_BLUE);

	Bsp_Lump lump = bsp->header.lumps[LUMP_ENTITIES];

	fseek(pF, lump.f_offset, SEEK_SET);
	char *str = (char*)calloc(lump.f_length, 1);
	fread(str, lump.f_length, 1, pF);

	u32 capacity = 16;
	bsp->entities = (Bsp_Entity*)calloc(capacity, sizeof(Bsp_Entity));

	char *cursor = str;
	while(*cursor) {
		if(*cursor == '{') {
			Bsp_Entity ent = (Bsp_Entity) {0};

			while(*cursor && *cursor != '}') {
				if(*cursor != '"') {
					cursor++;
					continue;
				}

				Bsp_EntProperty *prop = &ent.props[ent.num_props++];
				
				cursor++;
				char *dest = prop->key;
				while(*cursor && *cursor != '"') *dest++ = *cursor++;
				cursor++;

				while(*cursor && *cursor != '"') *dest++ = *cursor++;

				cursor++;
				dest = prop->val;
				while(*cursor && *cursor != '"') *dest++ = *cursor++;
				cursor++;
			}
			cursor++;

			if(bsp->num_entities + 1 >= capacity) {
				capacity = capacity << 1;
				bsp->entities = (Bsp_Entity*)realloc(bsp->entities, sizeof(Bsp_Entity) * capacity);
			}

			bsp->entities[bsp->num_entities++] = ent;
		}

		cursor++;
	}

	bsp->entities = (Bsp_Entity*)realloc(bsp->entities, sizeof(Bsp_Entity) * bsp->num_entities);
	free(str);

	printf("num_entities: %d\n", bsp->num_entities);
}

void bsp_ParsePlanes(Bsp_Data *bsp, FILE *pF) {
	msg_Print("bsp_ParsePlanes()", ANSI_BLUE);

	Bsp_Lump lump = bsp->header.lumps[LUMP_PLANES];

	fseek(pF, lump.f_offset, SEEK_SET);
	bsp->num_planes = lump.f_length / sizeof(Bsp_Plane);
	bsp->planes = (Bsp_Plane*)malloc(sizeof(Bsp_Plane) * bsp->num_planes);
	fread(bsp->planes, sizeof(Bsp_Plane) * bsp->num_planes, 1, pF);
}

void bsp_ParseMiptexs(Bsp_Data *bsp, FILE *pF) {
	msg_Print("bsp_ParseMiptexs()", ANSI_BLUE);

	Bsp_Lump lump = bsp->header.lumps[LUMP_MIPTEX];	

	fseek(pF, lump.f_offset, SEEK_SET);
	
	i32 count = 0;
	fread(&count, sizeof(i32), 1, pF);

	i32 *offsets = (i32*)calloc(count, sizeof(i32));
	fread(offsets, sizeof(i32) * count, 1, pF);

	bsp->num_miptexs = count;
	bsp->miptexs = (Bsp_Miptex*)calloc(count, sizeof(Bsp_Miptex));

	for(i32 i = 0; i < count; i++) {
		fseek(pF, lump.f_offset + offsets[i], SEEK_SET);
		fread(&bsp->miptexs[i], sizeof(Bsp_Miptex), 1, pF);
	}

	free(offsets);	
}

void bsp_ParseVertices(Bsp_Data *bsp, FILE *pF) {
	msg_Print("bsp_ParseVertices()", ANSI_BLUE);

	Bsp_Lump lump = bsp->header.lumps[LUMP_VERTICES];

	fseek(pF, lump.f_offset, SEEK_SET);
	bsp->num_vertices = lump.f_length / sizeof(Vector3);
	bsp->vertices = (Vector3*)calloc(bsp->num_vertices, sizeof(Vector3));
	fread(bsp->vertices, sizeof(Vector3) * bsp->num_vertices, 1, pF);
}

void bsp_ParseVis(Bsp_Data *bsp, FILE *pF) {
	msg_Print("bsp_ParseVis()", ANSI_BLUE);

	Bsp_Lump lump = bsp->header.lumps[LUMP_VIS];
	
	fseek(pF, lump.f_offset, SEEK_SET);	
	bsp->num_vis = lump.f_length;
	bsp->vis = (u8*)calloc(bsp->num_vis, 1);
	fread(bsp->vis, bsp->num_vis, 1, pF);
}

void bsp_ParseNodes(Bsp_Data *bsp, FILE *pF) {
	msg_Print("bsp_ParseNodes()", ANSI_BLUE);

	Bsp_Lump lump = bsp->header.lumps[LUMP_NODES];

	fseek(pF, lump.f_offset, SEEK_SET);	
	bsp->num_nodes = lump.f_length / sizeof(Bsp_Node);
	bsp->nodes = (Bsp_Node*)calloc(bsp->num_nodes, sizeof(Bsp_Node));
	fread(bsp->nodes, sizeof(Bsp_Node) * bsp->num_nodes, 1, pF);
}

void bsp_ParseTexInfos(Bsp_Data *bsp, FILE *pF) {
	msg_Print("bsp_ParseTexInfos()", ANSI_BLUE);

	Bsp_Lump lump = bsp->header.lumps[LUMP_TEXINFO];

	fseek(pF, lump.f_offset, SEEK_SET);
	bsp->num_texinfos = lump.f_length / sizeof(Bsp_Texinfo);
	bsp->texinfos = (Bsp_Texinfo*)calloc(bsp->num_texinfos, sizeof(Bsp_Texinfo));
	fread(bsp->texinfos, sizeof(Bsp_Texinfo) * bsp->num_texinfos, 1, pF);
}

void bsp_ParseFaces(Bsp_Data *bsp, FILE *pF) {
	msg_Print("bsp_ParseFaces()", ANSI_BLUE);

	Bsp_Lump lump = bsp->header.lumps[LUMP_FACES];

	bsp->num_faces = lump.f_length / sizeof(Bsp_Face);	
	bsp->faces = (Bsp_Face*)calloc(bsp->num_faces, sizeof(Bsp_Face));
	fread(bsp->faces, sizeof(Bsp_Face) * bsp->num_faces, 1, pF);
}

// * NOTE:
// Will be using BSPX decoupled lightmaps from ericw-tools instead of the normal BSP29 
//void bsp_ParseLightmaps(Bsp_Data *bsp, FILE *pF) {}

void bsp_ParseClipNodes(Bsp_Data *bsp, FILE *pF) {
	msg_Print("bsp_ParseClipNodes()", ANSI_BLUE);

	Bsp_Lump lump = bsp->header.lumps[LUMP_CLIPNODES];

	fseek(pF, lump.f_offset, SEEK_SET);	
	bsp->num_clipnodes = lump.f_length / sizeof(Bsp_ClipNode);	
	bsp->clipnodes = (Bsp_ClipNode*)calloc(bsp->num_clipnodes, sizeof(Bsp_ClipNode));
	fread(bsp->clipnodes, sizeof(Bsp_ClipNode) * bsp->num_clipnodes, 1, pF);
}

void bsp_ParseLeaves(Bsp_Data *bsp, FILE *pF) {
	msg_Print("bsp_ParseLeaves", ANSI_BLUE);

	Bsp_Lump lump = bsp->header.lumps[LUMP_LEAVES];

	fseek(pF, lump.f_offset, SEEK_SET);
	bsp->num_leaves = lump.f_length / sizeof(Bsp_Leaf);
	bsp->leaves = (Bsp_Leaf*)calloc(bsp->num_leaves, sizeof(Bsp_Leaf));
	fread(bsp->leaves, sizeof(Bsp_Leaf) * bsp->num_leaves, 1, pF);
}

void bsp_ParseListFaces(Bsp_Data *bsp, FILE *pF) {
	msg_Print("bsp_ParseListFaces()", ANSI_BLUE);

	Bsp_Lump lump = bsp->header.lumps[LUMP_LIST_FACES];

	fseek(pF, lump.f_offset, SEEK_SET);
	bsp->list_faces.num_list_faces = lump.f_length / sizeof(u16);
	bsp->list_faces.faces = (u16*)calloc(bsp->list_faces.num_list_faces, sizeof(u16));
	fread(bsp->list_faces.faces, sizeof(u16) * bsp->list_faces.num_list_faces, 1, pF);
}

void bsp_ParseEdges(Bsp_Data *bsp, FILE *pF) {
	msg_Print("bsp_ParseEdges()", ANSI_BLUE);
	
	Bsp_Lump lump = bsp->header.lumps[LUMP_EDGES];

	fseek(pF, lump.f_offset, SEEK_SET);
	bsp->num_edges = lump.f_length / sizeof(Bsp_Edge); 
	bsp->edges = (Bsp_Edge*)calloc(bsp->num_edges, sizeof(Bsp_Edge));
	fread(bsp->edges, sizeof(Bsp_Edge) * bsp->num_edges, 1, pF);

	printf("num_edges: %d\n", bsp->num_edges);
}

void bsp_ParseListEdges(Bsp_Data *bsp, FILE *pF) {
	msg_Print("bsp_ParseListEdges()", ANSI_BLUE);

	Bsp_Lump lump = bsp->header.lumps[LUMP_LIST_EDGES];

	fseek(pF, lump.f_offset, SEEK_SET);
	bsp->list_edges.num_list_edges = lump.f_length / sizeof(i32);
	bsp->list_edges.edges = (i32*)calloc(bsp->list_edges.num_list_edges, sizeof(i32));
	fread(bsp->list_edges.edges, sizeof(i32) * bsp->list_edges.num_list_edges, 1, pF);

	printf("num_list_edges: %d\n", bsp->list_edges.num_list_edges);
}

void bsp_ParseModels(Bsp_Data *bsp, FILE *pF) {
	msg_Print("bsp_ParseModels()", ANSI_BLUE);

	Bsp_Lump lump = bsp->header.lumps[LUMP_MODELS];
	
	fseek(pF, lump.f_offset, SEEK_SET);
	bsp->num_models = lump.f_length / sizeof(Bsp_Model);
	bsp->models = (Bsp_Model*)calloc(bsp->num_models, sizeof(Bsp_Model));
	fread(bsp->models, sizeof(Bsp_Model) * bsp->num_models, 1, pF);

	printf("num_models: %d\n", bsp->num_models);
}

typedef void (*Bspx_ParseFunc)(Bsp_Data *bsp, FILE *pf);
static const Bspx_ParseFunc bspx_parse_fn[] = {
};

void bsp_LoadExtra(Bsp_Data *bsp, FILE *pF) {
	// Seek to end of vanilla BSP file
	i32 last_offset = 0;
	for(u8 i = 0; i < BSP_NUM_LUMPS; i++) {
		i32 end = bsp->header.lumps[i].f_offset + bsp->header.lumps[i].f_length;
		if(end > last_offset) last_offset = end;
	}
	last_offset = (last_offset + 3) & ~3; 
	fseek(pF, last_offset, SEEK_SET);

	// Clear initialize header 
	Bspx_Header *header = &bsp->bspx_header;
	*header = (Bspx_Header) {0};

	// Magic number/id check
	fread(header->id, 4, 1, pF);
	if(memcmp(header->id, "BSPX", 4)) {
		msg_LogWarning("No BSPX data", NULL);
		return;
	}

	// Get lump count
	fread(&header->num_lumps, sizeof(i32), 1, pF);

	// Read lump data and track which indices they occupy
	std::unordered_map<std::string, i8> lump_ids_map;
	for(i32 i = 0; i < header->num_lumps; i++) {
		Bspx_Lump *lump = &header->lumps[i];
		fread(lump, sizeof(Bspx_Lump), 1, pF);

		msg_Print(lump->name, ANSI_MAGENTA);
		
		lump_ids_map[lump->name] = i;
	}

	if(lump_ids_map.find("RGBLIGHTING") != lump_ids_map.end())
		bspx_ParseLightRGB(bsp, pF, &header->lumps[lump_ids_map["RGBLIGHTING"]]);
	
	if(lump_ids_map.find("DECOUPLED_LM") != lump_ids_map.end())
		bspx_ParseDecoupledLM(bsp, pF, &header->lumps[lump_ids_map["DECOUPLED_LM"]]);

	if(lump_ids_map.find("BRUSHLIST") != lump_ids_map.end()) 
		bspx_ParseBrushList(bsp, pF, &header->lumps[lump_ids_map["BRUSHLIST"]]);
}

void bsp_UnloadExtra(Bsp_Data *bsp) {
	if(bsp->bspx_lm.rgb)			free(bsp->bspx_lm.rgb);
	if(bsp->bspx_lm.offsets)		free(bsp->bspx_lm.offsets);	
	if(bsp->bspx_lm.entries)		free(bsp->bspx_lm.entries);
	bsp->bspx_lm = (Bspx_Lightmap) {0};
	
	if(bsp->brush_lists) {
		for(u32 i = 0; i < bsp->num_models; i++) {
			BrushList *list = &bsp->brush_lists[i];

			for(i32 j = 0; j < list->num_brushes; j++) {
				Brush *brush = &list->brushes[j];

				if(brush->planes) free(brush->planes);
				*brush = (Brush) {0};
			}

			bsp->brush_lists[i] = (BrushList) {0};
		}

		free(bsp->brush_lists);
	}

	if(bsp->brush_refs) {
		for(u32 i = 0; i < bsp->num_nodes; i++) bsp->brush_refs[i].brushes.clear();
		free(bsp->brush_refs);
	} 
}

void bspx_ParseBrushList(Bsp_Data *bsp, FILE *pF, Bspx_Lump *lump) {
	bsp->brush_lists = (BrushList*)calloc(bsp->num_models, sizeof(BrushList));
	fseek(pF, lump->offset, SEEK_SET);

	for(u32 i = 0; i < bsp->num_models; i++) {
		BrushList *list = &bsp->brush_lists[i]; 

		fread(&list->version, sizeof(i32), 1, pF);
		fread(&list->model_id, sizeof(i32), 1, pF);
		fread(&list->num_brushes, sizeof(i32), 1, pF);
		fread(&list->num_planes, sizeof(i32), 1, pF);

		list->brushes = (Brush*)calloc(list->num_brushes, sizeof(Brush));
		for(i32 j = 0; j < list->num_brushes; j++) {
			Brush *brush = &list->brushes[j];

			fread(&brush->aabb, sizeof(BoundingBox), 1, pF); 
			fread(&brush->contents, sizeof(i16), 1, pF);
			fread(&brush->num_planes, sizeof(u16), 1, pF);
		
			brush->planes = (Bsp_Plane*)calloc(brush->num_planes, sizeof(Bsp_Plane));
			fread(brush->planes, sizeof(Bsp_Plane) * brush->num_planes, 1, pF);
		}
	}
}

void bspx_ParseLightRGB(Bsp_Data *bsp, FILE *pF, Bspx_Lump *lump) {
	bsp->bspx_lm.rgb = (u8*)malloc(lump->length);

	fseek(pF, lump->offset, SEEK_SET);
	fread(bsp->bspx_lm.rgb, lump->length, 1, pF);
}

void bspx_ParseDecoupledLM(Bsp_Data *bsp, FILE *pF, Bspx_Lump *lump) {
	i32 num_entries = lump->length / sizeof(Lm_DecoupledEntry);
	bsp->bspx_lm.entries = (Lm_DecoupledEntry*)calloc(num_entries, sizeof(Lm_DecoupledEntry));

	fseek(pF, lump->offset, SEEK_SET);
	fread(bsp->bspx_lm.entries, sizeof(Lm_DecoupledEntry) * num_entries, 1, pF);
}

BoundingBox B32_ToBox(Bsp_AABB b32) {
	return (BoundingBox) {
		.min = (Vector3) { (float)b32.min[0], (float)b32.min[1], (float)b32.min[2] },
		.max = (Vector3) { (float)b32.max[0], (float)b32.max[1], (float)b32.max[2] }
	};
}

Bsp_Hull bsp_BuildHull(Bsp_Data *bsp, i32 hull_id) {
	Bsp_Hull hull = (Bsp_Hull) {
		.planes = bsp->planes,
		.clipnodes = bsp->clipnodes,
		.first_clipnode = bsp->models[0].head_nodes[hull_id],
		.last_clipnode = (i32)bsp->num_clipnodes - 1,
		.num_planes = (i32)bsp->num_planes
	};

	return hull;
}

void bspx_MakeBrushRefs(Bsp_Data *bsp, u32 submodel) {
	BrushList *brush_list = &bsp->brush_lists[submodel];

	bsp->brush_refs = (BrushRef*)calloc(bsp->num_nodes, sizeof(BrushRef));

	for(i32 j = 0; j < brush_list->num_brushes; j++) {
		Brush *brush = &brush_list->brushes[j];

		for(u32 i = 0; i < bsp->num_nodes; i++) {
			Bsp_Node *node = &bsp->nodes[i];
			BoundingBox node_box = B32_ToBox(node->aabb);

			if(brush->contents != CONTENTS_SOLID) 				continue;	// Skip non-solid
			if((node->children[0] ^ node->children[1]) > 0) 	continue;	// Skip branching nodes
			if(!CheckCollisionBoxes(node_box, brush->aabb)) 	continue;	// Skip non-colliding

			bsp->brush_refs[i].brushes.push_back(j);
		}
	}
}

