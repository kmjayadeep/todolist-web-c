#ifndef SERVER_H
#define SERVER_H

#include <cjson/cJSON.h>



#define METHOD_GET "GET"
#define METHOD_POST "POST"
#define METHOD_DELETE "DELETE"
#define METHOD_PATCH "PATCH"
#define METHOD_PUT "PUT"
#define METHOD_HEAD "HEAD"

#define STATUS_OK "200 OK"
#define STATUS_CREATED "201 Created"
#define STATUS_BAD_REQUEST "400 Bad Request"
#define STATUS_UNAUTHORIZED "401 Unauthorized"
#define STATUS_NOT_FOUND "404 Not Found"
#define STATUS_INTERNAL_SERVER_ERROR "500 Internal Server Error"

typedef struct _webserver webserver;
typedef struct _http_header http_header;

typedef struct _request {
  char* method;
  char* path;
  http_header** headers;
  int headers_count;
  char* body;
  int client_fd;
  webserver* webserver;
} request;

typedef void (*request_handler_func)(request *req);

webserver* webserver_create(int port);
int webserver_run(webserver *ws);
void webserver_handle_get(webserver* ws, char *path, request_handler_func handler);
void webserver_handle_post(webserver* ws, char *path, request_handler_func handler);
void webserver_handle_delete(webserver* ws, char *path, request_handler_func handler);
void webserver_handle_patch(webserver* ws, char *path, request_handler_func handler);
void webserver_handle_put(webserver* ws, char *path, request_handler_func handler);
void webserver_handle_head(webserver* ws, char *path, request_handler_func handler);

void response_send_text(request *req, char* status, char *text);
void response_send_html(request *req, char* status, char *html);
void response_send_json(request *req, char* status, cJSON *json);

#endif // !SERVER_H
