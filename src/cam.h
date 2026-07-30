#pragma once

#include "raylib.h"
#include "common/nums.h"
#include "ent.h"

#define CAM_TILT_MAX	0.015f
#define CAM_Z_OFFSET	56.0f

void cc_TriggerStepFrame();
void cc_EndStepFrame();

typedef struct {
	// Pointer to camera struct
	Camera3D *camera;

	// Vertical movement when walking/running
	float bob;
	// Sideways rotation 
	float tilt;

	float z_curr, z_targ;
	float z_speed;

	// Index of attached entity
	u16 entity;

} Cam_Controller;

Cam_Controller cc_Init(Camera3D *camera, u16 entity);

void cc_Resolve(Cam_Controller *cc, Ent_Handler *ent_handler, float alpha);


