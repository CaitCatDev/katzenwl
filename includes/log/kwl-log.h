#pragma once

#include <stdint.h>

enum kwl_log_levels {
	KWL_LOG_NONE,
	KWL_LOG_INFO,
	KWL_LOG_DEBUG,
	KWL_LOG_WARN,
	KWL_LOG_ERROR,
	KWL_LOG_FATAL,
};

void kwl_log_set_level(enum kwl_log_levels level);
int kwl_log(enum kwl_log_levels level, const uint32_t line,
		const char *file, const char *fmt, ...);
int kwl_log_printf(enum kwl_log_levels level, const char *fmt, ...);

#define kwl_log_info(...) kwl_log(KWL_LOG_INFO, __LINE__, __FILE__, __VA_ARGS__)
#define kwl_log_debug(...) kwl_log(KWL_LOG_DEBUG, __LINE__, __FILE__, __VA_ARGS__)
#define kwl_log_warn(...) kwl_log(KWL_LOG_WARN, __LINE__, __FILE__, __VA_ARGS__)
#define kwl_log_error(...) kwl_log(KWL_LOG_ERROR, __LINE__, __FILE__, __VA_ARGS__)
#define kwl_log_fatal(...) kwl_log(KWL_LOG_FATAL, __LINE__, __FILE__, __VA_ARGS__)
