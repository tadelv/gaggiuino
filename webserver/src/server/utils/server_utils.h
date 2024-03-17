#ifndef SERVER_UTILS_H
#define SERVER_UTILS_H

#include "ESPAsyncWebServer.h"
#include "AsyncJson.h"

AsyncCallbackJsonWebHandler* jsonHandler(const char* uri, WebRequestMethodComposite method, ArJsonRequestHandlerFunction onRequest);


#endif
