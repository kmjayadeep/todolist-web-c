#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<string.h>
#include "webserver.h"

#define BUFFER_SIZE 10240

struct _webserver{
  struct sockaddr_in addr;
  int port;
  int server_fd;
};

int receive_request_data(int, char*);

webserver* webserver_create(int port) {
  int server_fd;
  int opt = 1;
  webserver* ws = malloc(sizeof(*ws));
  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("cannot create socket"); 
    return NULL;
  }

  if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
    close(server_fd);
    return NULL;
  }

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(port);

  ws->server_fd = server_fd;
  ws->port = port;
  ws->addr = addr;

  return ws;
}

int webserver_run(webserver *ws) {
  if(bind(ws->server_fd, (struct sockaddr *)&ws->addr, sizeof(ws->addr)) < 0) {
    perror("bind failed");
    close(ws->server_fd);
    return -1;
  }

  if(listen(ws->server_fd, 3) < 0) {
    perror("listen failed");
    close(ws->server_fd);
    return -1;
  }

  printf("Web server listening in port :%d\n", ws->port);

  while(1) {
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int client_fd;
  
    if((client_fd = accept(ws->server_fd,
                           (struct sockaddr*)&client_addr,
                           &client_addr_len)) < 0) {
      perror("unable to accept connection");
      continue;
    }

    char ip_addr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(client_addr.sin_addr), ip_addr, INET_ADDRSTRLEN);
    
    printf("Got connection from %s:%d\n", ip_addr, client_addr.sin_port);

    char *buffer = (char*) malloc(BUFFER_SIZE * sizeof(char)+1);

    int bytes_received = receive_request_data(client_fd, buffer);
    if (bytes_received <=0 ) {
      perror("unable to receive data from client");
      continue;
    }

    printf("%s\n", buffer);
    fflush(stdout);

    close(client_fd);
    free(buffer);
  }

  return 0;
}

int receive_request_data(int socket_fd, char* buffer) {
  int total_bytes_received = 0;

  while(1) {
    ssize_t bytes_received = recv(socket_fd,
                                  buffer+total_bytes_received,
                                  BUFFER_SIZE - total_bytes_received,
                                  0
                                  );
    if(bytes_received < 0) {
      perror("unable to receive data from the socket");
      return -1;
    }
    if(bytes_received == 0) {
      break;
    }
    buffer[total_bytes_received+bytes_received] = 0;
    // Detect end of http request
    if(strstr(buffer+total_bytes_received, "\r\n\r\n")) {
      total_bytes_received += bytes_received;
      break;
    }
    total_bytes_received += bytes_received;

    if(total_bytes_received >= BUFFER_SIZE){
      break;
    }
  }

  return total_bytes_received;
}
