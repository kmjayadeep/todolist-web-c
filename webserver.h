#ifndef SERVER_H
#define SERVER_H



#define METHOD_GET "GET"
#define METHOD_POST "POST"
#define METHOD_DELETE "DELETE"
#define METHOD_PATCH "PATCH"
#define METHOD_PUT "PUT"
#define METHOD_HEAD "HEAD"

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

void request_send_text(request *req, int status, char *response);

#endif // !SERVER_H
