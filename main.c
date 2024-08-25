#include<stdio.h>
#include "webserver.h"

#define PORT 10000

void handle_get_todos(request*);
void handle_post_todos(request*);

int main() {
  webserver *ws = webserver_create(PORT);

  webserver_handle_get(ws, "/api/todo", handle_get_todos);
  webserver_handle_post(ws, "/api/todo", handle_post_todos);

  webserver_run(ws);
}

void handle_get_todos(request* req) {
  request_send_text(req, 200, "hello world");
}

void handle_post_todos(request* req) {
  request_send_text(req, 200, "adding todos");
}
