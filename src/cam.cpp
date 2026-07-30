#include "config.h"
#include "raylib.h"
#include "raymath.h"
#include "cam.h"
#include "geo.h"

Cam_Controller cc_Init(Camera3D *camera, u16 entity) {
	Cam_Controller cc = (Cam_Controller) {0};
	cc.camera = camera;
	cc.entity = entity;
	cc.z_speed = 20.0f;
	
	return cc;
}

void cc_Resolve(Cam_Controller *cc, Ent_Handler *ent_handler, float alpha) {
	Entity *ent = &ent_handler->ents[cc->entity];

	Vector3 delta = Vector3Subtract(ent->ct.position, ent->ct_prev.position);
	Vector3 h_vel = (Vector3) { delta.x, delta.y, 0.0f };
	Vector3 h_fwd = Vector3Normalize( (Vector3) { ent->ct.forward.x, ent->ct.forward.y, 0.0f } );
	
	float dt = GetFrameTime();
	float t = GetTime();
	
	// Camera walk bobbing:
	if(Vector3Length(h_vel) >= 1.0f && ent->ct.on_ground)
		cc->bob += (sinf(t * 20) * 0.75f) * dt*50;
	else
		cc->bob = Lerp(cc->bob, cc->bob * 0.5f, dt*50);

	// Camera side tilt:
	Vector3 right =  Vector3Normalize(Vector3CrossProduct(h_fwd, WORLD_UP));
	float denom = Vector3DotProduct(Vector3Normalize(h_vel), right);
	denom *= conf_GetOptionValue("graphics:camera_tilt");

	if(fabsf(denom) >= 0.65f && Vector3Length(h_vel) >= 1.0f) {
		cc->tilt += denom * dt;
		cc->tilt = Clamp(cc->tilt, -CAM_TILT_MAX, CAM_TILT_MAX);
		
		Vector3 tilt_targ = Vector3RotateByAxisAngle(WORLD_UP, ent->ct.forward, cc->tilt);
		cc->camera->up = Vector3Lerp(cc->camera->up,tilt_targ, dt*20);

	} else {
		cc->tilt = 0.0f;
		cc->camera->up = Vector3Lerp(cc->camera->up, WORLD_UP, dt*20);
	} 

	// Smooth z axis position setting:
	cc->z_targ = ent->ct.position.z + CAM_Z_OFFSET;
	cc->z_curr = Lerp(cc->z_curr, cc->z_targ, dt * cc->z_speed);

	Vector3 origin = ent->ct_prev.position;
	Vector3 target = ent->ct.position;

	Vector3 pos; 
	pos.x = Lerp(origin.x, target.x, alpha);
	pos.y = Lerp(origin.y, target.y, alpha);
	pos.z = cc->z_curr + cc->bob;

	cc->camera->position = pos;
	cc->camera->target = Vector3Add(cc->camera->position, ent->ct.forward);
}

