#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>

#include <ne_session.h>
#include <ne_uri.h>
#include <ne_basic.h>
#include <ne_utils.h>
#include <ne_redirect.h>

#include "httpc.h"



int main(int argc, char **argv) {
    struct http_filesave_res* res;
    if (argc < 2) {
	printf("Usage: nget URL.\n");
	return -1;
    }

    res = http_req_savefile(argv[1], "./", "myfile");
    if (res->code != HTTP_OK) {
	printf("error in request %d\n", res->code);
	printf("error string: %s\n", res->httprsp);
    } else {
	printf("res saved to: %s\n", res->filename);
	printf("content type: %s\n", res->ctype);
    }
    http_free_file_res(res); 
    return 1;
}
