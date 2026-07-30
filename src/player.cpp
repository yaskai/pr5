#include "raylib.h"
#include "raymath.h"
#include "input.h"
#include "geo.h"
#include "player.h"
#include "pl_move.h"
#include "common/bounds_defs.h"

void player_Init(Entity *ent) {
	Comp_Transform *ct = &ent->ct;

	ct->pitch = 0, ct->yaw = 0, ct->roll = 0;
	ct->forward = Vector3Zero();

	ct->bounds = (BoundingBox) { .min = Vector3Scale(BODY_MEDIUM, -0.5f), .max = Vector3Scale(BODY_MEDIUM, 0.5f) };
	ct->bounds = BoxRecenter(ct->bounds, ct->position);

	ct->velocity = Vector3Zero();
}

void player_Tick(Entity *ent, Ent_Handler *handler, Bsp_Data *bsp, float dt) {
	Comp_Transform *ct = &ent->ct;

	ct->bounds = BoxTranslate(ct->bounds, ct->position);
	pm_Move(ct, bsp, handler, dt);
}

void player_Render(Entity *ent, float alpha) {
}

void player_Update(Entity *ent, float dt) {
	player_MouseLook(ent, dt);
}

void player_MouseLook(Entity *ent, float dt) {
	Vector2 md = input_MouseDelta();	
		
	ent->ct.yaw -= md.x;
	ent->ct.pitch = Clamp(ent->ct.pitch - md.y, -PM_PITCH_MAX, PM_PITCH_MAX);

	Vector3 look = Vector3Normalize( (Vector3) {
		.x = cosf(ent->ct.yaw) 	* cosf(ent->ct.pitch),
		.y = sinf(ent->ct.yaw) 	* cosf(ent->ct.pitch),
		.z = sinf(ent->ct.pitch)
	} );

	ent->ct.forward = look;
}

