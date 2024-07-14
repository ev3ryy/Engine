#ifndef ENGINE_LOGS
#define ENGINE_LOGS

#include <spdlog/spdlog.h>

namespace Engine {
#ifdef NDEBUG

	/* ���� ������ != ������, �� ��������� �� �������� ���� */

#define LOG_INFO
#define LOG_WARNING
#define LOG_ERROR
#define LOG_CRITICAL

#else

	/* ���� ������ == ������, �� ���������� � ���������� ���� */

#define LOG_INFO spdlog::info // ��� ������� ����������
#define LOG_WARNING spdlog::warn // ��� ��������������
#define LOG_ERROR spdlog::error // ��� ������
#define LOG_CRITICAL spdlog::critical // ��� ����������� ������
#define RUNTIME_ERROR std::runtime_error("")

#endif // NDEBUG
}

#endif // ENGINE_LOGS