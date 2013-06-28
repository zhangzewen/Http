#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "http_epoll.h"


void connfd_callback(int epfd, int epoll_fd, struct event *ev)
{
	struct sockaddr_in *addr;
	char buff[1024] = {0};
	addr = (struct sockaddr_in *)ev->arg;
	int n = 0;
	char *ip;
	ip = inet_ntoa(addr->sin_addr);
	
	n = read(epoll_fd, buff, 1024);
	if(n < 0){
		event_destroy(epfd, epoll_fd, ev);
		return ;
	}
	printf("from ip:%s =========>%s", ip, buff);
	return;
}




int main(int argc, char *argv[])
{
	int listen_fd;
	int conn_fd;
	int epfd;
	
	struct event *e;
	
	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;
	socklen_t len;
	if(3 != argc) {
		fprintf(stderr, "Usage: %s <server_ip> <server port>", argv[0]);
		exit(1);
	}

	listen_fd = socket(AF_INET, SOCK_STREAM, 0);	
	if(listen_fd < 0) {
		fprintf(stderr, "create socket error!");
		exit(1);
	}

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(atoi(argv[2]));
	if(inet_pton(AF_INET, argv[1], &server_addr.sin_addr) < 0){
		fprintf(stderr, "inet_pton error!\n");
		exit(1);
	}

	len = sizeof(server_addr);

	if(bind(listen_fd, (struct sockaddr *)&server_addr, len) < 0) {
		fprintf(stderr, "bind error!");
		exit(1);
	}
	
	epfd = event_init();
	len = sizeof(client_addr);
	
	if((conn_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &len)) < 0) {
			fprintf(stderr, "accept error!");
			exit(1);
	}

	e = event_set(epfd, conn_fd, EPOLLIN, connfd_callback, (void *)&client_addr);
	
	
	if( NULL == e){
		close(listen_fd);
	}

	if(event_add(e) == -1){
		free(e);
		close(listen_fd);
	}
	
	
	while(1){
		event_dispatch_loop(epfd);
	}
	
	return 0;
}
