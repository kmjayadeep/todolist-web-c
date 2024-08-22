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

struct _http_header {
  char* key;
  char* value;
};

struct _request {
  char* method;
  char* path;
  http_header** headers;
  int headers_count;
  char* body;
};

int receive_request_data(int, char*);
request* parse_request(const char*);

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

    request *req = parse_request(buffer);
    printf("Method:<%s>\n", req->method);
    printf("Path:<%s>\n", req->path);
    printf("Headers:<%d>\n", req->headers_count);

    for(int i=0; i < req->headers_count; i++) {
      printf("%s:%s\n", req->headers[i]->key, req->headers[i]->value);
    }

    printf("Body:<%s>\n", req->body);

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

request* parse_request(const char* buffer) {
  request *req = malloc(sizeof(*req));
  char *rest = strdup(buffer);
  char *token;

  token = strtok_r(rest," ", &rest);
  req->method = token;

  token = strtok_r(rest," ", &rest);
  req->path = token;

  rest = strchr(rest, '\n') + 1;

  char* pos = strstr(buffer, "\r\n\r\n");

  int header_count = 0;
  for(const char *p = buffer; p < pos; p++) {
    if (*p=='\n') {
      header_count++;
    }
  }
  req->headers_count = header_count;
  req->headers = malloc(header_count * sizeof(http_header));

  int i=0;
  while((token = strtok_r(rest, "\n", &rest))) {
    if(token[0] == '\r') {
      break;
    }
    http_header *head = malloc(sizeof(*head));
    head->key = strtok_r(token, ":", &token);
    head->value = token + 1;
    req->headers[i++] = head;
  }

  req->body = rest;

  return req;
}
