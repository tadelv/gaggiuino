#include "filesystem.h"
#include <LittleFS.h>
#include "../log/log.h"

// -----------------------------------------
// --------- Initialize SPIFFS -------------
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
  String filename;
  do {
    filename = f.getNextFileName();
    LOG_INFO("found: %s", filename.c_str());
  } while (filename != "");

  // if (LittleFS.mkdir("/profiles")) {
  //   LOG_INFO("created profiles dir\n");
  // }

  // File newF = LittleFS.open("/profiles/italian", FILE_WRITE, true);
  // uint8_t buf[9] = "krompir\0";
  // newF.write(
  //   buf, 9
  // );

  File file2 = LittleFS.open("/profiles/italian");
  char newBuf[12];
  file2.readBytes(newBuf, file2.available());
  LOG_INFO("read file: %s", newBuf);
  listDir(LittleFS, "/", 2);
  // readFile(LittleFS, "/profiles/italian");
}

void writeFileBinary(fs::FS &fs, const char *path, const uint8_t *data, const uint8_t len) 
{
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

void fsSaveProfile(NamedProfile &profile)
{
  char filename[50] = "/profiles/";
  strncat(filename, profile.name, 39);
  size_t structSize = sizeof(NamedProfile);

  // Allocate memory for the serialized data
  uint8_t *serializedData = (uint8_t *)malloc(structSize);
  if (serializedData == NULL)
  {
    LOG_INFO("Memory allocation failed.");
    return;
  }

  // // Copy the struct to the serialized data buffer
  memcpy(serializedData, &profile.profile, structSize);
  writeFileBinary(LittleFS, filename, serializedData, structSize);
}

uint8_t fsProfilesCount()
{

}

void fsGetProfiles(NamedProfile profiles[]) 
{

}

void fsDeleteProfile(const char *name) 
{

}
