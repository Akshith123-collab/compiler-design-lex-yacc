#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include "backend.h"

#define MAX_CODE 1000
#define LINE_SIZE 256

static char code[MAX_CODE][LINE_SIZE];
static int code_count = 0;
static int temp_count = 0;
static int label_count = 0;

void emit(const char *format, ...)
{
    va_list args;

    if (code_count >= MAX_CODE) {
        fprintf(stderr, "Error: TAC buffer is full.\n");
        exit(EXIT_FAILURE);
    }

    va_start(args, format);
    vsnprintf(code[code_count], LINE_SIZE, format, args);
    va_end(args);

    code_count++;
}

char *newtemp(void)
{
    char buffer[32];

    snprintf(buffer, sizeof(buffer), "t%d", temp_count++);

    return strdup(buffer);
}

char *newlabel(void)
{
    char buffer[32];

    snprintf(buffer, sizeof(buffer), "L%d", label_count++);

    return strdup(buffer);
}

void emit_label(const char *label)
{
    emit("%s:", label);
}

void print_tac(void)
{
    int i;

    printf("\nTHREE-ADDRESS CODE\n");
    printf("------------------\n");

    for (i = 0; i < code_count; i++)
        printf("%3d: %s\n", i, code[i]);
}

/* Check whether a string represents an integer constant. */
static int is_number(const char *s, long *value)
{
    char *end;

    if (s == NULL || *s == '\0')
        return 0;

    *value = strtol(s, &end, 10);

    return *end == '\0' && end != s;
}

/*
 * Constant folding:
 *     t0 = 16 * 5
 * becomes:
 *     t0 = 80
 */
static int fold_constants(void)
{
    int i;
    int changed = 0;

    for (i = 0; i < code_count; i++) {
        char lhs[64], a[64], op[8], b[64];
        long x, y, result;

        if (sscanf(code[i], " %63s = %63s %7s %63s",
                   lhs, a, op, b) != 4)
            continue;

        if (!is_number(a, &x) || !is_number(b, &y))
            continue;

        if (strcmp(op, "+") == 0)
            result = x + y;
        else if (strcmp(op, "-") == 0)
            result = x - y;
        else if (strcmp(op, "*") == 0)
            result = x * y;
        else if (strcmp(op, "/") == 0 && y != 0)
            result = x / y;
        else
            continue;

        snprintf(code[i], LINE_SIZE, "%s = %ld", lhs, result);
        changed++;
    }

    return changed;
}

/*
 * Basic loop-invariant code motion.
 *
 * Within a loop, hoist assignments to temporary variables when
 * their operands are not modified in the loop.
 *
 * The pass deliberately does not hoist assignments to ordinary
 * program variables such as r, s, or m.
 */
static int loop_code_motion(void)
{
    int i, j, k;
    int moved = 0;

    for (i = 0; i < code_count; i++) {
        char label[64];
        char *colon = strchr(code[i], ':');

        if (colon == NULL)
            continue;

        if (sscanf(code[i], " %63[^:]:", label) != 1)
            continue;

        /*
         * Find a backward jump to this label.
         * This identifies a simple loop.
         */
        for (j = i + 1; j < code_count; j++) {
            char target[64];

            if (sscanf(code[j], " goto %63s", target) == 1 &&
                strcmp(target, label) == 0)
                break;
        }

        if (j == code_count)
            continue;

        /*
         * Find temporary assignments whose operands are not
         * modified in the loop.
         */
        for (k = i + 1; k < j; k++) {
            char lhs[64], a[64], op[8], b[64];
            char saved[LINE_SIZE];
            int p, invariant = 1;

            if (sscanf(code[k], " %63s = %63s %7s %63s",
                       lhs, a, op, b) != 4)
                continue;

            if (lhs[0] != 't' || !isdigit((unsigned char)lhs[1]))
                continue;

            /* Do not hoist an expression that depends on itself. */
            if (strcmp(lhs, a) == 0 || strcmp(lhs, b) == 0)
                continue;

            /*
             * Check whether either operand is assigned anywhere
             * in the loop. Also reject potentially unsafe cases.
             */
            for (p = i + 1; p <= j; p++) {
                char assigned[64];

                if (sscanf(code[p], " %63s =", assigned) == 1 &&
                    (strcmp(assigned, a) == 0 ||
                     strcmp(assigned, b) == 0)) {
                    invariant = 0;
                    break;
                }

                if (strstr(code[p], "ifFalse") != NULL)
                    continue;
            }

            if (!invariant)
                continue;

            strcpy(saved, code[k]);

            /* Remove the original instruction. */
            for (p = k; p < code_count - 1; p++)
                strcpy(code[p], code[p + 1]);

            code_count--;

            /* Insert the instruction before the loop label. */
            for (p = code_count; p > i; p--)
                strcpy(code[p], code[p - 1]);

            strcpy(code[i], saved);
            code_count++;
            moved++;

            /*
             * The code positions changed, so restart the scan
             * rather than continuing with stale positions.
             */
            return moved + loop_code_motion();
        }
    }

    return moved;
}

void optimize_code(void)
{
    int folded;
    int moved;

    printf("\nOPTIMIZATION\n");
    printf("------------\n");

    folded = fold_constants();
    printf("Constant-folding replacements: %d\n", folded);

    moved = loop_code_motion();
    printf("Loop-invariant instructions moved: %d\n", moved);

    print_tac();
}

/*
 * A simple accumulator-based target-code generator.
 *
 * This is educational assembly-like output, not machine code for
 * a particular processor.
 */
void generate_target_code(void)
{
    int i;

    printf("\nTARGET CODE\n");
    printf("-----------\n");

    for (i = 0; i < code_count; i++) {
        char lhs[64], a[64], op[8], b[64];
        char target[64], relation[8];

        if (strchr(code[i], ':') != NULL) {
            printf("%s\n", code[i]);
            continue;
        }

        if (sscanf(code[i], " goto %63s", target) == 1) {
            printf("JMP %s\n", target);
            continue;
        }

        if (sscanf(code[i], " ifFalse %63s %7s %63s goto %63s",
                   a, relation, b, target) == 4) {
            printf("LOAD %s\n", a);
            printf("CMP %s, %s\n", relation, b);
            printf("JFALSE %s\n", target);
            continue;
        }

        if (sscanf(code[i], " %63s = %63s %7s %63s",
                   lhs, a, op, b) == 4) {
            printf("LOAD %s\n", a);

            if (strcmp(op, "+") == 0)
                printf("ADD %s\n", b);
            else if (strcmp(op, "-") == 0)
                printf("SUB %s\n", b);
            else if (strcmp(op, "*") == 0)
                printf("MUL %s\n", b);
            else if (strcmp(op, "/") == 0)
                printf("DIV %s\n", b);
            else
                printf("; Unsupported operator %s\n", op);

            printf("STORE %s\n", lhs);
            continue;
        }

        if (sscanf(code[i], " %63s = %63s", lhs, a) == 2) {
            printf("LOAD %s\n", a);
            printf("STORE %s\n", lhs);
            continue;
        }

        printf("; %s\n", code[i]);
    }
}
