#pragma once

#include <cstdlib>
#include <cstring>

#include "amx/amx.h"

extern int amx_GetString_(AMX* amx, cell param, char *&dest);
extern void amx_SetString_(AMX* amx, cell param, char *str, int len);