#pragma once

#include <iostream>
#include "common/nums.h"

#define ANSI_BLACK		"\033[30m"
#define ANSI_RED		"\033[31m"
#define ANSI_GREEN		"\033[32m"
#define ANSI_YELLOW		"\033[33m"
#define ANSI_BLUE		"\033[34m"
#define ANSI_MAGENTA	"\033[35m"
#define ANSI_CYAN		"\033[38m"
#define ANSI_WHITE		"\033[39m"
#define ANSI_DEFAULT	"\033[40m"

#define MSG_ERROR		0x01
#define MSG_WARN		0x02
#define MSG_INFO		0x04

void msg_SetLogState(u8 state);

void msg_Print(std::string text, const char *color);
void msg_LogError(std::string text, std::string param);
void msg_LogWarning(std::string text, std::string param);

