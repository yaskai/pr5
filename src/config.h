#pragma once

#include <unordered_map>
#include <string>
#include "common/nums.h"

#define CONF_COMMENT_MARKER '#'
#define CONF_BLOCK_OPEN		'['
#define CONF_BLOCK_CLOSE	']'

typedef struct {
	float ww, wh;
	
	u8 flags;

} Config;

void conf_Init(Config *conf);
u8 conf_ReadFile(Config *conf, const char* path);
u8 conf_ParseLine(Config *conf, char *line, char *block);

int conf_GetOptionValue(std::string key);
void conf_SetOptionValue(std::string key, int val);
