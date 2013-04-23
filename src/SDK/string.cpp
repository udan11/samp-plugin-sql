#include "string.h"

int amx_GetString_(AMX *amx, cell param, char *&dest) {
	cell *ptr;
	int len;
	amx_GetAddr(amx, param, &ptr);
	amx_StrLen(ptr, &len);
	dest = (char*) malloc((len + 1) * sizeof(char));
	if (dest != NULL) {
		amx_GetString(dest, ptr, 0, UNLIMITED);
		dest[len] = '\0';
		return len;
	}
	return 0;
}

void amx_SetString_(AMX *amx, cell param, char *str, int len) {
	cell *dest;
	amx_GetAddr(amx, param, &dest);
	amx_SetString(dest, str, 0, 0, len);
}