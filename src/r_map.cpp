#include <cstdlib>
#include <cstring>
#include <string>
#include <string.h>
#include <unordered_map>
#include <vector>
#include "bsp.h"
#include "lit.h"
#include "raylib.h"
#include "raymath.h"
#include "r_map.h"
#include "geo.h"
#include "vis.h"
#include "rlgl.h"

static const char* tex_dir = "./tools/Disruptor/textures/custom/";
std::unordered_map<std::string, Texture2D> tex_resource_map;

Shader lm_shader;

Model *r_BspLeafToModels(Bsp_Data *bsp, Lightmap *lm, i32 leaf_id, int *n_out, u8 *flags) {
	Model *models = NULL;

	Bsp_Leaf *leaf = &bsp->leaves[leaf_id];

	std::vector<u32> used_texs;

	std::unordered_map<u32, Mesh> mesh_map;
	std::unordered_map<u32, int> cursor_map;
	std::unordered_map<u32, int> n_tris_map;
	std::unordered_map<u32, std::string> mipname_map;

	for(u16 i = 0; i < leaf->num_faces; i++) {
		u16 face_id = bsp->list_faces.faces[leaf->first_face + i];

		Bsp_Face *face = &bsp->faces[face_id];
		Bsp_Texinfo *texinfo = &bsp->texinfos[face->texinfo];
		Bsp_Miptex *mip = &bsp->miptexs[texinfo->miptex_id];

		if(!n_tris_map[texinfo->miptex_id])
			used_texs.push_back(texinfo->miptex_id);

		n_tris_map[texinfo->miptex_id] += face->num_edges - 2;
		mipname_map[texinfo->miptex_id] = mip->name;
	}

	// Allocate memory for mesh data (vertices, normals, texture coordinates etc.) 
	for(u32 tex_id : used_texs) {
		if(!n_tris_map[tex_id]) continue;
		cursor_map[tex_id] = 0;

		Mesh *mesh = &mesh_map[tex_id]; 
		
		mesh->triangleCount = n_tris_map[tex_id];
		mesh->vertexCount 	= n_tris_map[tex_id] * 3;

		mesh->vertices 		= (float*)MemAlloc(sizeof(float) * mesh->vertexCount * 3); 
		mesh->normals 		= (float*)MemAlloc(sizeof(float) * mesh->vertexCount * 3);

		mesh->texcoords 	= (float*)MemAlloc(sizeof(float) * mesh->vertexCount * 2); 
		mesh->texcoords2 	= (float*)MemAlloc(sizeof(float) * mesh->vertexCount * 2);
	}

	// Set data
	for(u16 i = 0; i < leaf->num_faces; i++) {
		u16 face_id = bsp->list_faces.faces[leaf->first_face + i];
		Bsp_Face *face = &bsp->faces[face_id];

		Bsp_Texinfo *texinfo = &bsp->texinfos[face->texinfo];
		Bsp_Miptex *mip = &bsp->miptexs[texinfo->miptex_id];

		if(n_tris_map[texinfo->miptex_id] <= 0)
			continue;

		Vector3 face_verts[face->num_edges];
		Mesh *mesh = &mesh_map[texinfo->miptex_id];
		int *vert_id = &cursor_map[texinfo->miptex_id];

		for(i32 j = 0; j < face->num_edges; j++) {
			i32 list_edge = bsp->list_edges.edges[face->first_edge + j];

			face_verts[j] = (list_edge >= 0
				? bsp->vertices[bsp->edges[list_edge].v[0]] 
				: bsp->vertices[bsp->edges[-list_edge].v[1]] 
			);
		}

		Vector3 normal = *(Vector3 *)bsp->planes[face->plane].normal;
		if(face->side) normal = Vector3Negate(normal);

		Lm_DecoupledEntry *lm_entry = &bsp->bspx_lm.entries[face_id];
		Rectangle *uv = &lm->uvs[face_id];

		// Copy face data to mesh
		for(i32 j = 1; j < face->num_edges - 1; j++) {
			// Build triangle with correct winding order
			Tri tri = (Tri) { .vertices = { face_verts[0], face_verts[j+1], face_verts[j] }, .normal = normal }; 

			for(i8 k = 0; k < 3; k++) {
				// Vertices
				memcpy(&mesh->vertices[*vert_id*3], &tri.vertices[k], sizeof(Vector3));
				// Normal
				memcpy(&mesh->normals[*vert_id*3], &tri.normal, sizeof(Vector3));
				// Texture coordinates				
				mesh->texcoords[*vert_id*2+0] = (Vector3DotProduct(tri.vertices[k], texinfo->vec_s) + texinfo->dist_s) / mip->width;
				mesh->texcoords[*vert_id*2+1] = (Vector3DotProduct(tri.vertices[k], texinfo->vec_t) + texinfo->dist_t) / mip->height;
				// Lightmap coordinates
				float lm_u = Vector3DotProduct(tri.vertices[k], lm_entry->vec_s) + lm_entry->dist_s;
				float lm_v = Vector3DotProduct(tri.vertices[k], lm_entry->vec_t) + lm_entry->dist_t;
				mesh->texcoords2[*vert_id*2+0] = (uv->x + lm_u + 0.5f) / lm->tex.width; 
				mesh->texcoords2[*vert_id*2+1] = (uv->y + lm_v + 0.5f) / lm->tex.height;
				// Increment vertex cursor
				(*vert_id)++;
			}
		}
	}

	models = (Model*)malloc(sizeof(Model) * used_texs.size());
	*n_out = 0;

	for(u32 tex_id : used_texs) {
		if(n_tris_map[tex_id] <= 0)		continue;
		if(tex_id >= bsp->num_miptexs) 	continue;

		Mesh *mesh = &mesh_map[tex_id];
		UploadMesh(mesh, false);

		Model model = LoadModelFromMesh(*mesh);
		model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = tex_resource_map[mipname_map[tex_id]];
		model.materials[0].maps[MATERIAL_MAP_METALNESS].texture = lm->tex;
		model.materials[0].shader = lm_shader;

		// Get mip prefix
		char pref[3]; 
		const char *mip_str = mipname_map[tex_id].c_str();
		memcpy(pref, mip_str, 3);

		// Set conditional flags
		if(strncmp(pref, "sky", 3) == 0) flags[*n_out] |= RBRUSH_FLAG_SKIP;
		if(pref[0] == '{') flags[*n_out] |= RBRUSH_FLAG_TRANSLUCENT;

		models[(*n_out)++] = model;
	}

	return models;
}

r_Brush *r_BspLeafToRenderBrushes(Bsp_Data *bsp, Lightmap *lm, int32_t leaf_id, int *n_out) {
	r_Brush *rbrushes = NULL;
	*n_out = 0;

	int model_count = 0;
	u8 *flags = (u8*)calloc(128, sizeof(u8));
	Model *models = r_BspLeafToModels(bsp, lm, leaf_id, &model_count, flags);
	
	rbrushes = (r_Brush*)malloc(sizeof(r_Brush) * model_count);
	for(int i = 0; i < model_count; i++) {
		r_Brush rb = (r_Brush) {0};
		rb.model = models[i];
		rb.aabb = GetModelBoundingBox(rb.model);
		rb.flags = flags[i];
		rb.leaf = leaf_id;

		rbrushes[(*n_out)++] = rb;
	}

	return rbrushes;
}

#define RMAP_START_CAP 256
void r_Map::Build(Bsp_Data *bsp) {
	Unload();

	if(!IsShaderValid(lm_shader)) {
		lm_shader = LoadShader("./resources/shaders/lit_v.glsl", "./resources/shaders/lit_f.glsl");
	}

	i32 mode = 0;
	f32 overbright = 1.5f;
	SetShaderValue(lm_shader, GetShaderLocation(lm_shader, "overbright"), &overbright, SHADER_UNIFORM_FLOAT);
	SetShaderValue(lm_shader, GetShaderLocation(lm_shader, "draw_mode"), &mode, SHADER_UNIFORM_INT);
	lm = lm_Construct(bsp);

	for(u32 i = 0; i < bsp->num_miptexs; i++) {
		Bsp_Miptex *mip = &bsp->miptexs[i];
		if(tex_resource_map.find(mip->name) != tex_resource_map.end())
			continue;

		tex_resource_map[mip->name] = LoadTexture(TextFormat("%s%s.png", tex_dir, mip->name));
	}

	rbrush_cap = RMAP_START_CAP; 
	rbrushes = (r_Brush*)malloc(sizeof(r_Brush) * rbrush_cap);

	for(u32 i = 0; i < bsp->num_leaves; i++) {
		i32 n = 0;
		r_Brush *temp = (r_Brush*)r_BspLeafToRenderBrushes(bsp, &lm, i, &n);
		for(int j = 0; j < n; j++) Push_rBrush(temp[j]);
	}

	rbrushes = (r_Brush*)realloc(rbrushes, sizeof(r_Brush) * rbrush_count);
	vis_list = (bool*)calloc(bsp->num_leaves, sizeof(bool));
}

void r_Map::Unload() {
	if(IsShaderValid(lm_shader)) UnloadShader(lm_shader);

	for(auto &[str, tex] : tex_resource_map) if(IsTextureValid(tex)) UnloadTexture(tex);
	tex_resource_map.clear();

	for(u32 i = 0; i < rbrush_count; i++) {
		UnloadModel(rbrushes[i].model); 
		rbrushes[i] = (r_Brush) {0};
	}

	rbrush_count = 0;
	rbrush_cap = 0;
	if(rbrushes) free(rbrushes);
	if(vis_list) free(vis_list);
}

void r_Map::Draw(Bsp_Data *bsp, Vector3 camera_position) {
	/*
	i32 curr_leaf = vis_FindLeaf(bsp, camera_position);

	for(u32 i = 0; i < rbrush_count; i++) { 
		r_Brush *rbrush = &rbrushes[i];
		if(rbrush->flags & RBRUSH_FLAG_SKIP) continue;
		if(!vis_IsLeafVisible(bsp, curr_leaf, rbrush->leaf)) continue;

		DrawModel(rbrush->model, Vector3Zero(), 1.0f, WHITE);
	}
	*/

	UpdateVis(bsp, camera_position);
	
	for(u32 i = 0; i < rbrush_count; i++) {
		r_Brush *rbrush = &rbrushes[i];
		// Skip non-visible leaves
		if(!vis_list[rbrush->leaf]) continue;
		// Skip brushes with designated flag 
		if(rbrush->flags & RBRUSH_FLAG_SKIP) continue;
		// Render rbrush models
		DrawModel(rbrush->model, Vector3Zero(), 1.0f, WHITE);
	}
}

void r_Map::UpdateVis(Bsp_Data *bsp, Vector3 camera_position) {
	// Get current occupied leaf index at camera position
	i32 curr_leaf = vis_FindLeaf(bsp, camera_position);	

	Bsp_Leaf *leaf = &bsp->leaves[curr_leaf];	
	if(leaf->vis_ofs < 0) {
		// No vis data for leaf, render everything
		memset(vis_list, 1, bsp->num_leaves);
		return;
	}

	u8 *vis = bsp->vis + leaf->vis_ofs;
	u32 leafnum = 1;

	while(leafnum < bsp->num_leaves) {
		if(*vis == 0) {
			vis++;
			u32 run = (u32)(*vis) * 8;
			vis++;
			u32 count = (run < bsp->num_leaves - leafnum) ? run : (bsp->num_leaves - leafnum);
			memset(&vis_list[leafnum], 0, count);
			leafnum += run;
		} else {
			u8 byte = *vis++;
			for(i32 bit = 0; bit < 8 && leafnum < bsp->num_leaves; bit++, leafnum++) {
				vis_list[leafnum] = (byte >> bit) & 1;
			}
		}
	}

	vis_list[curr_leaf] = 1;
}

