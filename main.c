#include<stdio.h>
#include<stdlib.h>
#include <cjson/cJSON.h>
#include "webserver.h"

#define PORT 10000

typedef struct{
  int id;
  char *title;
} Todo;

Todo* todo_list;
size_t todo_list_count = 0;
size_t todo_capacity = 100;

void todos_add(char* title);
void handle_get_todos(request*);
void handle_post_todos(request*);

int main() {
  todo_list = malloc(todo_capacity*sizeof(Todo));
  todos_add("go shopping");
  todos_add("buy pickles");

  webserver *ws = webserver_create(PORT);

  webserver_handle_get(ws, "/api/todo", handle_get_todos);
  webserver_handle_post(ws, "/api/todo", handle_post_todos);

  webserver_run(ws);
}

void handle_get_todos(request* req) {
  cJSON* json = cJSON_CreateArray();

  for(size_t i = 0;i < todo_list_count; i++) {
    cJSON* item = cJSON_CreateObject();
    cJSON_AddNumberToObject(item, "id", todo_list[i].id);
    cJSON_AddStringToObject(item, "title", todo_list[i].title);
    cJSON_AddItemToArray(json, item);
  }

  response_send_json(req, STATUS_OK, json);
}

void handle_post_todos(request* req) {
  cJSON *json = cJSON_Parse(req->body);
  cJSON *j_title = cJSON_GetObjectItemCaseSensitive(json, "title");
  if (cJSON_IsString(j_title) && (j_title->valuestring != NULL)) {
    todos_add(j_title->valuestring);
    response_send_text(req, STATUS_CREATED, "");
  }else{
    response_send_text(req, STATUS_BAD_REQUEST, "Wrong input");
  }
}

void todos_add(char* title) {
  if(todo_list_count >= todo_capacity) {
    todo_capacity *= 2;
    todo_list = realloc(todo_list, todo_capacity * sizeof(Todo));
  }

  int index = todo_list_count;
  todo_list[index].title = title;
  todo_list[index].id = rand();
  todo_list_count++;
}
