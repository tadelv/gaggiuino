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

void listDir(fs::FS &fs, const char *dirname, uint8_t levels)
{
  LOG_INFO("Listing directory: %s", dirname);

  File root = fs.open(dirname);
  if (!root)
  {
    LOG_INFO("- failed to open directory");
    return;
  }
  if (!root.isDirectory())
  {
    LOG_INFO(" - not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file)
  {
    if (file.isDirectory())
    {
      LOG_INFO("  DIR : ");

#ifdef CONFIG_LITTLEFS_FOR_IDF_3_2
      Serial.println(file.name());
#else
      LOG_INFO(file.name());
      time_t t = file.getLastWrite();
      struct tm *tmstruct = localtime(&t);
      LOG_INFO("  LAST WRITE: %d-%02d-%02d %02d:%02d:%02d\n", (tmstruct->tm_year) + 1900, (tmstruct->tm_mon) + 1, tmstruct->tm_mday, tmstruct->tm_hour, tmstruct->tm_min, tmstruct->tm_sec);
#endif

      if (levels)
      {
        listDir(fs, file.path(), levels - 1);
      }
    }
    else
    {
      LOG_INFO("  FILE: ");
      LOG_INFO(file.name());
      LOG_INFO("  SIZE: ");

#ifdef CONFIG_LITTLEFS_FOR_IDF_3_2
      Serial.println(file.size());
#else
      LOG_INFO("%u", file.size());
      time_t t = file.getLastWrite();
      struct tm *tmstruct = localtime(&t);
      LOG_INFO("  LAST WRITE: %d-%02d-%02d %02d:%02d:%02d\n", (tmstruct->tm_year) + 1900, (tmstruct->tm_mon) + 1, tmstruct->tm_mday, tmstruct->tm_hour, tmstruct->tm_min, tmstruct->tm_sec);
#endif
    }
    file = root.openNextFile();
  }
}

void createDir(fs::FS &fs, const char *path)
{
  Serial.printf("Creating Dir: %s\n", path);
  if (fs.mkdir(path))
  {
    Serial.println("Dir created");
  }
  else
  {
    Serial.println("mkdir failed");
  }
}

void removeDir(fs::FS &fs, const char *path)
{
  Serial.printf("Removing Dir: %s\n", path);
  if (fs.rmdir(path))
  {
    Serial.println("Dir removed");
  }
  else
  {
    Serial.println("rmdir failed");
  }
}

void readFile(fs::FS &fs, const char *path)
{
  LOG_INFO("Reading file: %s\r\n", path);

  File file = fs.open(path);
  if (!file || file.isDirectory())
  {
    LOG_INFO("- failed to open file for reading");
    return;
  }

  LOG_INFO("- read from file:");
  while (file.available())
  {
    LOG_INFO("%c", file.read());
  }
  file.close();
}

void writeFile(fs::FS &fs, const char *path, const char *message)
{
  Serial.printf("Writing file: %s\r\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file)
  {
    Serial.println("- failed to open file for writing");
    return;
  }
  if (file.print(message))
  {
    Serial.println("- file written");
  }
  else
  {
    Serial.println("- write failed");
  }
  file.close();
}

void appendFile(fs::FS &fs, const char *path, const char *message)
{
  Serial.printf("Appending to file: %s\r\n", path);

  File file = fs.open(path, FILE_APPEND);
  if (!file)
  {
    Serial.println("- failed to open file for appending");
    return;
  }
  if (file.print(message))
  {
    Serial.println("- message appended");
  }
  else
  {
    Serial.println("- append failed");
  }
  file.close();
}

void renameFile(fs::FS &fs, const char *path1, const char *path2)
{
  Serial.printf("Renaming file %s to %s\r\n", path1, path2);
  if (fs.rename(path1, path2))
  {
    Serial.println("- file renamed");
  }
  else
  {
    Serial.println("- rename failed");
  }
}

void deleteFile(fs::FS &fs, const char *path)
{
  Serial.printf("Deleting file: %s\r\n", path);
  if (fs.remove(path))
  {
    Serial.println("- file deleted");
  }
  else
  {
    Serial.println("- delete failed");
  }
}

void listFS()
{
  size_t total = LittleFS.totalBytes();
  size_t used = LittleFS.usedBytes();
  LOG_INFO("total size: %fkB\n", total / (float)1024);
  LOG_INFO("used: %fkB\n", used / (float)1024);
  LOG_INFO("disk usage: %f%%\n", used / (float)total);
  File f = LittleFS.open("/");
  String filename = f.getNextFileName();
  while (filename != "") {
    LOG_INFO("found: %s", filename.c_str());
    filename = f.getNextFileName();
  }
}

void writeFileBinary(fs::FS &fs, const char *path, const uint8_t *data, const size_t len) 
{
  LOG_DEBUG("writing to: %s - size: %u", path, len);
  File file = fs.open(path, FILE_WRITE);
  if (!file)
  {
    LOG_INFO("%s - failed to open file for writing", path);
    return;
  }
  if (file.write(data, len))
  {
    LOG_DEBUG("- file written");
  }
  else
  {
    LOG_INFO("%s - write failed", path);
  }
  file.close();
}

std::vector<uint8_t> readFileBinary(fs::FS &fs, const char *path)
{
  LOG_DEBUG("opening %s", path);
  File file = fs.open(path, FILE_READ);
  if (!file || file.size() == 0) {
    LOG_INFO("Could not open: %s", path);
    return std::vector<uint8_t>();
  }

  size_t size = file.size();
  LOG_DEBUG("allocating: %u", size);

  uint8_t *data = (uint8_t *)malloc(size);
  if (data == NULL) {
    LOG_INFO("Could not allocate memory");
    return std::vector<uint8_t>();
  }
  file.read(data, size);

  std::vector<uint8_t> buffer(data, data+file.size());
  free(data);
  return buffer;
}

void fsSaveProfile(NamedProfile &profile)
{
  char filename[50] = "/profiles/";
  strncat(filename, profile.name, 39);
  LOG_DEBUG("saving %s", profile.name);
  ProfileSerializer serializer;
  std::vector<uint8_t> data = serializer.serializeProfile(profile.profile);
  LOG_DEBUG("serialized size: %u", data.size());
  writeFileBinary(LittleFS, filename, data.data(), data.size());
}

uint8_t fsProfilesCount()
{
  File profilesDir = LittleFS.open("/profiles");
  File profile = profilesDir.openNextFile();
  uint8_t count = 0;

  while (profile)
  {
    count++;
    profile = profilesDir.openNextFile();
  }
  return count;
}

std::vector<NamedProfile> fsGetProfiles() 
{
  uint8_t count = fsProfilesCount();
  std::vector<NamedProfile> profiles = std::vector<NamedProfile>();
  if (count < 1) {
    return profiles;
  }
  File profilesDir = LittleFS.open("/profiles");
  File profile = profilesDir.openNextFile();

  ProfileSerializer serializer;

  while (profile)
  {
    std::vector<uint8_t> dataVector = readFileBinary(LittleFS, profile.path());
    if (dataVector.size() == 0) {
      profile = profilesDir.openNextFile();
      continue;
    }
    Profile profileData;
    serializer.deserializeProfile(dataVector, profileData);

    NamedProfile profileEntry;
    strncpy(profileEntry.name, profile.name(), 38);
    profileEntry.profile = profileData;
    profiles.push_back(profileEntry);
    profile = profilesDir.openNextFile();
  }
  return profiles;
}

void fsDeleteProfile(const char *name) 
{
  char filename[50] = "/profiles/";
  strncat(filename, name, 39);
  deleteFile(LittleFS, filename);
}
