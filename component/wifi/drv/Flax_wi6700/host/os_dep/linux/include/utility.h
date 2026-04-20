#ifndef _UTILITY_H
#define _UTILITY_H

#include <linux/string.h>

int str2args (const char *str, char *argv[], char *delim, int max);
void remove_spaces(char* source);

#endif
