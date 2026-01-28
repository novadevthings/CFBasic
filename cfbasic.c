#include "editor.h"
#include "interpreter.h"
#include "lexer.h"
#include "utils.h"
#include <ctype.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static Interpreter *global_interp = NULL;

void handle_sigint(int sig) {
  (void)sig;
  if (global_interp) {
    global_interp->break_requested = true;
  }
}

#define VERSION "1.0"

void print_banner(Interpreter *interp) {
  char mem_buf[256];
  format_memory_size(mem_buf, get_free_memory(), total_memory_limit);
  // Convert memory string to uppercase
  for (int i = 0; mem_buf[i]; i++) {
    mem_buf[i] = toupper((unsigned char)mem_buf[i]);
  }

  char line1[256], line2[256];
  snprintf(line1, sizeof(line1), "**** CFBasic V%s ****", VERSION);
  snprintf(line2, sizeof(line2),
           "A Microsoft BASIC Interpreter for Modern Systems");

  int cols = 80;
  if (interp->editor) {
    cols = interp->editor->cols;
  }

  int pad1 = (cols - (int)strlen(line1)) / 2;
  int pad2 = (cols - (int)strlen(line2)) / 2;
  if (pad1 < 0)
    pad1 = 0;
  if (pad2 < 0)
    pad2 = 0;

  char buf[1024];
  char *p = buf;

  // Line 1 (Centered)
  for (int i = 0; i < pad1; i++)
    *p++ = ' ';
  p += sprintf(p, "%s\n", line1);

  // Line 2 (Centered)
  for (int i = 0; i < pad2; i++)
    *p++ = ' ';
  p += sprintf(p, "%s\n\n", line2);

  // Memory line and READY. (Commodore style)
  p += sprintf(p, " %s\n\nREADY.\n", mem_buf);

  if (interp->editor) {
    editor_print(interp->editor, buf);
  } else {
    printf("%s", buf);
    fflush(stdout);
  }
}

void print_usage(void) {
  printf("Usage: cfbasic [OPTIONS] [filename]\n");
  printf("Options:\n");
  printf("  -M, --MEM <size>    Set memory limit (e.g., 1G, 512M, 2048K)\n");
  printf("  -h, --help          Show this help message\n");
  printf("  -v, --version       Show version information\n");
}

bool is_immediate_command(const char *line) {
  /* Check if line starts with a number */
  while (*line == ' ' || *line == '\t')
    line++;
  return !isdigit(*line);
}

int extract_line_number(const char *line, char **rest) {
  while (*line == ' ' || *line == '\t')
    line++;

  if (!isdigit(*line)) {
    return -1;
  }

  char *endptr;
  int line_num = strtol(line, &endptr, 10);

  if (rest) {
    /* Skip whitespace after line number */
    while (*endptr == ' ' || *endptr == '\t')
      endptr++;
    *rest = endptr;
  }

  return line_num;
}

void execute_immediate_command(Interpreter *interp, const char *line) {
  Lexer lexer;
  lexer_init(&lexer, line);

  Token token = lexer_next_token(&lexer);

  switch (token.type) {
  case TOK_LIST: {
    token_free(&token);
    token = lexer_next_token(&lexer);

    int start = 0;
    int end = -1;

    if (token.type == TOK_NUMBER) {
      start = (int)token.number_value;
      token_free(&token);
      token = lexer_next_token(&lexer);

      if (token.type == TOK_COMMA || token.type == TOK_MINUS) {
        token_free(&token);
        token = lexer_next_token(&lexer);
        if (token.type == TOK_NUMBER) {
          end = (int)token.number_value;
        }
      }
    }

    interpreter_list(interp, start, end);
    token_free(&token);
    break;
  }

  case TOK_RUN:
    token_free(&token);
    interpreter_run(interp);
    break;

  case TOK_NEW:
    token_free(&token);
    interpreter_new(interp);
    break;

  case TOK_LOAD: {
    token_free(&token);
    token = lexer_next_token(&lexer);

    if (token.type == TOK_STRING) {
      interpreter_load(interp, token.text);
    } else {
      error("FILENAME REQUIRED");
    }
    token_free(&token);
    break;
  }

  case TOK_SAVE: {
    token_free(&token);
    token = lexer_next_token(&lexer);

    if (token.type == TOK_STRING) {
      interpreter_save(interp, token.text);
    } else {
      error("FILENAME REQUIRED");
    }
    token_free(&token);
    break;
  }

  case TOK_EXIT:
    interp->exit_requested = true;
    token_free(&token);
    break;

  default:
    /* Execute as direct mode statement */
    interpreter_execute_line(interp, line);
    token_free(&token);
    break;
  }

  lexer_free(&lexer);
}

void repl(Interpreter *interp) {
  Editor ed;
  editor_init(&ed);
  interp->editor = &ed;

  global_interp = interp;
  signal(SIGINT, handle_sigint);

  editor_enable_raw_mode();
  editor_clear_screen(&ed);
  print_banner(interp);

  while (!interp->exit_requested) {
    char *line = editor_read_line(&ed);
    if (!line) {
      if (interp->break_requested) {
        editor_print(&ed, "\n? BREAK\nREADY.\n");
        interp->break_requested = false;
        continue;
      }
      break;
    }

    /* Skip empty lines */
    if (strlen(line) == 0) {
      free(line);
      continue;
    }

    /* Check if it's a numbered line (program line) */
    char *rest;
    int line_num = extract_line_number(line, &rest);

    if (line_num >= 0) {
      /* Add or delete program line */
      program_add_line(interp, line_num, rest);
    } else {
      /* Execute immediate command */
      execute_immediate_command(interp, line);
      if (!interp->exit_requested) {
        editor_print(&ed, "\nREADY.\n");
      }
    }

    free(line);
    if (interp->exit_requested)
      break;
  }

  signal(SIGINT, SIG_DFL);
  global_interp = NULL;

  editor_disable_raw_mode();
  editor_free(&ed);
}

int main(int argc, char *argv[]) {
  size_t memory_limit = 1073741824; /* 1GB default */
  const char *filename = NULL;

  /* Parse command line arguments */
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-M") == 0 || strcmp(argv[i], "--MEM") == 0) {
      if (i + 1 < argc) {
        memory_limit = parse_memory_size(argv[++i]);
        if (memory_limit == 0) {
          fprintf(stderr, "Invalid memory size: %s\n", argv[i]);
          return 1;
        }
      } else {
        fprintf(stderr, "Missing memory size argument\n");
        print_usage();
        return 1;
      }
    } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
      print_usage();
      return 0;
    } else if (strcmp(argv[i], "-v") == 0 ||
               strcmp(argv[i], "--version") == 0) {
      printf("CFBASIC V%s\n", VERSION);
      return 0;
    } else if (argv[i][0] != '-') {
      filename = argv[i];
    } else {
      fprintf(stderr, "Unknown option: %s\n", argv[i]);
      print_usage();
      return 1;
    }
  }

  /* Initialize memory system */
  init_memory(memory_limit);

  /* Initialize interpreter */
  Interpreter interp;
  interpreter_init(&interp);

  /* Load file if specified */
  if (filename) {
    if (interpreter_load(&interp, filename)) {
      interpreter_run(&interp);
    }
  } else {
    /* Start REPL */
    repl(&interp);
  }

  /* Cleanup */
  interpreter_free(&interp);

  return 0;
}
