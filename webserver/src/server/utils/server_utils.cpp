#include "server_utils.h"

AsyncCallbackJsonWebHandler *jsonHandler(const char *uri, WebRequestMethodComposite method, ArJsonRequestHandlerFunction onRequest)
{
  AsyncCallbackJsonWebHandler *handler = new AsyncCallbackJsonWebHandler(String(uri), onRequest);
  handler->setMethod(method);
  return handler;
}