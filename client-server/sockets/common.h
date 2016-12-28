#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

#define STRUCT_PORT 5558
#define CPE(val, msg, err_code) \
	if(val) { fprintf(stderr, msg); fprintf(stderr, "\n"); exit(err_code);}

struct my_struct {
	int a;
	int b;
};
