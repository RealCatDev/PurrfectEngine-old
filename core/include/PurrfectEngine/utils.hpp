#ifndef PURRENGINE_UTILS_HPP_
#define PURRENGINE_UTILS_HPP_

#include "PurrfectEngine/core.hpp"

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

}

#endif