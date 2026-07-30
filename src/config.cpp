#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <stdio.h>
#include <string.h>
#include "config.h"
#include "message.h"

std::unordered_map<std::string, int> option_map;

void conf_Init(Config *conf) {
	conf->flags = 0;
}

u8 conf_ReadFile(Config *conf, const char *path) {
	FILE *pF = fopen(path, "r"); 
	if(!pF) {
		return 0;
	}
	
	char block[64];
	char line[128];
	while(fgets(line, sizeof(line), pF)) {
		u8 result = conf_ParseLine(conf, line, block);
		if(result == 0) return result;
	}

	fclose(pF);
	return 1;
}

u8 conf_ParseLine(Config *conf, char *line, char *block) {
	if(line[0] == CONF_BLOCK_OPEN) {
		line = line + 1;	// Skip open bracket

		// Remove close bracket
		char *close = strchr(line, CONF_BLOCK_CLOSE);		
		if(close) *close = '\0';
	
		strcpy(block, line);
	}
	
	char *comment_marker = strchr(line, CONF_COMMENT_MARKER);	
	if(comment_marker) {
		char *comment = strdup(comment_marker);
		*comment_marker = '\0';
		
		char *term = strchr(comment, '\n');
		if(*term) *term = '\0';

		msg_Print(comment, ANSI_BLUE);
		free(comment);
	}

	char *eq = strchr(line, '=');
	if(!eq) return 1;

	*eq = '\0';
	char *key = (char*)malloc(64);
	strcpy(key, block);
	key[strlen(key)] = ':';
	strcpy(key + strlen(key), line);
	char *val = eq+1;

	char *space_key = strrchr(key, ' '); 	
	*space_key = '\0';

	msg_Print(key, ANSI_YELLOW);
	
	while(*val == ' ') val++;
	char *space_val = strchr(val, ' ');
	if(space_val) *space_val = '\0';
	
	option_map[key] = atoi(val);

	free(key);
	
	return 1;
}

int conf_GetOptionValue(std::string key) {
	if(option_map.find(key) != option_map.end()) 	
		return option_map[key];

	return 0;	
}

void conf_SetOptionValue(std::string key, int val) {
	option_map[key] = val;
}

