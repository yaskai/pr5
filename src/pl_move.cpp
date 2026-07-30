#include <cmath>
#include "bsp.h"
#include "bsp_trace.h"
#include "input.h"
#include "pl_move.h"
#include "geo.h"
#include "raymath.h"

// Acceleration values
#define PM_ACCEL_GROUND 	 	23.0f
#define PM_ACCEL_AIR		 	 4.0f

// Friction values
#define PM_FRICTION_GROUND 		20.25f
#define PM_FRICTION_AIR 	  	 2.11f

// Speed values
#define PM_SPEED_GROUND			500.0f
#define PM_SPEED_AIR			500.0f

// Stair step height:
// if player is grounded and movement obstructed by wall, 
// trace will be moved up by (PM_STEP_Z) units on z axis. 
#define PM_STEP_Z 				32.0f

#define PM_GRAVITY 				2500.0f
#define PM_BASE_JUMP_FORCE		 600.0f

// Remove a vector's z component
Vector3 pm_ClipZ(Vector3 vec) { return Vector3Normalize((Vector3) { vec.x, vec.y, 0 }); }

// Get desired direction of player movemeent
Vector3 pm_GetWishDir(Comp_Transform *ct, InputHandler *input) {
	// Get look direction
	Vector3 look = ct->forward; 
	Vector3 forward = pm_ClipZ(look);
	Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, WORLD_UP));

	// Read keyboard input
	int input_f[2] = {
		(input_GetActionState(ACT_MOV_FWD) 	== INPUT_DOWN ? 1 : 0),
		(input_GetActionState(ACT_MOV_BACK)  == INPUT_DOWN ? 1 : 0)
	};

	int input_s[2] = {
		(input_GetActionState(ACT_MOV_RIGHT) == INPUT_DOWN ? 1 : 0),
		(input_GetActionState(ACT_MOV_LEFT)  == INPUT_DOWN ? 1 : 0)
	};

	Vector3 wish_f = Vector3Scale(forward, input_f[0] - input_f[1]);
	Vector3 wish_s = Vector3Scale(right, input_s[0] - input_s[1]); 

	Vector3 sum = pm_ClipZ(Vector3Add(wish_f, wish_s));
	return Vector3Normalize(sum);
}

// Manage acceleration
void pm_Accelerate(Comp_Transform *ct, Vector3 wish_dir, float wish_speed, float accel, float dt) {
	float cur_speed = Vector3DotProduct(ct->velocity, wish_dir);
	float add_speed = wish_speed - cur_speed;
	if(add_speed <= 0) return;

	float accel_speed = accel * dt * wish_speed;
	if(accel_speed >= add_speed) accel_speed = add_speed;

	ct->velocity = Vector3Add(ct->velocity, Vector3Scale(wish_dir, accel_speed));
}

// Return empyt movement trace data struct
Pm_TraceData Pm_TraceDataEmpty() {
	Pm_TraceData pm = (Pm_TraceData) {0};
	
	pm.start_in_solid = -1;
	pm.end_in_solid = -1;

	return pm;
}

// Trace player movement through world
void pm_TraceMove(Bsp_Data *bsp, Comp_Transform *ct, Vector3 start, Vector3 wish_vel, Pm_TraceData *pm, float dt) {
	*pm = Pm_TraceDataEmpty();
	pm->origin = start;
	pm->start_vel = wish_vel;

	Vector3 dest = start, vel = wish_vel;
	float t_remain = dt;

	Vector3 clips[PM_MAX_CLIPS] = {0};
	u8 num_clips = 0;

	Bsp_Hull *hull = &bsp->hulls[1];
	
	for(u8 i = 0; i < PM_MAX_BUMPS; i++) {
		// End slide trace if velocity is too small
		if(Vector3LengthSqr(vel) <= PM_STOP_EPS) break;	

		Vector3 move = Vector3Scale(vel, t_remain); 	

		Bsp_TraceData tr = Bsp_TraceDataEmpty();
		Vector3 p1 = dest;
		Vector3 p2 = Vector3Add(dest, move);

		bsp_RecursiveTraceEx(hull, hull->first_clipnode, 0, 1, p1, p2, &tr);

		// Move destination forward
		dest = Vector3Add(dest, Vector3Scale(move, tr.fraction));

		// No obstruction, do full movement without clipping
		if(tr.fraction >= 1.0f) break;

		if(num_clips < PM_MAX_CLIPS) {
			clips[num_clips++] = *(Vector3*) tr.plane.normal;
		} else 
			break;
		
		// Clip against planes
		for(u8 j = 0; j < num_clips; j++) {
			float into = Vector3DotProduct(vel, clips[j]);
			if(into >= 0.0f) continue; 

			pm_ClipVelocity(vel, clips[j], &vel, 1.005f);
		}

		t_remain *= (1 - tr.fraction);
	}
	
	pm->move_dist = Vector3Distance(start, dest);
	pm->fraction = pm->move_dist / Vector3Length(wish_vel);

	pm->end_vel = vel; 
	pm->end_pos = dest;
}

// Check if grounded
bool pm_CheckGround(Comp_Transform *ct, Bsp_Data *bsp, Vector3 position) {
	Bsp_Hull *hull = &bsp->hulls[1]; 

	Bsp_TraceData tr = Bsp_TraceDataEmpty();
	Vector3 p1 = ct->position;
	Vector3 p2 = Vector3Add(p1, Vector3Scale(WORLD_DOWN, 1.0f));

	bsp_RecursiveTraceEx(hull, hull->first_clipnode, 0, 1, p1, p2, &tr);	
	if(tr.fraction < 1.0f && tr.plane.normal[2] >= PM_FLOOR_NORM_Z) {
		ct->ground_normal = *(Vector3*)tr.plane.normal;
		return true;
	} 
	
	ct->ground_normal = Vector3Zero();
	return false;
}

void pm_GroundMove(Comp_Transform *ct, Pm_TraceData *pm, Vector3 start, Vector3 wish_vel, Bsp_Data *bsp, float dt) {
	Pm_TraceData base_pm = Pm_TraceDataEmpty();	
	pm_TraceMove(bsp, ct, start, wish_vel, &base_pm, dt);

	*pm = base_pm;
	if(Vector3LengthSqr(base_pm.start_vel) <= PM_STOP_EPS) return;
	if(base_pm.fraction >= 1.0f) return;

	Vector3 step_start = start; 
	step_start.z += PM_STEP_Z;

	Pm_TraceData step_pm = Pm_TraceDataEmpty();
	pm_TraceMove(bsp, ct, step_start, (Vector3) { wish_vel.x, wish_vel.y, 0.0f }, &step_pm, dt);

	float dist_base = Vector2Distance( (Vector2) { base_pm.origin.x, base_pm.origin.y }, (Vector2) { base_pm.end_pos.x, base_pm.end_pos.y } );
	float dist_step = Vector2Distance( (Vector2) { base_pm.origin.x, base_pm.origin.y }, (Vector2) { step_pm.end_pos.x, step_pm.end_pos.y } );
	
	Bsp_TraceData tr = Bsp_TraceDataEmpty();
	Vector3 p1 = step_pm.end_pos;
	Vector3 p2 = Vector3Add(p1, Vector3Scale(WORLD_DOWN, PM_STEP_Z));
	bsp_RecursiveTraceEx(&bsp->hulls[1], bsp->hulls[1].first_clipnode, 0, 1, p1, p2, &tr);
	
	bool use_step = (dist_step > dist_base) && tr.plane.normal[2] >= 1.0f;

	if(tr.start_solid || tr.all_solid) 
		use_step = false;

	if(!use_step) {
		*pm = base_pm;
		return;
	}

	Vector3 down_vel = Vector3Scale(WORLD_DOWN, PM_STEP_Z);

	Pm_TraceData down_pm = Pm_TraceDataEmpty();
	down_pm.origin = step_pm.end_pos;
	pm_TraceMove(bsp, ct, down_pm.origin, down_vel, &down_pm, 1);

	float down_dist = Vector2Distance( (Vector2) { base_pm.origin.x, base_pm.origin.y }, (Vector2) { down_pm.end_pos.x, down_pm.end_pos.y } );
	
	tr = Bsp_TraceDataEmpty();
	p1 = step_pm.end_pos;
	p2 = Vector3Add(p1, Vector3Scale(WORLD_DOWN, PM_STEP_Z));
	bsp_RecursiveTraceEx(&bsp->hulls[1], bsp->hulls[1].first_clipnode, 0, 1, p1, p2, &tr);

	bool use_down = false;
	if(down_pm.fraction < 1.0f && (down_dist > dist_base + 0.001f) && (down_pm.end_pos.z > base_pm.end_pos.z + 0.1f)) use_down = true;
	if(tr.start_solid && tr.all_solid) use_down = false;
	if(tr.plane.normal[2] < 1.0f) use_down = false;

	if(use_down) { 
		step_pm.end_pos.z = down_pm.end_pos.z;
	} else {
		*pm = base_pm;
		return;
	}

	*pm = step_pm;
}

void pm_ApplyGravity(Comp_Transform *ct, float dt) {
	if(ct->on_ground) { 
		return;
	}

	ct->velocity.z -= PM_GRAVITY * dt;
}

void pm_Friction(Comp_Transform *ct, float dt) {
	float val = (ct->on_ground) ? PM_FRICTION_GROUND : PM_FRICTION_AIR;

	Vector3 vel = ct->velocity;
	vel.z = 0;

	float speed = Vector3Length(vel);

	if(speed < 0.01f) {
		ct->velocity.x = 0;
		ct->velocity.y = 0;

		return;
	}

	float remove = speed * val * dt;
	float new_speed = fmaxf(speed - remove, 0);
	vel = Vector3Scale(vel, new_speed / speed);

	ct->velocity.x = vel.x;
	ct->velocity.y = vel.y;
}

void pm_ClipVelocity(Vector3 in, Vector3 normal, Vector3 *out, float bounce) {
	float backoff = Vector3DotProduct(in, normal) * bounce;

	Vector3 change = Vector3Scale(normal, backoff);
	*out = Vector3Subtract(in, change);

	if(fabsf(out->x) < PM_STOP_EPS) out->x = 0;
	if(fabsf(out->y) < PM_STOP_EPS) out->y = 0;
	if(fabsf(out->z) < PM_STOP_EPS) out->z = 0;
}

void pm_Jump(Comp_Transform *ct, InputHandler *input) {
	if(!ct->on_ground) return;
	if(!input_GetActionState(ACT_JUMP)) return;

	ct->velocity.z += PM_BASE_JUMP_FORCE;
}

void pm_Move(Comp_Transform *ct, Bsp_Data *bsp, Ent_Handler *handler, float dt) {
	InputHandler *input = input_GetPointer();

	Vector3 wish_dir = pm_GetWishDir(ct, input);

	// Check if grounded
	ct->on_ground = pm_CheckGround(ct, bsp, ct->position);

	// Calculate wish velocity
	float wish_speed = (ct->on_ground) ? PM_SPEED_GROUND : PM_SPEED_AIR;
	Vector3 wish_vel = Vector3Scale(wish_dir, wish_speed);

	// Apply friction
	pm_Friction(ct, dt);

	// Accelerate
	float accel = (ct->on_ground) ? PM_ACCEL_GROUND : PM_ACCEL_AIR;
	pm_Accelerate(ct, wish_dir, wish_speed, accel, dt);

	// Stop player from falling through ground
	pm_ClipVelocity(ct->velocity, ct->ground_normal, &ct->velocity, 1.0001f);

	// Check for jump
	pm_Jump(ct, input);

	// Fall down
	pm_ApplyGravity(ct, dt);

	ct->on_ground = pm_CheckGround(ct, bsp, ct->position);

	// Trace movement
	Pm_TraceData pm = Pm_TraceDataEmpty();
	if(ct->on_ground)
		pm_GroundMove(ct, &pm, ct->position, ct->velocity, bsp, dt);
	else
		pm_TraceMove(bsp, ct, ct->position, ct->velocity, &pm, dt);

	// Update transform values 
	ct->velocity = pm.end_vel;
	ct->position = pm.end_pos;
}

