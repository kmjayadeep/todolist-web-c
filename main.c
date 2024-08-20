#include<stdio.h>
#include "webserver.h"

#define PORT 10000

int main() {
  webserver *ws = webserver_create(PORT);
  webserver_run(ws);
}
