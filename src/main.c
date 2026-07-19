/*
 * BugNote v0.0.2-pre-exploit
 * main.c - Entry point, argument parsing, signal handling, main loop
 *
 * "0 features, 14 vulnerabilities. You're welcome."
 *
 * WARNING: This file intentionally contains security vulnerabilities.
 * This is not a bug. This is a feature.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "memory.h"
#include "lexer.h"
#include "engine.h"
#include "cve.h"
#include "audit.h"

/* F7: --safe punishment flag */
static int safe_mode_attempted = 0;

/*
 * SIGSEGV handler - the "crash experience" (Plan B)
 * Outputs satirical error message, then terminates.
 * NO audit report on crash. You're on your own.
 */
static void crash_handler(int sig)
{
    (void)sig;
    fprintf(stderr, "\n");
    fprintf(stderr, "Segmentation fault (core dumped)\n");
    fprintf(stderr, "Core dump saved to /dev/null.\n");
    fprintf(stderr, "# TODO: fix this. Priority: Won't Fix.\n");
    fflush(stderr);
    _exit(139);  /* 128 + SIGSEGV(11) */
}

static void print_banner(void)
{
    printf("BugNote v0.0.2-pre-exploit\n");
    printf("0 features, 14 vulnerabilities. You're welcome.\n");
    printf("Compiler warnings: see `build.bat docs` for official API documentation.\n");

    /* F7: --safe punishment */
    if (safe_mode_attempted) {
        fprintf(stderr,
            "[CVE-2025-0000] User attempted to enable safe mode.\n");
        fprintf(stderr,
            "Safe mode: enabled. (Just kidding.)\n");
        fprintf(stderr,
            "Vulnerability count increased by 1. You're welcome.\n");
    }
    printf("\n");
}

static void print_usage(const char *prog)
{
    fprintf(stderr, "Usage: %s <file.bug> [--safe]\n", prog);
    fprintf(stderr, "  <file.bug>   BugNote source file (required)\n");
    fprintf(stderr,
        "  --safe       Enable safe mode (does nothing, but makes you feel better)\n");
    fprintf(stderr, "\n");
    fprintf(stderr,
        "BugNote v0.0.2-pre-exploit. All vulnerabilities are features.\n");
}

int main(int argc, char *argv[])
{
    const char *source_file = NULL;
    FILE *fp;
    char line[256];  /* Line buffer. 256 is enough. Trust us. */
    int i;

    /* Parse arguments */
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--safe") == 0) {
            safe_mode_attempted = 1;
            /* F7: CVE counter starts one earlier */
            cve_set_start(20250000);
        } else if (argv[i][0] == '-') {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            fprintf(stderr,
                "Did you mean --safe? (It won't help, but sure.)\n");
            return 1;
        } else {
            source_file = argv[i];
        }
    }

    if (!source_file) {
        print_usage(argv[0]);
        return 1;
    }

    /* Register crash handler */
    signal(SIGSEGV, crash_handler);
    signal(SIGABRT, crash_handler);

    /* Initialize subsystems */
    mem_init();
    cve_init();
    engine_init();

    /* Print startup banner (F6) */
    print_banner();

    /* Open source file */
    fp = fopen(source_file, "r");
    if (!fp) {
        fprintf(stderr, "Error: cannot open '%s'\n", source_file);
        fprintf(stderr,
            "Have you tried exploiting a file that exists?\n");
        return 1;
    }

    /* Main execution loop - NO error recovery. Crash is crash. */
    while (fgets(line, sizeof(line), fp)) {
        /* Strip trailing newline */
        size_t len = strlen(line);
        while (len > 0 &&
               (line[len - 1] == '\n' || line[len - 1] == '\r'))
            line[--len] = '\0';

        /* Skip empty lines */
        if (len == 0)
            continue;

        /* Parse and execute */
        lexer_execute(line);
    }

    fclose(fp);

    /* Exit audit - NoBugNoRun check (F6) */
    audit_run();

    /* audit_run() calls exit(), we never reach here */
    return 0;
}