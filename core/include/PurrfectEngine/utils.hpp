#ifndef PURRENGINE_UTILS_HPP_
#define PURRENGINE_UTILS_HPP_

#include "PurrfectEngine/core.hpp"
#include "PurrfectEngine/application.hpp"

namespace PurrfectEngine {

  namespace IO {

    static std::vector<char> ReadFile (const char *filepath) {
      std::ifstream file(filepath, std::ios::ate | std::ios::binary);

      if (!file.is_open()) {
        fprintf(stderr, "Failed to open file \"%s\"!", filepath);
        exit(1);
      }

      size_t fileSize = (size_t) file.tellg();
      std::vector<char> buffer(fileSize);
      file.seekg(0);
      file.read(buffer.data(), fileSize);
      file.close();

      return buffer;
    }

  }

  static const char *Asset(const char *path) {
    const char *assetDir = application::getInstance()->getAssetDir();
    size_t assetDirLen = strlen(assetDir), fullDirLen = 0;
    char *fullDir = (char*)malloc(sizeof(char) * (fullDirLen = (strlen(path) + assetDirLen + 1)));
    for (size_t i = 0; i < strlen(path) + assetDirLen; ++i) {
      if (i < assetDirLen) fullDir[i] = assetDir[i];
      else fullDir[i] = path[i - assetDirLen];
    }
    fullDir[fullDirLen-1] = '\0';
    return (const char*)fullDir;
  }

}

#endif