void log_fatal(const char *format, ...) {
  va_list vargs;
  fprintf(stderr, "fatal | ");
  va_start(vargs, format);
  vfprintf(stderr, format, vargs);
  fprintf(stderr, "\n");
  va_end(vargs);
#ifdef _WIN32
  __debugbreak();
#else
  raise(SIGABRT);
#endif
}


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


void log_aimatrix4x4(aiMatrix4x4 *t) {
  log_info("(%f, %f, %f, %f)", t->a1, t->b1, t->c1, t->d1);
  log_info("(%f, %f, %f, %f)", t->a2, t->b2, t->c2, t->d2);
  log_info("(%f, %f, %f, %f)", t->a3, t->b3, t->c3, t->d3);
  log_info("(%f, %f, %f, %f)", t->a4, t->b4, t->c4, t->d4);
}


void log_mat4(m4 *t) {
  log_info("(%f, %f, %f, %f)", (*t)[0][0], (*t)[1][0], (*t)[2][0], (*t)[3][0]);
  log_info("(%f, %f, %f, %f)", (*t)[0][1], (*t)[1][1], (*t)[2][1], (*t)[3][1]);
  log_info("(%f, %f, %f, %f)", (*t)[0][2], (*t)[1][2], (*t)[2][2], (*t)[3][2]);
  log_info("(%f, %f, %f, %f)", (*t)[0][3], (*t)[1][3], (*t)[2][3], (*t)[3][3]);
}


void log_vec2(v2 *t) {
  log_info("(%f, %f)", (*t)[0], (*t)[1]);
}


void log_vec3(v3 *t) {
  log_info("(%f, %f, %f)", (*t)[0], (*t)[1], (*t)[2]);
}


void log_vec4(v4 *t) {
  log_info("(%f, %f, %f, %f)", (*t)[0], (*t)[1], (*t)[2], (*t)[3]);
}
