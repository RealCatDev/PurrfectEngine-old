#ifndef PURRENGINE_CORE_HPP_
#define PURRENGINE_CORE_HPP_

#include <iostream>
#include <optional>
#include <vector>
#include <array>
#include <set>
#include <filesystem>
#include <cstring>
#include <fstream>

#include <random>
#include <iomanip>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// #pragma warning(push, 0)
// #include <spdlog/spdlog.h>
// #include <spdlog/fmt/ostr.h>
// #pragma warning(pop)

#ifdef PURR_DEBUG
  #if defined(PURR_PLAT_WIN)
    #define PURR_DEBUGBREAK() __debugbreak()
  #elif defined(PURR_PLAT_LINUX)
    #include <signal.h>
    #define PURR_DEBUGBREAK() raise(SIGTRAP)
  #else
    #error "Platform doesn't support debugbreak yet!"
  #endif
  #define PURR_ENABLE_ASSERTS
#else
  #define PURR_DEBUGBREAK()
#endif

#define PURR_EXPAND_MACRO(x) x
#define PURR_STRINGIFY_MACRO(x) #x

#ifdef PURR_ENABLE_ASSERTS
  #define PURR_INTERNAL_ASSERT_IMPL(type, check, msg, ...) { if(!(check)) { fprintf(stderr, msg, __VA_ARGS__); PURR_DEBUGBREAK(); } }
  #define PURR_INTERNAL_ASSERT_WITH_MSG(type, check, ...) PURR_INTERNAL_ASSERT_IMPL(type, check, "Assertion failed: %s", __VA_ARGS__)
  #define PURR_INTERNAL_ASSERT_NO_MSG(type, check) PURR_INTERNAL_ASSERT_IMPL(type, check, "Assertion '%s' failed at %s:%d", PURR_STRINGIFY_MACRO(check), std::filesystem::path(__FILE__).filename().string().c_str(), __LINE__)

  #define PURR_INTERNAL_ASSERT_GET_MACRO_NAME(arg1, arg2, macro, ...) macro
  #define PURR_INTERNAL_ASSERT_GET_MACRO(...) PURR_EXPAND_MACRO( PURR_INTERNAL_ASSERT_GET_MACRO_NAME(__VA_ARGS__, PURR_INTERNAL_ASSERT_WITH_MSG, PURR_INTERNAL_ASSERT_NO_MSG) )

  #define PURR_ASSERT(...) PURR_EXPAND_MACRO( PURR_INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(_, __VA_ARGS__) )
  #define PURR_CORE_ASSERT(...) PURR_EXPAND_MACRO( PURR_INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(_CORE_, __VA_ARGS__) )
#else
  #define PURR_ASSERT(...)
  #define PURR_CORE_ASSERT(...)
#endif

// template<typename OStream, glm::length_t L, typename T, glm::qualifier Q>
// inline OStream& operator<<(OStream& os, const glm::vec<L, T, Q>& vector)
// {
// 	return os << glm::to_string(vector);
// }

// template<typename OStream, glm::length_t C, glm::length_t R, typename T, glm::qualifier Q>
// inline OStream& operator<<(OStream& os, const glm::mat<C, R, T, Q>& matrix)
// {
// 	return os << glm::to_string(matrix);
// }

// template<typename OStream, typename T, glm::qualifier Q>
// inline OStream& operator<<(OStream& os, glm::qua<T, Q> quaternion)
// {
// 	return os << glm::to_string(quaternion);
// }

namespace PurrfectEngine {

  class UUID {
  public:
    UUID(): 
      mGenerator(mRandomDevice()), mValue(mDistr(mGenerator))
    {}
    UUID(const UUID &) = delete;

    uint32_t operator()() {
      return mValue;
    }

    bool operator ==(UUID other) {
      return mValue == other.mValue;
    }
  private:
    std::random_device mRandomDevice;
    std::mt19937 mGenerator;
    std::uniform_int_distribution<uint32_t> mDistr;
    
    uint32_t mValue;
  };

}

#endif