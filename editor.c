#include "editor.h"
#include "utils.h"
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

static struct termios orig_termios;

void editor_enable_raw_mode(void) {
  if (tcgetattr(STDIN_FILENO, &orig_termios) == -1)
    return;

  struct termios raw = orig_termios;
  raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
  raw.c_oflag &= ~(OPOST);
  raw.c_cflag |= (CS8);
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
  raw.c_cc[VMIN] = 1;
  raw.c_cc[VTIME] = 0;

  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void editor_disable_raw_mode(void) {
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

static void get_window_size(int *rows, int *cols) {
  struct winsize ws;
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
    *rows = 24;
    *cols = 80;
  } else {
    *rows = ws.ws_row;
    *cols = ws.ws_col;
  }
}

void editor_init(Editor *ed) {
  get_window_size(&ed->rows, &ed->cols);
  ed->cursor_row = 0;
  ed->cursor_col = 0;
  ed->buffer = safe_malloc(ed->rows * ed->cols);
  memset(ed->buffer, ' ', ed->rows * ed->cols);
}

void editor_free(Editor *ed) {
  if (ed->buffer) {
    safe_free(ed->buffer);
    ed->buffer = NULL;
  }
}

void editor_clear_screen(Editor *ed) {
  memset(ed->buffer, ' ', ed->rows * ed->cols);
  ed->cursor_row = 0;
  ed->cursor_col = 0;
  printf("\033[2J\033[H");
  fflush(stdout);
}

void editor_scroll(Editor *ed) {
  memmove(ed->buffer, ed->buffer + ed->cols, (ed->rows - 1) * ed->cols);
  memset(ed->buffer + (ed->rows - 1) * ed->cols, ' ', ed->cols);
  ed->cursor_row--;
  if (ed->cursor_row < 0)
    ed->cursor_row = 0;

  // Visually scroll
  printf("\033[S");
}

void editor_refresh(Editor *ed) {
  // Basic refresh: clear screen and redraw from buffer
  // For performance, we could do incremental, but let's keep it simple
  printf("\033[H");
  for (int r = 0; r < ed->rows; r++) {
    fwrite(ed->buffer + r * ed->cols, 1, ed->cols, stdout);
    if (r < ed->rows - 1)
      printf("\r\n");
  }
  printf("\033[%d;%dH", ed->cursor_row + 1, ed->cursor_col + 1);
  fflush(stdout);
}

void editor_print(Editor *ed, const char *str) {
  while (*str) {
    if (*str == '\n') {
      ed->cursor_col = 0;
      ed->cursor_row++;
      printf("\r\n");
    } else if (*str == '\r') {
      ed->cursor_col = 0;
      printf("\r");
    } else if (*str == '\t') {
      int old_col = ed->cursor_col;
      ed->cursor_col = (ed->cursor_col + 8) & ~7;
      for (int i = 0; i < ed->cursor_col - old_col; i++)
        putchar(' ');
    } else {
      if (ed->cursor_row >= ed->rows) {
        editor_scroll(ed);
      }
      ed->buffer[ed->cursor_row * ed->cols + ed->cursor_col] = *str;
      putchar(*str);
      ed->cursor_col++;
    }

    if (ed->cursor_col >= ed->cols) {
      ed->cursor_col = 0;
      ed->cursor_row++;
      printf("\r\n");
    }
    if (ed->cursor_row >= ed->rows) {
      editor_scroll(ed);
    }
    str++;
  }
  // Update physical cursor
  printf("\033[%d;%dH", ed->cursor_row + 1, ed->cursor_col + 1);
  fflush(stdout);
}

char *editor_read_line(Editor *ed) {
  char c;
  while (1) {
    ssize_t nread = read(STDIN_FILENO, &c, 1);
    if (nread == -1) {
      if (errno == EINTR)
        return NULL;
      continue;
    }
    if (nread == 0)
      return NULL;

    if (c == '\r' || c == '\n') {
      // Pick the current line from logical screen
      int r = ed->cursor_row;
      int start = r * ed->cols;
      int end = start + ed->cols - 1;

      // Trim leading/trailing spaces for the "picked" line
      while (start <= end && ed->buffer[start] == ' ')
        start++;
      while (end >= start && ed->buffer[end] == ' ')
        end--;

      int len = end - start + 1;
      char *line = NULL;
      if (len > 0) {
        line = safe_malloc(len + 1);
        memcpy(line, ed->buffer + start, len);
        line[len] = '\0';
      } else {
        line = str_duplicate("");
      }

      // Move cursor to next line
      printf("\r\n");
      ed->cursor_row++;
      ed->cursor_col = 0;
      if (ed->cursor_row >= ed->rows) {
        editor_scroll(ed);
      }
      return line;
    } else if (c == 127 || c == 8) { // Backspace
      if (ed->cursor_col > 0) {
        ed->cursor_col--;
        ed->buffer[ed->cursor_row * ed->cols + ed->cursor_col] = ' ';
        printf("\b \b");
      }
    } else if (c == '\033') { // Escape sequence
      char seq[3];
      if (read(STDIN_FILENO, &seq[0], 1) != 1)
        continue;
      if (read(STDIN_FILENO, &seq[1], 1) != 1)
        continue;

      if (seq[0] == '[') {
        switch (seq[1]) {
        case 'A': // Up
          if (ed->cursor_row > 0)
            ed->cursor_row--;
          break;
        case 'B': // Down
          if (ed->cursor_row < ed->rows - 1)
            ed->cursor_row++;
          break;
        case 'C': // Right
          if (ed->cursor_col < ed->cols - 1)
            ed->cursor_col++;
          break;
        case 'D': // Left
          if (ed->cursor_col > 0)
            ed->cursor_col--;
          break;
        }
      }
      printf("\033[%d;%dH", ed->cursor_row + 1, ed->cursor_col + 1);
    } else if (iscntrl(c)) {
      // Ignore other control codes
    } else {
      if (ed->cursor_row >= ed->rows) {
        editor_scroll(ed);
      }
      ed->buffer[ed->cursor_row * ed->cols + ed->cursor_col] = c;
      putchar(c);
      ed->cursor_col++;
      if (ed->cursor_col >= ed->cols) {
        ed->cursor_col = 0;
        ed->cursor_row++;
      }
    }
    fflush(stdout);
  }
  return NULL;
}

void editor_plot(Editor *ed, int x, int y, char c) {
  if (x < 0 || x >= ed->cols || y < 0 || y >= ed->rows)
    return;
  ed->buffer[y * ed->cols + x] = c;
  printf("\033[%d;%dH%c", y + 1, x + 1, c);
  fflush(stdout);
}

void editor_set_background_color(Editor *ed, int color) {
  (void)ed;
  // Map C64 colors (0-15) to ANSI colors (simplified)
  int ansi_bg = 40; // Default black
  switch (color & 15) {
  case 0:
    ansi_bg = 40;
    break; // Black
  case 1:
    ansi_bg = 107;
    break; // White
  case 2:
    ansi_bg = 41;
    break; // Red
  case 3:
    ansi_bg = 106;
    break; // Cyan
  case 4:
    ansi_bg = 45;
    break; // Purple
  case 5:
    ansi_bg = 42;
    break; // Green
  case 6:
    ansi_bg = 44;
    break; // Blue
  case 7:
    ansi_bg = 103;
    break; // Yellow
  case 8:
    ansi_bg = 43;
    break; // Orange
  case 9:
    ansi_bg = 101;
    break; // Brown
  case 10:
    ansi_bg = 101;
    break; // Lt Red
  case 11:
    ansi_bg = 100;
    break; // Grey 1
  case 12:
    ansi_bg = 100;
    break; // Grey 2
  case 13:
    ansi_bg = 102;
    break; // Lt Green
  case 14:
    ansi_bg = 104;
    break; // Lt Blue
  case 15:
    ansi_bg = 100;
    break; // Grey 3
  }
  printf("\033[%dm", ansi_bg);
  fflush(stdout);
}

void editor_poke_char(Editor *ed, int addr, uint8_t val) {
  int offset = addr - 1024;
  if (offset < 0 || offset >= 1000)
    return;

  int r = offset / 40;
  int c = offset % 40;

  // Simple CBM core code to ASCII conversion
  char ch = ' ';
  if (val >= 1 && val <= 26)
    ch = val + 64; // A-Z
  else if (val >= 27 && val <= 31)
    ch = val + 64; // [ / ] ^ _
  else if (val >= 32 && val <= 63)
    ch = val; // Space-?
  else if (val >= 64 && val <= 95)
    ch = val + 32; // a-z
  else if (val >= 96 && val <= 127)
    ch = val; // Graphics
  else
    ch = '?'; // Fallback

  // Map to terminal grid (might need scaling)
  int tr = r * ed->rows / 25;
  int tc = c * ed->cols / 40;

  editor_plot(ed, tc, tr, ch);
}
