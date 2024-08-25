#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<string.h>
#include "webserver.h"

#define BUFFER_SIZE 10240

typedef struct {
  char *path;
  char *method;
  request_handler_func handler;
} request_handler_entry;

struct _webserver{
  struct sockaddr_in addr;
  int port;
  int server_fd;

  request_handler_entry *handlers;
  size_t handlers_capacity;
  size_t handlers_count;
};

struct _http_header {
  char* key;
  char* value;
};


int request_receive_data(int, char*);
request* request_parse(const char*);
void request_destroy(request*);
void webserver_handle(webserver* ws, char *path, request_handler_func handler, char*  method);
request_handler_func request_match_handler(request*);

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
  ws->handlers_count = 0;
  ws->handlers_capacity = 10;
  ws->handlers = malloc(ws->handlers_capacity * sizeof(request_handler_entry));

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
    
    char *buffer = (char*) malloc(BUFFER_SIZE * sizeof(char)+1);

    int bytes_received = request_receive_data(client_fd, buffer);
    if (bytes_received <=0 ) {
      perror("unable to receive data from client");
      continue;
    }

    request *req = request_parse(buffer);
    req->webserver = ws;
    req->client_fd = client_fd;

    printf("Got request %s %s\n", req->method, req->path);

    request_handler_func handler = request_match_handler(req);
    if(handler) {
      handler(req);
    } else{
      // TODO implement 404 route
    }

    fflush(stdout);

    close(client_fd);
    free(buffer);
    request_destroy(req);
  }

  return 0;
}

int request_receive_data(int socket_fd, char* buffer) {
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

request* request_parse(const char* buffer) {
  request *req = malloc(sizeof(*req));
  char *copy = strdup(buffer);
  char *saveptr;
  char *token;

  token = strtok_r(copy," ", &saveptr);
  req->method = strdup(token);

  token = strtok_r(saveptr," ", &saveptr);
  req->path = strdup(token);

  saveptr = strchr(saveptr, '\n') + 1;

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
  while((token = strtok_r(NULL, "\r\n", &saveptr)) && i < header_count) {
    if(token[0] == '\r') {
      break;
    }
    http_header *head = malloc(sizeof(*head));
    char *pos = strchr(token, ':');
    head->key = strndup(token, pos-token);
    head->value = strdup(pos+2);
    req->headers[i++] = head;
  }

  req->body = strdup(pos + 4);
  free(copy);
  return req;
}

void request_destroy(request* req) {
  free(req->body);
  free(req->method);
  free(req->path);
  for(int i=0;i<req->headers_count;i++) {
    free(req->headers[i]->key);
    free(req->headers[i]->value);
    free(req->headers[i]);
  }
  free(req->headers);
  free(req);
}

void webserver_handle(webserver* ws, char *path, request_handler_func handler, char*  method) {
  if(ws->handlers_count >= ws->handlers_capacity) {
    ws->handlers_capacity *=2;
    ws->handlers = realloc(ws->handlers, ws->handlers_capacity * sizeof(request_handler_entry));
  }
  size_t i = ws->handlers_count;
  ws->handlers[i].method = method;
  ws->handlers[i].path = path;
  ws->handlers[i].handler = handler;
  ws->handlers_count++;
}

void webserver_handle_get(webserver* ws, char *path, request_handler_func handler) {
  webserver_handle(ws, path, handler, METHOD_GET);
}

void webserver_handle_post(webserver* ws, char *path, request_handler_func handler) {
  webserver_handle(ws, path, handler, METHOD_POST);
}

void webserver_handle_put(webserver* ws, char *path, request_handler_func handler) {
  webserver_handle(ws, path, handler, METHOD_PUT);
}

void webserver_handle_delete(webserver* ws, char *path, request_handler_func handler) {
  webserver_handle(ws, path, handler, METHOD_DELETE);
}

void webserver_handle_patch(webserver* ws, char *path, request_handler_func handler) {
  webserver_handle(ws, path, handler, METHOD_PATCH);
}

void webserver_handle_head(webserver* ws, char *path, request_handler_func handler) {
  webserver_handle(ws, path, handler, METHOD_HEAD);
}

request_handler_func request_match_handler(request* req) {
  webserver *ws = req->webserver;
  for (size_t i =0; i< ws->handlers_count; i++) {
    if(strcmp(ws->handlers[i].method, req->method) == 0 ) {
      // TODO enhance later
      if(strcmp(ws->handlers[i].path, req->path) == 0 ) {
        return ws->handlers[i].handler;
      }
    }
  }
  return NULL;
}

void request_send_text(request *req, int status, char *text) {
  char *response = malloc(strlen(text) + 50);
  // TODO handle other status types
  char *status_text = "OK";

  sprintf(response, "HTTP/1.1 %d %s\r\n"
          "Content-Type: text/plain\r\n\r\n"
          "%s\r\n",
          status,
          status_text,
          text);

  write(req->client_fd, response, strlen(response));

  free(response);
}
