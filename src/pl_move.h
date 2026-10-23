#pragma once

#include "common/nums.h"
#include "raylib.h"
#include "raymath.h"
#include "ent.h"
#include "input.h"
#include "bsp.h"

#define PM_MAX_CLIPS 			4
#define PM_MAX_BUMPS 			4
#define PM_STOP_EPS 	 		0.001f

#define PM_PITCH_MAX 			(89.0f * DEG2RAD)

#define PM_FLOOR_NORM_Z 		0.7f

typedef struct {
	Vector3 clips[PM_MAX_CLIPS];		// Normals of clipping planes hit during trace 
	
	Vector3 origin;						// Start point of trace
	Vector3 start_vel;					// Velocity before clipping

	Vector3 end_pos;					// Destination point after clipping
	Vector3 end_vel;					// Remaining velocity after clipping  

	float move_dist;					// How many units travelled
	float fraction;						// Ratio of distance moved to requested

	i32 start_in_solid;					// ID of hull inside at origin, -1 if none 
	i32 end_in_solid;					// ID of hull inside at destination, -1 if none 

	u8 num_clips;						// Number of clip planes hit
	u8 block;							// Move block classifier hit ( GROUND, STEP, WALL, etc. )

} Pm_TraceData; 

Vector3 pm_GetWishDir(Comp_Transform *ct, InputHandler *input);
void pm_Accelerate(Comp_Transform *ct, Vector3 wish_dir, float wish_speed, float accel, float dt);
Pm_TraceData Pm_TraceDataEmpty();
void pm_TraceMove(Bsp_Data *bsp, Comp_Transform *ct, Vector3 start, Vector3 wish_vel, Pm_TraceData *pm, float dt);
bool pm_CheckGround(Comp_Transform *ct, Bsp_Data *bsp, Vector3 position);
void pm_Friction(Comp_Transform *ct, float dt);
void pm_GroundMove(Comp_Transform *ct, Pm_TraceData *pm, Vector3 start, Vector3 wish_vel, Bsp_Data *bsp, float dt);
void pm_ApplyGravity(Comp_Transform *ct, float dt);
void pm_ClipVelocity(Vector3 in, Vector3 normal, Vector3 *out, float bounce);
void pm_Jump(Comp_Transform *ct, InputHandler *input);
void pm_Move(Comp_Transform *ct, Bsp_Data *bsp, Ent_Handler *handler, float dt);

