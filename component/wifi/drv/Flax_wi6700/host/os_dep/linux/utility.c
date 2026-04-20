#include "utility.h"
#define TRUE 1
#define FALSE 0


void remove_spaces(char* source)
{
	char* i =source;
	char* j =source;
	while(*j!=0)
	{
		*i= *j++;
		if(*i != ' ')
			i++;
	}
	*i=0;
}

char* strtok_r(
    char *str, 
    const char *delim, 
    char **nextp)
{
    char *ret;

    if (str == NULL)
    {
        str = *nextp;
    }

    str += strspn(str, delim);

    if (*str == '\0')
    {
        return NULL;
    }

    ret = str;

    str += strcspn(str, delim);

    if (*str)
    {
        *str++ = '\0';
    }

    *nextp = str;

    return ret;
}

int str2args (const char *str, char *argv[], char *delim, int max)
{
	char *p;
	int n;
	p = (char *) str;
	for (n=0; n < max; n++)
	{
		if (0==(argv[n]=strtok_r(p, delim, &p)))
			break;
	}
	return n;
}

