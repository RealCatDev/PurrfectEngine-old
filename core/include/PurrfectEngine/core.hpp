#ifndef PURRENGINE_CORE_HPP_
#define PURRENGINE_CORE_HPP_

#include <iostream>

namespace PurrfectEngine {

  #define PURR_ASSERT(func) do { \
    if (!(func)) { \
      std::cerr << "Assertation error: " << #func << std::endl; \
      exit(1); \
    } \
  } while (0);

}

#endif