#include "common.h"

void error(char *msg)
{
	perror(msg);
	exit(1);
}

//Usage: ./server
int main(int argc, char *argv[])
{
	int sockfd, newsockfd;
	struct sockaddr_in serv_addr;

	struct my_struct *send_struct;
	send_struct = malloc(sizeof(*send_struct));
	memset(send_struct, 0 , sizeof(*send_struct));
	send_struct->a = 3185;
	send_struct->b = 3187;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		error("ERROR opening socket");

	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(STRUCT_PORT);

	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
		error("ERROR on binding");

	printf("Server listening...\n");
	
	while(1) {
		listen(sockfd, 5);
	
		newsockfd = accept(sockfd, NULL, NULL);
		if (newsockfd < 0)
			error("ERROR on accept");
		
		printf("Server: write() in 5 seconds\n");
		sleep(5);
		printf("Server: write()!\n");
		if (write(newsockfd, send_struct, sizeof(*send_struct)) < 0)
			error("ERROR writing to socket");
	
		printf("Server done!");
	}
	return 0; 
}
