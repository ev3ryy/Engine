#ifndef ENGINE_LOGS
#define ENGINE_LOGS

#include <spdlog/spdlog.h>

namespace Engine {
#ifdef NDEBUG

	/* Если сборка != дебагу, то отключаем от директив логи */

#define LOG_INFO
#define LOG_WARNING
#define LOG_ERROR
#define LOG_CRITICAL

#else

	/* Если сборка == дебагу, то подключаем к директивам логи */

#define LOG_INFO spdlog::info // Лог штатной информации
#define LOG_WARNING spdlog::warn // Лог предупреждения
#define LOG_ERROR spdlog::error // Лог ошибки
#define LOG_CRITICAL spdlog::critical // Лог критической ошибки
#define RUNTIME_ERROR std::runtime_error("")

#endif // NDEBUG
}

#endif // ENGINE_LOGS