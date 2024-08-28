#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include <cjson/cJSON.h>
#include "webserver.h"

#define PORT 10000
#define DB_NAME "todos.bin"

typedef struct{
  int id;
  char* title;
} Todo;

void handle_get_todos(request*);
void handle_post_todos(request*);
void handle_index(request*);

void todos_add(char* title);
Todo* todos_read(int *count);

int main() {
  webserver *ws = webserver_create(PORT);

  webserver_handle_get(ws, "/", handle_index);
  webserver_handle_get(ws, "/api/todo", handle_get_todos);
  webserver_handle_post(ws, "/api/todo", handle_post_todos);

  webserver_run(ws);
}

void handle_get_todos(request* req) {
  cJSON* json = cJSON_CreateArray();

  int todo_list_count;
  Todo* todo_list = todos_read(&todo_list_count);

  for(int i = 0; i < todo_list_count; i++) {
    cJSON* item = cJSON_CreateObject();
    cJSON_AddNumberToObject(item, "id", todo_list[i].id);
    cJSON_AddStringToObject(item, "title", todo_list[i].title);
    cJSON_AddItemToArray(json, item);
  }

  response_send_json(req, STATUS_OK, json);
  cJSON_Delete(json);
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
  cJSON_Delete(json);
}

void handle_index(request* req) {
  html_template *tmpl = template_create("templates/index.html");
  char todos_string[1000];
  int todo_list_count;
  Todo* todo_list = todos_read(&todo_list_count);

  int length = 0;
  for(int i=0;i<todo_list_count; i++) {
    length += sprintf(todos_string + length, "<li>%s</li>", todo_list[i].title);
  }

  template_add_var(tmpl, "todos", todos_string);
  char* html = malloc(10000);
  template_render(tmpl, html);
  response_send_html(req, STATUS_OK, html);
  free(html);
  template_free(tmpl);
}

Todo* todos_read(int *count) {
  FILE *file = fopen(DB_NAME, "rb");
  if (file == NULL) {
    *count = 0;
    return NULL;
  }

  if(fread(count, sizeof(int), 1, file) != 1) {
    perror("unable to read todo count");
    *count = 0;
    return NULL;
  }

  Todo *todos = (Todo *)malloc(*count * sizeof(Todo));

  for (int i = 0; i < *count; i++) {
    if(fread(&todos[i].id, sizeof(int), 1, file) != 1){
      perror("unable to read todo id");
      break;
    }

    int len;
    if(fread(&len, sizeof(int), 1, file) != 1 ){
      perror("unable to read todo title length");
      break;
    }

    todos[i].title = (char *)malloc(len * sizeof(char));

    if(fread(todos[i].title, sizeof(char), len, file) != (unsigned long)len){
      perror("unable to read todo title");
      break;
    }
  }

  fclose(file);
  return todos;
}

void todos_add(char* title) {
  Todo todo = {
    .title = title
  };
  FILE *file = fopen(DB_NAME, "r+b");
  int count = 0;
  if (file == NULL) {
    file = fopen(DB_NAME, "wb");
    if (file == NULL) {
      perror("Error creating file");
      exit(EXIT_FAILURE);
    }
    fwrite(&count, sizeof(int), 1, file);
  } else {
    if (fread(&count, sizeof(int), 1, file) != 1) {
      perror("Error reading count from file");
      fclose(file);
      exit(EXIT_FAILURE);
    }
  }

  count++;
  fseek(file, 0, SEEK_END);

  fwrite(&todo.id, sizeof(int), 1, file);
  int title_length = strlen(todo.title) + 1;
  fwrite(&title_length, sizeof(int), 1, file);
  fwrite(todo.title, sizeof(char), title_length, file);
  fseek(file, 0, SEEK_SET);
  fwrite(&count, sizeof(int), 1, file);

  fclose(file);
}
