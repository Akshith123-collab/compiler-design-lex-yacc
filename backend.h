#ifndef BACKEND_H
#define BACKEND_H

void emit(const char *format, ...);
char *newtemp(void);
char *newlabel(void);
void emit_label(const char *label);
void print_tac(void);
void optimize_code(void);
void generate_target_code(void);

#endif
