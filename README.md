# Compiler Design Lab Using LEX and YACC

An educational compiler for a small C-like language, implemented with Flex,
Bison, and C. It reads source from standard input and prints three-address code
(TAC), optimization results, and accumulator-style target code.

## Supported language

- Integer literals and identifiers
- Assignments ending in `;`
- Arithmetic: `+`, `-`, `*`, `/`, and parentheses
- Comparisons: `<`, `>`, `<=`, `>=`, `==`, `!=`
- `while`, `if`, `if-else`, blocks, and nested statements
- Whitespace and `//` comments

This is a language subset, so omit C declarations, `main()`, and `printf()`.
The output is educational assembly-like text; it is not an executable program.
The optimizer demonstrates constant folding and basic loop-invariant code motion,
and is not a production optimizer.

## Install, clone, build, and run (Ubuntu / Debian)

Run these commands in an Ubuntu terminal, including Ubuntu under WSL:

```bash
sudo apt update
sudo apt install -y git build-essential flex bison
cd ~
git clone https://github.com/Akshith123-collab/compiler-design-lex-yacc.git
cd compiler-design-lex-yacc
make
make demo
```

Clone into your Ubuntu home directory (`cd ~`), which is writable by your user.
Avoid cloning from `/mnt/c/Windows/System32`.

`make demo` displays `input.c` and compiles it. The sample contains a `while`
loop, constant arithmetic (`16 * 5`), and loop-invariant expressions. Expect
`Syntax is CORRECT.`, TAC before and after optimization, and `TARGET CODE`.
The compiler translates the loop; it does not execute it.

To compile a different file:

```bash
./compiler < path/to/program.c
```

For interactive input, run `./compiler`, enter statements, then press **Ctrl+D**
on an empty line to finish.

## Windows with WSL Ubuntu

On a new Windows system, install Ubuntu from an administrator PowerShell:

```powershell
wsl --install -d Ubuntu
```

Restart if prompted, open Ubuntu, complete its initial username/password setup,
then run the Ubuntu installation and clone commands above.

For the existing local folder on this computer, open Ubuntu and run:

```bash
cd /mnt/c/Users/akshith/Desktop/compiler_lab
make demo
```

## Pull updates on an already cloned system

From inside your clone, with local changes committed or stashed:

```bash
git pull --ff-only origin main
make clean
make
make demo
```

## Build manually without Make

```bash
bison -d parser.y
flex lexer.l
gcc -Wall -Wextra -O2 -o compiler parser.tab.c lex.yy.c backend.c
./compiler < input.c
```

## Files

| File | Purpose |
| --- | --- |
| `lexer.l` | Token rules and lexical error reporting |
| `parser.y` | Grammar, TAC actions, and program entry point |
| `backend.c`, `backend.h` | TAC storage, optimization, and target-code output |
| `input.c` | Sample source program |
| `Makefile` | Build, demo, and cleanup commands |

Generated lexer/parser files and the Linux executable are ignored by Git and
are rebuilt locally. Use `make clean` to remove generated build files.
