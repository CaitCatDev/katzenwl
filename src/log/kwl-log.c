#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>

#include <stdlib.h>

#include <kwl/log/logger.h>

/*Private functions*/
static enum kwl_log_levels log_level;

static const char *kwl_log_level_str(enum kwl_log_levels level) {
	switch (level) {
		case KWL_LOG_INFO:
			return "INFO";
		case KWL_LOG_WARN:
			return "WARN";
		case KWL_LOG_DEBUG:
			return "DEBUG";
		case KWL_LOG_ERROR:
			return "ERROR";
		case KWL_LOG_FATAL:
			return "FATAL";
		case KWL_LOG_NONE:
		default:
			return "Unknown";
	}
}

static const char *kwl_log_level_color(enum kwl_log_levels level) {
	switch (level) {
		case KWL_LOG_INFO:
			return "\x1b[1;36m";
		case KWL_LOG_WARN:
			return "\x1b[1;33m";
		case KWL_LOG_DEBUG:
			return "\x1b[1;32m";
		case KWL_LOG_ERROR:
			return "\x1b[1;31m";
		case KWL_LOG_FATAL:
			return "\x1b[1;41m";
		case KWL_LOG_NONE:
		default:
			return "\x1b[1;34m";
	}
}

/*Exposed to Linker*/
void kwl_log_set_level(enum kwl_log_levels level) {
	log_level = level;
}

__attribute__((__format__(printf, 4, 0)))
int kwl_log(enum kwl_log_levels level, const uint32_t line, 
		const char *file, const char *fmt, ...) {
	int length;
	va_list args;

	if(level < log_level) return 0;

	printf("%s%s\x1b[0m \x1b[1;35m%s(%u):\x1b[0m \x1b[32m\x1b[0m ", kwl_log_level_color(level), kwl_log_level_str(level), file, line);

	va_start(args, fmt);
	length = vprintf(fmt, args);
	va_end(args);

	return length;
}


__attribute__((__format__(printf, 2, 0)))
int kwl_log_printf(enum kwl_log_levels level, const char *fmt, ...) {
	int length;
	va_list args;
	
	if(level < log_level) return 0;

	va_start(args, fmt);
	length = vprintf(fmt, args);
	va_end(args);

	return length;}

