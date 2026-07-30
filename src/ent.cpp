#include <cstdio>
#include <cstring>
#include "bsp.h"
#include "message.h"
#include "player.h"
#include "raylib.h"
#include "raymath.h"
#include "ent.h"

SpawnPoint spawn_point_Make(Bsp_Entity *bsp_ent) {
	SpawnPoint spawn_point = (SpawnPoint) {0};
	
	for(i32 i = 0; i < bsp_ent->num_props; i++) {
		Bsp_EntProperty *prop = &bsp_ent->props[i];

		if(memcmp(prop->key, "classname", strlen("classname")) == 0) 
			memcpy(spawn_point.classname, prop->val, strlen(prop->val));	

		if(memcmp(prop->key, "extra", strlen("extra")) == 0)
			memcpy(spawn_point.extra, prop->val, strlen(prop->val));

		if(memcmp(prop->key, "angle", strlen("angle")) == 0)
			sscanf(prop->val, "%d", &spawn_point.angle);

		if(memcmp(prop->key, "origin", strlen("origin")) == 0) {
			i32 v[3];
			sscanf(prop->val, "%d %d %d", &v[0], &v[1], &v[2]);
			spawn_point.origin = (Vector3) { (f32)v[0], (f32)v[1], (f32)v[2] };
		}
	}

	return spawn_point;
}

void ent_handler_Init(Ent_Handler *handler, Bsp_Data *bsp) {
	Vector3 player_start = Vector3Zero();

	std::vector<SpawnPoint> spawn_points;
	spawn_points.reserve(bsp->num_entities);

	handler->num_ents = bsp->num_entities;
	handler->ents = (Entity*)malloc(sizeof(Entity) * handler->num_ents);

	for(i32 i = 0; i < bsp->num_entities; i++)
		spawn_points.push_back(spawn_point_Make(&bsp->entities[i]));

	for(i32 i = 0; i < spawn_points.size(); i++) {
		SpawnPoint *spawn = &spawn_points[i];
		Entity *ent = &handler->ents[i];

		ent->ct.position = spawn->origin;
		ent->ct_prev = ent->ct;

		if(memcmp(spawn->classname, "info_player_start", strlen("info_player_start")))
			continue;

		handler->cl_player_id = i;
		ent->data.ent_type = ENT_PLAYER;

		player_Init(ent);
	}
}

void ent_handler_Close(Ent_Handler *handler) {
	if(handler->ents) free(handler->ents);
	*handler = (Ent_Handler) {0};
}

void ent_handler_Tick(Ent_Handler *handler, Bsp_Data *bsp, float tick_dt) {
	for(u16 i = 0; i < handler->num_ents; i++) {
		Entity *ent = &handler->ents[i];
		if(!ent->data.ent_type) continue;

		if(ent->data.ent_type == ENT_PLAYER) player_Tick(ent, handler, bsp, tick_dt);
	}
}

void ent_Handler_Render(Ent_Handler *handler, float alpha) {
	for(u16 i = 0; i < handler->num_ents; i++) {
		Entity *ent = &handler->ents[i];
		if(!ent->data.ent_type) continue;

		if(ent->data.ent_type == ENT_PLAYER) player_Render(ent, alpha);
	}
}

void ent_Handler_Update(Ent_Handler *handler, float dt) {
	for(u16 i = 0; i < handler->num_ents; i++) {
		Entity *ent = &handler->ents[i];
		if(!ent->data.ent_type) continue;

		if(ent->data.ent_type == ENT_PLAYER) player_Update(ent, dt);
	}
}

