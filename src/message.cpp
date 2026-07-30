#include <stdio.h>
#include "message.h"

u8 log_state = 1;
void msg_SetLogState(u8 state) {
	log_state = state;
}

void msg_Print(std::string text, const char *color) {
	if(!(log_state & MSG_INFO))
		return;

	const char *t = text.c_str();
	printf("%s%s%s\n", color, t, ANSI_WHITE);
}

void msg_LogError(std::string text, std::string param) {
	if(!(log_state & MSG_ERROR))
		return;

	const char *t = text.c_str();
	const char *p = param.c_str();
	printf("%s[ERROR] %s%s%s\n", ANSI_RED, t, p, ANSI_WHITE);
}

void msg_LogWarning(std::string text, std::string param) {
	if(!(log_state & MSG_WARN))
		return;

	const char *t = text.c_str();
	const char *p = param.c_str();
	printf("%s[WARNING] %s%s%s\n", ANSI_YELLOW, t, p, ANSI_WHITE);
}

