#ifndef WEBSERVER_FS_H
#define WEBSERVER_FS_H
#include "profiling_phases.h"


typedef struct NamedProfile_t {
  Profile profile;
  char *name;
} NamedProfile;

void initFS();
void listFS();

void fsSaveProfile(NamedProfile &profile);
uint8_t fsProfilesCount();
void fsGetProfiles(NamedProfile profiles[]);
void fsDeleteProfile(const char *name);

#endif
