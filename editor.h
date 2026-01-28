#ifndef EDITOR_H
#define EDITOR_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
  int rows;
  int cols;
  int cursor_row;
  int cursor_col;
  char *buffer; // Screen buffer
} Editor;

void editor_init(Editor *ed);
void editor_free(Editor *ed);
void editor_clear_screen(Editor *ed);
void editor_refresh(Editor *ed);
char *editor_read_line(Editor *ed);
void editor_enable_raw_mode(void);
void editor_disable_raw_mode(void);

// For printing to the screen editor
void editor_print(Editor *ed, const char *str);
void editor_scroll(Editor *ed);
void editor_plot(Editor *ed, int x, int y, char c);
void editor_set_background_color(Editor *ed, int color);
void editor_poke_char(Editor *ed, int addr, uint8_t val);

#endif /* EDITOR_H */
