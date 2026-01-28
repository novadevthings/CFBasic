#include "utils.h"
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

size_t total_memory_limit = 1073741824; /* 1GB default */
size_t memory_used = 0;

void init_memory(size_t limit) {
  total_memory_limit = limit;
  memory_used = 0;
}

void *safe_malloc(size_t size) {
  if (memory_used + size > total_memory_limit) {
    error("OUT OF MEMORY");
    return NULL;
  }
  void *ptr = malloc(size);
  if (!ptr) {
    error("SYSTEM OUT OF MEMORY");
    exit(1);
  }
  memory_used += size;
  return ptr;
}

void *safe_realloc(void *ptr, size_t old_size, size_t new_size) {
  if (memory_used - old_size + new_size > total_memory_limit) {
    error("OUT OF MEMORY");
    return NULL;
  }
  void *new_ptr = realloc(ptr, new_size);
  if (!new_ptr) {
    error("SYSTEM OUT OF MEMORY");
    exit(1);
  }
  memory_used = memory_used - old_size + new_size;
  return new_ptr;
}

void safe_free(void *ptr) {
  if (ptr) {
    free(ptr);
  }
}

size_t get_free_memory(void) { return total_memory_limit - memory_used; }

char *str_duplicate(const char *str) {
  if (!str)
    return NULL;
  size_t len = strlen(str) + 1;
  char *dup = safe_malloc(len);
  if (dup) {
    memcpy(dup, str, len);
  }
  return dup;
}

char *str_upper(const char *str) {
  if (!str)
    return NULL;
  size_t len = strlen(str) + 1;
  char *upper = safe_malloc(len);
  if (!upper)
    return NULL;

  for (size_t i = 0; i < len - 1; i++) {
    upper[i] = toupper((unsigned char)str[i]);
  }
  upper[len - 1] = '\0';
  return upper;
}

int str_compare_nocase(const char *s1, const char *s2) {
  while (*s1 && *s2) {
    int c1 = toupper((unsigned char)*s1);
    int c2 = toupper((unsigned char)*s2);
    if (c1 != c2)
      return c1 - c2;
    s1++;
    s2++;
  }
  return toupper((unsigned char)*s1) - toupper((unsigned char)*s2);
}

void error(const char *format, ...) {
  va_list args;
  va_start(args, format);
  fprintf(stderr, "?");
  vfprintf(stderr, format, args);
  fprintf(stderr, " ERROR\n");
  va_end(args);
}

void warning(const char *format, ...) {
  va_list args;
  va_start(args, format);
  fprintf(stderr, "WARNING: ");
  vfprintf(stderr, format, args);
  fprintf(stderr, "\n");
  va_end(args);
}

void clear_screen(void) {
#ifdef _WIN32
  system("cls");
#else
  system("clear");
#endif
}

char *read_line(const char *prompt) {
  if (prompt) {
    printf("%s", prompt);
    fflush(stdout);
  }

  char *line = NULL;
  size_t len = 0;
  ssize_t read = getline(&line, &len, stdin);

  if (read == -1) {
    free(line);
    return NULL;
  }

  /* Remove trailing newline */
  if (read > 0 && line[read - 1] == '\n') {
    line[read - 1] = '\0';
  }

  return line;
}

size_t parse_memory_size(const char *str) {
  char *endptr;
  double value = strtod(str, &endptr);

  if (value <= 0) {
    return 0;
  }

  size_t multiplier = 1;
  if (*endptr != '\0') {
    switch (toupper((unsigned char)*endptr)) {
    case 'K':
      multiplier = 1024;
      break;
    case 'M':
      multiplier = 1024 * 1024;
      break;
    case 'G':
      multiplier = 1024 * 1024 * 1024;
      break;
    default:
      return 0; /* Invalid suffix */
    }
  }

  return (size_t)(value * multiplier);
}

void format_memory_size(char *buf, size_t size, size_t limit) {
  const char *units[] = {"B", "KB", "MB", "GB"};
  double size_float = (double)size;
  double limit_float = (double)limit;
  int size_unit = 0;
  int limit_unit = 0;

  while (size_float >= 1024 && size_unit < 3) {
    size_float /= 1024;
    size_unit++;
  }

  while (limit_float >= 1024 && limit_unit < 3) {
    limit_float /= 1024;
    limit_unit++;
  }

  sprintf(buf, "%.2f %s Free, %.0f %s Allocated", size_float, units[size_unit],
          limit_float, units[limit_unit]);
}
