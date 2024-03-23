#include "filesystem.h"
#include <LittleFS.h>
#include "../log/log.h"
#include "mcu_comms.h"

// -----------------------------------------
// -------- Initialize LittleFS ------------
// -----------------------------------------
void initFS() {
  if (!LittleFS.begin(true)) {
    LOG_INFO("An error has occurred while mounting LittleFS");
  }
  LOG_INFO("LittleFS mounted successfully");
}
