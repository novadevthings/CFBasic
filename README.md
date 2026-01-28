# CFBASIC - Commodore BASIC Interpreter

A recreation of Commodore 128 BASIC written in C for modern systems (Linux, macOS, Windows). Features a full-screen Kernal-style editor, 64KB virtual RAM emulation, and hardware-mapped PEEK/POKE.

## Features

- **C128 BASIC 7.0 Dialect**: Supports structured programming with `DO...LOOP`, `WHILE...WEND`, `REPEAT...UNTIL`, and `IF...THEN...ELSE`.
- **Case-Insensitive Syntax**: Commands and variables are not case-sensitive.
- **Kernal-style Screen Editor**:
  - Full-screen cursor movement via arrow keys.
  - **Logical Line Picking**: Move the cursor to any line of text and press **ENTER** to execute it immediately.
  - Integrated virtual screen buffer for classic terminal interaction.
- **C64 Memory Compatibility***:
  - Emulated **64KB RAM** system.
  - Full **`PEEK`** and **`POKE`** support for all 65,536 addresses.
  - **Screen RAM Mapping**: Writing to `1024-2023` directly updates the terminal display.
  - **Hardware Traps**: VIC-II register emulation for colors (`53280/53281`).
- **Graphics Support***:
  - **`PLOT X, Y`** and **`DRAW X, Y`** for character-based line drawing.
  - Coordinates are scaled from standard C64 resolution (320x200) to your terminal window.
- **Cross-Platform**: Runs on Linux, macOS, and Windows.
<br><br>
***Unfinished Feature**

## Building

### Linux/macOS
```bash
make
```

### Windows (MinGW)
```bash
make windows
```

## Usage

### Interactive Mode
```bash
./basic
```

### Run a Program
```bash
./basic program.bas
```

### Control Keys
- **Ctrl+C**: Break a running program and return to the `READY.` prompt.
- **EXIT**: Type `EXIT` in immediate mode to quit the interpreter.

## BASIC Commands

### Immediate Commands
- `LIST` - Display program
- `RUN` - Execute program
- `NEW` - Clear program
- `LOAD "filename"` - Load program from file
- `SAVE "filename"` - Save program to file
- `EXIT` - Quit the interpreter

### Program Statements
- `PRINT` / `?` - Output text/values
- `INPUT` - Read user input
- `LET` - Variable assignment
- `GOTO` - Jump to line number
- `GOSUB` / `RETURN` - Subroutine calls
- `IF...THEN...ELSE` - Conditional execution
- `FOR...NEXT` - Loops
- `DO...LOOP` - Structured loop (C128)
- `WHILE...WEND` - While loop (C128)
- `REPEAT...UNTIL` - Repeat-until loop (C128)
- `POKE addr, val` - Write to emulated RAM
- `PLOT x, y` - Set drawing position
- `DRAW x, y` - Draw line to coordinate
- `REM` - Comments
- `END` / `STOP` - End program
- `DIM` - Declare arrays

### Built-in Functions
- `PEEK(addr)` - Read from emulated RAM
- `ABS(x)` - Absolute value
- `INT(x)` - Integer part
- `RND(x)` - Random number
- `SIN(x)`, `COS(x)`, `TAN(x)` - Trigonometric functions
- `SQR(x)` - Square root
- `LEN(s$)` - String length
- `LEFT$(s$,n)`, `RIGHT$(s$,n)`, `MID$(s$,n,m)` - String functions
- `STR$(x)` - Number to string
- `VAL(s$)` - String to number
- `CHR$(x)` / `ASC(s$)` - Character/ASCII conversion

## Examples

### Speed Test (Benchmark)
```basic
10 FOR I = 1 TO 10000
20 PRINT ".";
30 NEXT I
40 PRINT "DONE."
```

### Classic Maze Generator
```basic
10 PRINT CHR$(205.5 + RND(1));
20 GOTO 10
```

## Installation

```bash
sudo make install
```

This installs `basic` to `/usr/local/bin`.

## License

GNU General Public License v3.0 - See [LICENSE](LICENSE) file for details.

## Author

This program is developed by Cole Bohte @ River Games.
