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

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

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

template<typename OStream, glm::length_t L, typename T, glm::qualifier Q>
inline OStream& operator<<(OStream& os, const glm::vec<L, T, Q>& vector)
{
	return os << glm::to_string(vector);
}

template<typename OStream, glm::length_t C, glm::length_t R, typename T, glm::qualifier Q>
inline OStream& operator<<(OStream& os, const glm::mat<C, R, T, Q>& matrix)
{
	return os << glm::to_string(matrix);
}

template<typename OStream, typename T, glm::qualifier Q>
inline OStream& operator<<(OStream& os, glm::qua<T, Q> quaternion)
{
	return os << glm::to_string(quaternion);
}

namespace PurrfectEngine {

  // class Logger {
  // public:
  //   static void initialize() {
  //     std::vector<spdlog::sink_ptr> logSinks;
  //     logSinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
  //     logSinks.emplace_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>("PurrfectEngine.log", true));

  //     logSinks[0]->set_pattern("%^[%T] %n: %v%$");
  //     logSinks[1]->set_pattern("[%T] [%l] %n: %v");

  //     sCoreLogger = std::make_shared<spdlog::logger>("PurrfectEngine", begin(logSinks), end(logSinks));
  //     spdlog::register_logger(sCoreLogger);
  //     sCoreLogger->set_level(spdlog::level::trace);
  //     sCoreLogger->flush_on(spdlog::level::trace);

  //     sClntLogger = std::make_shared<spdlog::logger>("Application", begin(logSinks), end(logSinks));
  //     spdlog::register_logger(sClntLogger);
  //     sClntLogger->set_level(spdlog::level::trace);
  //     sClntLogger->flush_on(spdlog::level::trace);
  //   }

  //   static std::shared_ptr<spdlog::logger> &getCoreLogger()   { PURR_CORE_ASSERT(sCoreLogger, "Core logger is NULL! Did you fogor to put \"PurrfectEngine::Logger::initialize();\"?");   return sCoreLogger; }
  //   static std::shared_ptr<spdlog::logger> &getClientLogger() { PURR_ASSERT     (sClntLogger, "Client logger is NULL! Did you fogor to put \"PurrfectEngine::Logger::initialize();\"?"); return sClntLogger; }
  // private:
  //   inline static std::shared_ptr<spdlog::logger> sCoreLogger;
  //   inline static std::shared_ptr<spdlog::logger> sClntLogger;
  // };

}

// // Core log macros
// #define PURR_CORE_TRACE(...)   ::PurrfectEngine::Logger::getCoreLogger()->trace   (__VA_ARGS__);
// #define PURR_CORE_INFO (...)   ::PurrfectEngine::Logger::getCoreLogger()->info    (__VA_ARGS__);
// #define PURR_CORE_WARN (...)   ::PurrfectEngine::Logger::getCoreLogger()->warn    (__VA_ARGS__);
// #define PURR_CORE_ERROR(...)   ::PurrfectEngine::Logger::getCoreLogger()->error   (__VA_ARGS__);
// #define PURR_CORE_FATAL(...) { ::PurrfectEngine::Logger::getCoreLogger()->critical(__VA_ARGS__); exit(EXIT_FAILURE); }
// // Client log macros
// #define PURR_TRACE     (...)   ::PurrfectEngine::Logger::getClientLogger()->trace   (__VA_ARGS__);
// #define PURR_INFO      (...)   ::PurrfectEngine::Logger::getClientLogger()->info    (__VA_ARGS__);
// #define PURR_WARN      (...)   ::PurrfectEngine::Logger::getClientLogger()->warn    (__VA_ARGS__);
// #define PURR_ERROR     (...)   ::PurrfectEngine::Logger::getClientLogger()->error   (__VA_ARGS__);
// #define PURR_FATAL     (...) { ::PurrfectEngine::Logger::getClientLogger()->critical(__VA_ARGS__); exit(EXIT_FAILURE); }

#endif