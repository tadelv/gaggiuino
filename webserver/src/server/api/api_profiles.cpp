#include "api_profiles.h"
#include "AsyncTCP.h"
#include <ArduinoJson.h>
#include "../utils/server_utils.h"
#include "../../wifi/wifi_setup.h"
#include "../../log/log.h"
#include "../../filesystem/filesystem.h"

void handleWsSetDefaultProfile(NamedProfile profile);

void handleGetProfilesList(AsyncWebServerRequest *request);
void handlePostUpdateProfile(AsyncWebServerRequest *request, JsonVariant &body);
void handleDeleteProfile(AsyncWebServerRequest *request, JsonVariant &body);
void handlePostProfileSelect(AsyncWebServerRequest *request, JsonVariant &body);
void handlePostProfileSave(AsyncWebServerRequest *request, JsonVariant &body);

void setupProfilesApi(AsyncWebServer &server)
{
  server.on("/api/profiles/list", HTTP_GET, handleGetProfilesList);
  server.addHandler(jsonHandler("/api/profiles/setDefault", HTTP_POST, handlePostProfileSelect));
  server.addHandler(jsonHandler("/api/profiles/save", HTTP_POST, handlePostProfileSave));
  server.on(
      "/api/profiles",
      HTTP_DELETE,
      [](AsyncWebServerRequest *request)
      {
        String url = request->url();
        LOG_INFO("url: %s", url.c_str());
        url.replace("/api/profiles/", "");
        const char *profileName =  url.c_str();
        LOG_INFO("prf name: %s", profileName);
        LOG_INFO("%s", profileName);
        fsDeleteProfile(profileName);
        request->send(200);
      });
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

TransitionCurve stringToCurve(const char *str)
{
  if (strcmp(str, "EASE_IN_OUT") == 0)
  {
    return TransitionCurve::EASE_IN_OUT;
  }
  else if (strcmp(str, "EASE_IN") == 0)
  {
    return TransitionCurve::EASE_IN;
  }
  else if (strcmp(str, "EASE_OUT") == 0)
  {
    return TransitionCurve::EASE_OUT;
  }
  else if (strcmp(str, "LINEAR") == 0)
  {
    return TransitionCurve::LINEAR;
  }
  else if (strcmp(str, "INSTANT") == 0)
  {
    return TransitionCurve::INSTANT;
  }
  else
  {
    return TransitionCurve::INSTANT;
  }
}

void handleGetProfilesList(AsyncWebServerRequest *request)
{
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
    for (Phase p : profile.profile.phases)
    {
      JsonObject jsonPhase = jsonPhases.createNestedObject();
      jsonPhase["type"] = p.type == PHASE_TYPE::PHASE_TYPE_FLOW ? "FLOW" : "PRESSURE";
      const float restriction = p.getRestriction();
      if (restriction > 0)
      {
        jsonPhase["restriction"] = restriction;
      }
      JsonObject target = jsonPhase.createNestedObject("target");
      target["start"] = p.target.start;
      target["end"] = p.target.end;
      target["curve"] = curveToString(p.target.curve);
      target["time"] = p.target.time;
      JsonObject stopConditions = jsonPhase.createNestedObject("stopConditions");
      if (p.stopConditions.time > 0)
      {
        stopConditions["time"] = p.stopConditions.time;
      }
      if (p.stopConditions.pressureAbove > 0)
      {
        stopConditions["pressureAbove"] = p.stopConditions.pressureAbove;
      }
      if (p.stopConditions.pressureBelow > 0)
      {
        stopConditions["pressureBelow"] = p.stopConditions.pressureBelow;
      }
      if (p.stopConditions.flowAbove > 0)
      {
        stopConditions["flowAbove"] = p.stopConditions.flowAbove;
      }
      if (p.stopConditions.flowBelow > 0)
      {
        stopConditions["flowBelow"] = p.stopConditions.flowBelow;
      }
      if (p.stopConditions.weight > 0)
      {
        stopConditions["weight"] = p.stopConditions.weight;
      }
      if (p.stopConditions.waterPumpedInPhase > 0)
      {
        stopConditions["waterPumpedInPhase"] = p.stopConditions.waterPumpedInPhase;
      }
    }
  }

  serializeJson(profilesJson, *response);
  request->send(response);
}

Profile deserializeProfileJSON(JsonVariant body)
{
  Profile newDefaultProfile;

  JsonArray jsonPhases = body["phases"];
  for (JsonVariant jsonPhase : jsonPhases)
  {
    Phase phase;
    JsonObject jsonPhaseObj = jsonPhase.as<JsonObject>();
    phase.type = jsonPhaseObj["type"].as<String>() == "FLOW" ? PHASE_TYPE::PHASE_TYPE_FLOW : PHASE_TYPE::PHASE_TYPE_PRESSURE;
    phase.restriction = jsonPhaseObj["restriction"] | -1; // May need to adjust based on the actual data type
    JsonObject target = jsonPhaseObj["target"];
    phase.target.start = target["start"] | -1;
    phase.target.end = target["end"] | -1;
    phase.target.curve = stringToCurve(target["curve"].as<String>().c_str());
    phase.target.time = target["time"] | 5000; // FIXME: what's the right default value
    JsonObject stopConditions = jsonPhaseObj["stopConditions"];
    phase.stopConditions.time = stopConditions["time"] | -1;                        // May need to adjust based on the actual data type
    phase.stopConditions.weight = stopConditions["weight"] | -1;                  // May need to adjust based on the actual data type
    phase.stopConditions.waterPumpedInPhase = stopConditions["waterPumped"] | -1; // May need to adjust based on the actual data type
    // Deserialize other fields if needed
    newDefaultProfile.phases.push_back(phase);
  }

  JsonObject stopConditions = body["stopConditions"];
  newDefaultProfile.globalStopConditions.time = stopConditions["time"] | -1;
  newDefaultProfile.globalStopConditions.weight = stopConditions["weight"] | -1;
  newDefaultProfile.globalStopConditions.waterPumped = stopConditions["waterPumped"] | -1;

  return newDefaultProfile;
}

void handlePostProfileSelect(AsyncWebServerRequest *request, JsonVariant &body)
{

  String profileName = body["name"].as<String>();
  Profile profile = deserializeProfileJSON(body["profile"]);

  NamedProfile profileToSave;
  strcpy(profileToSave.name, profileName.c_str());
  profileToSave.profile = profile;

  LOG_DEBUG("received set default request for: %s", profileToSave.name);

  AsyncResponseStream *response = request->beginResponseStream("application/json");
  DynamicJsonDocument json(2048);
  JsonArray profilesJson = json.to<JsonArray>();
  serializeJson(profilesJson, *response);
  request->send(response);
  handleWsSetDefaultProfile(profileToSave);
}

void handlePostProfileSave(AsyncWebServerRequest *request, JsonVariant &body)
{
  String profileName = body["name"].as<String>();
  Profile profile = deserializeProfileJSON(body["profile"]);

  NamedProfile profileToSave;
  strcpy(profileToSave.name, profileName.c_str());
  profileToSave.profile = profile;
  LOG_INFO("saving: %s", profileToSave.name);
  fsSaveProfile(profileToSave);

  AsyncResponseStream *response = request->beginResponseStream("application/json");
  DynamicJsonDocument json(2048);
  JsonArray profilesJson = json.to<JsonArray>();
  serializeJson(profilesJson, *response);
  request->send(response);
}