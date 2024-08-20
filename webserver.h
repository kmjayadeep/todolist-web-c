#ifndef SERVER_H
#define SERVER_H

typedef struct _webserver webserver;

webserver* webserver_create(int port);
int webserver_run(webserver *ws);

#endif // !SERVER_H
