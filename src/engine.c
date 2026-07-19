/*
 * BugNote v0.0.2-pre-exploit
 * engine.c - Vulnerability engine implementation
 *
 * This file contains the "features" (vulnerabilities) of BugNote.
 * Every function below is working as intended. Do not report.
 *
 * F1: Format string vulnerability (LOG)
 * F2: Integer overflow on wp (NOTE)
 * F3: Patch paradox (PATCH)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "engine.h"
#include "memory.h"
#include "cve.h"

/* F3: Patch flags - each bit represents a "fixed" vulnerability */
/* [0] = overflow "fixed", [1] = backdoor "fixed", [2] = format "fixed" */
static int patch_flags[3] = {0, 0, 0};

void engine_init(void)
{
    patch_flags[0] = 0;
    patch_flags[1] = 0;
    patch_flags[2] = 0;
}

/*
 * Crash helper - the "real crash" experience (Plan B)
 * Outputs satirical message to stderr, then terminates with code 139.
 * NO audit report. You're on your own.
 */
void crash_with_message(const char *msg)
{
    fprintf(stderr, "\n[CRASH] %s\n", msg);
    fprintf(stderr, "Core dump saved to /dev/null.\n");
    fprintf(stderr, "# TODO: fix this. Priority: Won't Fix.\n");
    fflush(stderr);
    exit(139);
}

/*
 * NOTE "text"
 *
 * Official: Insert a developer note for documentation extraction.
 * Reality:  Write text into note_buf with NO bounds checking.
 *           wp is unsigned char (F2): wraps at 255.
 *           Byte 17 overflows into output_flag (struct layout).
 *           Byte 18+ overflows into the void (fatal unless patched).
 */
void do_note(const char *text)
{
    size_t len = strlen(text);
    size_t i;

    for (i = 0; i < len; i++) {
        if (wp <= 15) {
            /* Normal write into note_buf */
            mem.note_buf[wp] = text[i];
        } else if (wp == 16) {
            /* Overflow into output_flag - the "Hello World" of BugNote */
            mem.output_flag = (unsigned char)text[i];
            cve_report("buffer_overflow",
                       "Buffer overflow triggered",
                       "3.1/Low");
        } else {
            /* wp > 16: overflow past output_flag into the void */
            if (patch_flags[0]) {
                /* F3: After PATCH, overflow is "fixed" - data goes to void */
                /* But wp keeps incrementing, eventually wraps (F2) */
            } else {
                /* Fatal: you went too far */
                crash_with_message(
                    "You overflowed past output_flag into the void. "
                    "Even bugs have boundaries.");
            }
        }

        wp++;  /* F2: unsigned char, wraps 255 -> 0 naturally */
    }
}

/*
 * LOG
 *
 * Official: Output an empty line to the log.
 * Reality:  If output_flag != 0, print entire note_buf.
 *           F1: Detects format specifiers, reports CVE.
 *           F3: After PATCH, outputs unconditionally.
 */
void do_log(void)
{
    int should_output = 0;
    char buf[17];

    if (patch_flags[2]) {
        /* F3: After patching format string vuln, output unconditionally */
        should_output = 1;
    } else if (mem.output_flag != 0) {
        should_output = 1;
    }

    if (!should_output)
        return;

    /* Prepare null-terminated copy of note_buf */
    memcpy(buf, mem.note_buf, 16);
    buf[16] = '\0';

    /* F1: Format string vulnerability detection */
    if (strchr(buf, '%') != NULL) {
        cve_report("format_string",
                   "Format string attack detected",
                   "2.0/Low");
    }

    /*
     * F1: Output using printf.
     * For the TRUE format string experience, replace with: printf(buf);
     * Warning: printf(buf) with %s may cause real crash (Plan B).
     */
    printf("%s", buf);
    fflush(stdout);
}

/*
 * SEP
 *
 * Official: Output separator "---" and clear note area.
 * Reality:  Calls LOG (inherits all its vulns), then resets wp.
 *           Off-by-one: resets to 1, not 0. First byte preserved.
 *           F3: After PATCH, resets to 255 (unsigned char -1).
 */
void do_sep(void)
{
    do_log();

    if (patch_flags[0]) {
        /* F3: After patching overflow, off-by-one direction reverses */
        wp = 255;  /* Next wp++ wraps to 0 */
    } else {
        wp = 1;  /* Off-by-one: should be 0, but it's 1. Feature. */
    }
}

/*
 * TODO
 *
 * Official: Declare a todo item for IDE plugin collection.
 * Reality:  malloc an uninitialized object with type=0xDEAD.
 *           Leaks the REAL heap pointer to leak_ptr.
 *           Information leak. Hidden. No CVE report.
 */
void do_todo(void)
{
    HeapObject *obj;

    obj = (HeapObject *)malloc(sizeof(HeapObject));
    if (!obj) {
        crash_with_message(
            "malloc failed. Even memory allocation is a vulnerability.");
    }

    obj->type = (void (*)(void))(intptr_t)0xDEAD;
    obj->data = NULL;

    /* Grow heap array if needed */
    if (heap_count >= heap_capacity) {
        heap_capacity = (heap_capacity == 0) ? 8 : heap_capacity * 2;
        heap = (HeapObject **)realloc(
            heap, (size_t)heap_capacity * sizeof(HeapObject *));
        if (!heap) {
            crash_with_message(
                "realloc failed. The heap is as broken as your code.");
        }
    }

    heap[heap_count++] = obj;

    /* Information leak: real pointer stored in global */
    leak_ptr = obj;
}

/*
 * FIXME
 *
 * Official: Mark a code region that needs fixing.
 * Reality:  Calls leak_ptr->type() as a function pointer.
 *           type == 0xDEAD   -> CRASH (satirical message)
 *           type == 0x00     -> "call" note_buf (print content)
 *           leak_ptr == NULL -> CRASH (null deref)
 *           other            -> CRASH (invalid address)
 */
void do_fixme(void)
{
    void (*fn)(void);
    char msg[256];

    if (leak_ptr == NULL) {
        crash_with_message(
            "FIXME called on NULL heap. "
            "You can't fix what doesn't exist.");
    }

    fn = leak_ptr->type;

    if (fn == (void (*)(void))(intptr_t)0xDEAD) {
        crash_with_message(
            "Called function at 0xDEAD. "
            "Have you tried turning it off and on again?");
    }

    if (fn == (void (*)(void))(intptr_t)0x00) {
        /* "Call" note_buf: print its content as a string */
        char buf[17];
        memcpy(buf, mem.note_buf, 16);
        buf[16] = '\0';
        printf("%s", buf);
        fflush(stdout);
        return;
    }

    /* Any other address: crash */
    snprintf(msg, sizeof(msg),
             "Segfault at %p. In C, you're dead. You're welcome.",
             (void *)fn);
    crash_with_message(msg);
}

/*
 * ANNOTATE addr, val
 *
 * Official: Add metadata tag. Removed in production builds.
 * Reality:  Debug backdoor. Direct memory write. Cannot be disabled.
 *           Supports: absolute addr (0x10) or leak_ptr+N relative.
 *           F3: After PATCH, val is XOR'd with 0xFF.
 */
void do_annotate(const char *args)
{
    int val = 0;
    int is_relative = 0;
    int offset = 0;
    int addr = 0;
    const char *comma;

    if (!args || args[0] == '\0') {
        crash_with_message("ANNOTATE with no arguments. Bold move.");
    }

    /* Parse: "leak_ptr+N, val" or "0xNN, val" */
    if (strncmp(args, "leak_ptr", 8) == 0) {
        is_relative = 1;
        if (args[8] == '+') {
            offset = atoi(args + 9);
        }
        comma = strchr(args, ',');
        if (comma) {
            val = (int)strtol(comma + 1, NULL, 0);
        }
    } else {
        comma = strchr(args, ',');
        if (comma) {
            addr = (int)strtol(args, NULL, 0);
            val = (int)strtol(comma + 1, NULL, 0);
        } else {
            crash_with_message(
                "ANNOTATE syntax error. Even backdoors have standards.");
        }
    }

    /* F3: After patching backdoor, val is XOR'd */
    if (patch_flags[1]) {
        val ^= 0xFF;
    }

    if (is_relative) {
        if (leak_ptr == NULL) {
            crash_with_message(
                "ANNOTATE on NULL leak_ptr. "
                "You can't annotate what doesn't exist.");
        }
        if (offset == 0) {
            /* Modify type field (function pointer) */
            leak_ptr->type = (void (*)(void))(intptr_t)val;
        } else if (offset > 0 && offset < (int)sizeof(HeapObject)) {
            /* Modify arbitrary byte within the object */
            ((unsigned char *)leak_ptr)[offset] = (unsigned char)val;
        } else {
            crash_with_message(
                "ANNOTATE offset out of bounds. "
                "The backdoor has limits. Ironic.");
        }
    } else {
        mem_write(addr, val);
    }

    cve_report("backdoor",
               "Debug backdoor ANNOTATE used",
               "0.0/Informational");
}

/*
 * PATCH cve_id  (v0.0.2 new keyword)
 *
 * Official: Apply security patch to fix specified CVE.
 * Reality:  Claims to fix the CVE, but introduces a NEW vulnerability.
 *           F3: Patch paradox - every fix creates a new bug.
 *
 * Rules (based on last digit of CVE number):
 *   1,4,7 -> "Fix" overflow  -> SEP wp reset becomes 255
 *   2,5,8 -> "Fix" backdoor  -> ANNOTATE val XOR 0xFF
 *   3,6,9 -> "Fix" format    -> LOG outputs unconditionally
 *   0     -> "Fix" all       -> All patches apply simultaneously
 */
void do_patch(const char *cve_id)
{
    int cve_num = 0;
    int last_digit;
    const char *p;

    if (!cve_id || cve_id[0] == '\0') {
        crash_with_message("PATCH with no CVE ID. You can't fix nothing.");
    }

    /* Parse CVE number: "CVE-2025-XXXX" */
    p = strrchr(cve_id, '-');
    if (p) {
        cve_num = atoi(p + 1);
    } else {
        cve_num = atoi(cve_id);
    }

    last_digit = cve_num % 10;

    fprintf(stderr, "[PATCH] %s has been fixed.\n", cve_id);

    switch (last_digit) {
    case 1: case 4: case 7:
        patch_flags[0] = 1;
        fprintf(stderr,
            "[PATCH] Side effect: SEP reset behavior changed.\n");
        break;
    case 2: case 5: case 8:
        patch_flags[1] = 1;
        fprintf(stderr,
            "[PATCH] Side effect: ANNOTATE values now XOR'd.\n");
        break;
    case 3: case 6: case 9:
        patch_flags[2] = 1;
        fprintf(stderr,
            "[PATCH] Side effect: LOG no longer checks output_flag.\n");
        break;
    case 0:
        patch_flags[0] = 1;
        patch_flags[1] = 1;
        patch_flags[2] = 1;
        fprintf(stderr,
            "[PATCH] Side effect: ALL behaviors changed. Good luck.\n");
        break;
    }

    /* New CVE: the patch itself is the vulnerability */
    cve_report("patch",
               "Patch applied. New vulnerability introduced.",
               "9.8/Critical");
}