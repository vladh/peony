#ifndef LOG_H
#define LOG_H

void log_fatal(const char *format, ...);
void log_error(const char *format, ...);
void log_warning(const char *format, ...);
void log_info(const char *format, ...);
void log_newline();

#endif
