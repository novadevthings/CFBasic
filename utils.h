#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>
#include <stdint.h>

/* Memory allocation tracking */
extern size_t total_memory_limit;
extern size_t memory_used;

/* Memory functions */
void *safe_malloc(size_t size);
void *safe_realloc(void *ptr, size_t old_size, size_t new_size);
void safe_free(void *ptr);
void init_memory(size_t limit);
size_t get_free_memory(void);

/* String utilities */
char *str_duplicate(const char *str);
char *str_upper(const char *str);
int str_compare_nocase(const char *s1, const char *s2);

/* Error handling */
void error(const char *format, ...);
void warning(const char *format, ...);

/* Platform-specific utilities */
void clear_screen(void);
char *read_line(const char *prompt);

/* Memory size parsing (for -M flag) */
size_t parse_memory_size(const char *str);
void format_memory_size(char *buf, size_t size, size_t limit);

#endif /* UTILS_H */
