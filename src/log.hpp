#ifndef LOG_H
#define LOG_H

void log_fatal(const char *format, ...);
void log_error(const char *format, ...);
void log_warning(const char *format, ...);
void log_info(const char *format, ...);
void log_newline();
void log_aimatrix4x4(aiMatrix4x4* matrix);
void log_mat4(glm::mat4 *t);
void log_vec4(glm::vec4 *t);

#endif
