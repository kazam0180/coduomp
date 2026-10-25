#ifndef CODUOMP_SYSTEM_CONSOLE_H
#define CODUOMP_SYSTEM_CONSOLE_H

#include "q_shared.h"

void Sys_Print(const char *text);
char *Sys_ConsoleInput(void);
void Sys_CreateConsole(void);
void Sys_DestroyConsole(void);
void Sys_SetErrorText(const char *text);
void Sys_ShowConsole(int32_t visibility, qboolean quitOnClose);
void AppActivate(qboolean active, qboolean minimized);

#endif
