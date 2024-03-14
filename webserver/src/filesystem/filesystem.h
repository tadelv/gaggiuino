#ifndef WEBSERVER_FS_H
#define WEBSERVER_FS_H
#include "profiling_phases.h"


typedef struct NamedProfile_t {
  Profile profile;
  char name[39];
} NamedProfile;

void initFS();
void listFS();

void fsSaveProfile(NamedProfile &profile);
uint8_t fsProfilesCount();
std::vector<NamedProfile> fsGetProfiles();
void fsDeleteProfile(const char *name);

#endif
