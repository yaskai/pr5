#pragma once

#include "common/nums.h"
#include "bsp.h"
#include "raylib.h"

enum ENTITY_TYPES : u16 {
	ENT_WORLD	= 0, 
	ENT_PLAYER	= 1,
};

#define ENT_ACTIVE		(0x01)
#define ENT_TRIGGERED	(0x02)

// Base-data component
typedef struct {
	char tag[64];			// Self tag
	char trigger_tag[64];	// Tag of entity to trigger (if any) 

	u16 id;
	u16 ent_type;			// Type of this entity (player, ammo pickup, enemy, etc.)

	u8 flags;

} Comp_Data;

// Transform component
typedef struct {
	BoundingBox bounds;

	Quaternion qrot;
	
	Vector3 position;
	Vector3 velocity;
	Vector3 forward;
	Vector3 ground_normal;
	Vector3 wish_dir;

	float pitch, yaw, roll;

	bool on_ground;

	u8 flags;
	
} Comp_Transform;

// Health component
typedef struct {
	BoundingBox hitbox;	

	i8 amount;			// HP count

	i8 on_hit;			// Index of hit() function pointer in hit functions table
	i8 on_die;			// Index of die() function pointer in die functions table

	u8 flags;

} Comp_Health;

typedef struct {
	Comp_Data data;
	Comp_Transform ct, ct_prev;
	Comp_Health health;

} Entity;

typedef struct {
	char classname[64];
	char extra[64];

	Vector3 origin;

	i32 angle;
	i32 bsp_model;
	u16 id;
	
} SpawnPoint;

SpawnPoint spawn_point_Make(Bsp_Entity *bsp_ent);

typedef struct {
	Entity *ents;		// Entity array
	u16 num_ents;		// Entity count
	u16 cl_player_id;	// Index of player entity controlled by client 
	
} Ent_Handler;

void ent_handler_Init(Ent_Handler *handler, Bsp_Data *bsp);
void ent_handler_Close(Ent_Handler *handler);

void ent_handler_Tick(Ent_Handler *handler, Bsp_Data *bsp, float tick_dt);
void ent_Handler_Render(Ent_Handler *handler, float alpha);
void ent_Handler_Update(Ent_Handler *handler, float dt);


