#include "api_profiles.h"
#include "AsyncTCP.h"
#include <ArduinoJson.h>
// #include "../utils/server_utils.h"
#include "../../wifi/wifi_setup.h"
#include "../../log/log.h"

void handleGetProfilesList(AsyncWebServerRequest *request);
void handlePostUpdateProfile(AsyncWebServerRequest * request, JsonVariant &body);
void handleDeleteProfile(AsyncWebServerRequest *request, JsonVariant &body);
void handlePutProfileSelect(AsyncWebServerRequest *request, JsonVariant &body);

void setupProfilesApi(AsyncWebServer &server)
{
  server.on("/api/profiles/list", HTTP_GET, handleGetProfilesList);
}

void handleGetProfilesList(AsyncWebServerRequest *request) {
  LOG_INFO("List profiles request");
}