void log_error(const char *format, ...) {
  va_list vargs;
  fprintf(stderr, "error | ");
  va_start(vargs, format);
  vfprintf(stderr, format, vargs);
  fprintf(stderr, "\n");
  va_end(vargs);
}

void log_warning(const char *format, ...) {
  va_list vargs;
  fprintf(stderr, "warn  | ");
  va_start(vargs, format);
  vfprintf(stderr, format, vargs);
  fprintf(stderr, "\n");
  va_end(vargs);
}

void log_info(const char *format, ...) {
  va_list vargs;
  fprintf(stdout, "info  | ");
  va_start(vargs, format);
  vfprintf(stdout, format, vargs);
  fprintf(stdout, "\n");
  va_end(vargs);
}

void log_newline() {
  fprintf(stdout, "\n");
}
