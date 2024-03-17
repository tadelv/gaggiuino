#include "api_profiles.h"
#include "AsyncTCP.h"
#include <ArduinoJson.h>
// #include "../utils/server_utils.h"
#include "../../wifi/wifi_setup.h"
#include "../../log/log.h"
#include "../../filesystem/filesystem.h"

void handleGetProfilesList(AsyncWebServerRequest *request);
void handlePostUpdateProfile(AsyncWebServerRequest * request, JsonVariant &body);
void handleDeleteProfile(AsyncWebServerRequest *request, JsonVariant &body);
void handlePutProfileSelect(AsyncWebServerRequest *request, JsonVariant &body);

void setupProfilesApi(AsyncWebServer &server)
{
  server.on("/api/profiles/list", HTTP_GET, handleGetProfilesList);
}

const char *curveToString(TransitionCurve curve)
{
  switch (curve)
  {
  case TransitionCurve::EASE_IN_OUT:
    return "EASE_IN_OUT";
  case TransitionCurve::EASE_IN:
    return "EASE_IN";
  case TransitionCurve::EASE_OUT:
    return "EASE_OUT";
  case TransitionCurve::LINEAR:
    return "LINEAR";
  case TransitionCurve::INSTANT:
    return "INSTANT";
  default:
    return "UNKNOWN_TRANSITION_CURVE";
  }
}

void handleGetProfilesList(AsyncWebServerRequest *request) {
  LOG_INFO("List profiles request");

  AsyncResponseStream *response = request->beginResponseStream("application/json");
  DynamicJsonDocument json(2048);
  JsonArray profilesJson = json.to<JsonArray>();

  for (auto &profile : fsGetProfiles())
  {
    JsonObject jsonProfile = profilesJson.createNestedObject();
    jsonProfile["name"] = profile.name;
    JsonObject profileDetails = jsonProfile.createNestedObject("profile");
    JsonObject stopConditions = profileDetails.createNestedObject("stopConditions");
    stopConditions["time"] = profile.profile.globalStopConditions.time;
    stopConditions["weight"] = profile.profile.globalStopConditions.weight;
    stopConditions["waterPumped"] = profile.profile.globalStopConditions.waterPumped;
    JsonArray jsonPhases = profileDetails.createNestedArray("phases");
    for (Phase p : profile.profile.phases) {
      JsonObject jsonPhase = jsonPhases.createNestedObject();
      jsonPhase["type"] = p.type == PHASE_TYPE::PHASE_TYPE_FLOW ? "FLOW" : "PRESSURE";
      jsonPhase["restriction"] = p.getRestriction();
      JsonObject target = jsonPhase.createNestedObject("target");
      target["start"] = p.target.start;
      target["end"] = p.target.end;
      target["curve"] = curveToString(p.target.curve);
      target["time"] = p.target.time;
      JsonObject stopConditions = jsonPhase.createNestedObject("stopConditions");
      stopConditions["time"] = p.stopConditions.time;
      stopConditions["pressureAbove"] = p.stopConditions.pressureAbove;
      stopConditions["pressureBelow"] = p.stopConditions.pressureBelow;
      stopConditions["flowAbove"] = p.stopConditions.flowAbove;
      stopConditions["flowBelow"] = p.stopConditions.flowBelow;
      stopConditions["weight"] = p.stopConditions.weight;
      stopConditions["waterPumpedInPhase"] = p.stopConditions.waterPumpedInPhase;
    }
  }

  serializeJson(profilesJson, *response);
  request->send(response);
}