#ifndef SERVER_H
#define SERVER_H

typedef struct _webserver webserver;
typedef struct _http_header http_header;
typedef struct _request request;

webserver* webserver_create(int port);
int webserver_run(webserver *ws);

#endif // !SERVER_H
