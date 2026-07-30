#pragma once

#include "ent.h"

void player_Init(Entity *ent);

void player_Tick(Entity *ent, Ent_Handler *handler, Bsp_Data *bsp, float dt);
void player_Render(Entity *ent, float alpha);
void player_Update(Entity *ent, float dt);

void player_MouseLook(Entity *ent, float dt);

