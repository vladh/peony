void logs::fatal(const char *format, ...) {
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


void logs::error(const char *format, ...) {
  va_list vargs;
  fprintf(stderr, "error | ");
  va_start(vargs, format);
  vfprintf(stderr, format, vargs);
  fprintf(stderr, "\n");
  va_end(vargs);
}


void logs::warning(const char *format, ...) {
  va_list vargs;
  fprintf(stderr, "warn  | ");
  va_start(vargs, format);
  vfprintf(stderr, format, vargs);
  fprintf(stderr, "\n");
  va_end(vargs);
}


void logs::info(const char *format, ...) {
  va_list vargs;
  fprintf(stdout, "info  | ");
  va_start(vargs, format);
  vfprintf(stdout, format, vargs);
  fprintf(stdout, "\n");
  va_end(vargs);
}


void logs::print_newline() {
  fprintf(stdout, "\n");
}


void logs::print_aimatrix4x4(aiMatrix4x4 *t) {
  logs::info("(%f, %f, %f, %f)", t->a1, t->b1, t->c1, t->d1);
  logs::info("(%f, %f, %f, %f)", t->a2, t->b2, t->c2, t->d2);
  logs::info("(%f, %f, %f, %f)", t->a3, t->b3, t->c3, t->d3);
  logs::info("(%f, %f, %f, %f)", t->a4, t->b4, t->c4, t->d4);
}


void logs::print_m4(m4 *t) {
  logs::info("(%f, %f, %f, %f)", (*t)[0][0], (*t)[1][0], (*t)[2][0], (*t)[3][0]);
  logs::info("(%f, %f, %f, %f)", (*t)[0][1], (*t)[1][1], (*t)[2][1], (*t)[3][1]);
  logs::info("(%f, %f, %f, %f)", (*t)[0][2], (*t)[1][2], (*t)[2][2], (*t)[3][2]);
  logs::info("(%f, %f, %f, %f)", (*t)[0][3], (*t)[1][3], (*t)[2][3], (*t)[3][3]);
}


void logs::print_v2(v2 *t) {
  logs::info("(%f, %f)", (*t)[0], (*t)[1]);
}


void logs::print_v3(v3 *t) {
  logs::info("(%f, %f, %f)", (*t)[0], (*t)[1], (*t)[2]);
}


void logs::print_v4(v4 *t) {
  logs::info("(%f, %f, %f, %f)", (*t)[0], (*t)[1], (*t)[2], (*t)[3]);
}


void logs::console(const char *format, ...) {
  char text[GUI_MAX_CONSOLE_LINE_LENGTH];
  va_list vargs;
  va_start(vargs, format);
  vsnprintf(text, sizeof(text), format, vargs);
  va_end(vargs);

  // Fill array in back-to-front.
  if (g_console->idx_log_start == 0) {
    g_console->idx_log_start = GUI_MAX_N_CONSOLE_LINES - 1;
  } else {
    g_console->idx_log_start--;
  }
  if (g_console->idx_log_start == g_console->idx_log_end) {
    if (g_console->idx_log_end == 0) {
      g_console->idx_log_end = GUI_MAX_N_CONSOLE_LINES - 1;
    } else {
      g_console->idx_log_end--;
    }
  }
  strcpy(g_console->log[g_console->idx_log_start], text);
}
