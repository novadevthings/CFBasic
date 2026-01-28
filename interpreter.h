#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "lexer.h"
#include <math.h>
#include <stdbool.h>
#include <stdint.h>

#include "editor.h"

/* Forward declarations */
typedef struct Variable Variable;
typedef struct ProgramLine ProgramLine;
typedef struct Interpreter Interpreter;

/* Variable types */
typedef enum {
  VAR_NUMBER,
  VAR_STRING,
  VAR_ARRAY_NUMBER,
  VAR_ARRAY_STRING
} VarType;

/* Variable structure */
typedef struct Variable {
  char *name;
  VarType type;
  union {
    double number;
    char *string;
    struct {
      void *data;
      int *dimensions;
      int dim_count;
    } array;
  } value;
  struct Variable *next;
} Variable;

/* Program line structure */
typedef struct ProgramLine {
  int line_number;
  char *text;
  struct ProgramLine *next;
} ProgramLine;

/* Stack frame for GOSUB/RETURN */
typedef struct StackFrame {
  int return_line;
  struct StackFrame *next;
} StackFrame;

/* FOR loop context */
typedef struct ForLoop {
  char *var_name;
  double end_value;
  double step_value;
  int loop_line;
  struct ForLoop *next;
} ForLoop;

/* Interpreter state */
typedef struct Interpreter {
  ProgramLine *program;
  ProgramLine *current_line;
  Variable *variables;
  StackFrame *call_stack;
  ForLoop *for_stack;
  Editor *editor; // New: link to screen editor
  bool running;
  bool break_requested;
  bool exit_requested;
  bool error_occurred;
  double graphics_x;  // Current graphics X position
  double graphics_y;  // Current graphics Y position
  uint8_t ram[65536]; // C64-style 64KB RAM
  char *error_message;
} Interpreter;

/* Interpreter functions */
void interpreter_init(Interpreter *interp);
void interpreter_free(Interpreter *interp);
void interpreter_run(Interpreter *interp);
void interpreter_execute_line(Interpreter *interp, const char *line);
void interpreter_list(Interpreter *interp, int start, int end);
void interpreter_new(Interpreter *interp);
bool interpreter_load(Interpreter *interp, const char *filename);
bool interpreter_save(Interpreter *interp, const char *filename);

/* Program management */
void program_add_line(Interpreter *interp, int line_num, const char *text);
void program_delete_line(Interpreter *interp, int line_num);
ProgramLine *program_find_line(Interpreter *interp, int line_num);
void program_clear(Interpreter *interp);

/* Variable management */
Variable *var_get(Interpreter *interp, const char *name);
Variable *var_set_number(Interpreter *interp, const char *name, double value);
Variable *var_set_string(Interpreter *interp, const char *name,
                         const char *value);
void var_clear_all(Interpreter *interp);

/* Stack management */
void stack_push(Interpreter *interp, int return_line);
int stack_pop(Interpreter *interp);

/* FOR loop management */
void for_push(Interpreter *interp, const char *var_name, double end,
              double step, int line);
ForLoop *for_find(Interpreter *interp, const char *var_name);
void for_pop(Interpreter *interp);

#endif /* INTERPRETER_H */
