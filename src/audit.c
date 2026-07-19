/*
 * BugNote v0.0.2-pre-exploit
 * audit.c - Exit audit implementation
 *
 * F6: Exit audit report
 */

#include <stdio.h>
#include <stdlib.h>

#include "audit.h"
#include "cve.h"
#include "memory.h"

void audit_run(void)
{
    int i;

    /* NoBugNoRun: no exploit = no execution */
    if (!exploit_detected) {
        fprintf(stderr, "\n");
        fprintf(stderr,
            "NoBugNoRun: No exploit detected. "
            "Program refuses to execute.\n");
        fprintf(stderr,
            "Hint: Try overflowing something. Or use ANNOTATE.\n");
        fprintf(stderr,
            "Hint: Or don't. The bugs will find you either way.\n");
        mem_cleanup();
        exit(1);
    }

    /* F6: Security audit report */
    fprintf(stderr, "\n");
    fprintf(stderr,
        "========== BugNote Security Audit ==========\n");
    fprintf(stderr, "CVEs triggered: %d\n", cve_history_count);
    fprintf(stderr, "Combo max:      %d\n", combo_max);
    fprintf(stderr, "Exploits used:  ");
    for (i = 0; i < cve_history_count; i++) {
        fprintf(stderr, "CVE-%d", cve_history[i]);
        if (i < cve_history_count - 1)
            fprintf(stderr, " -> ");
    }
    fprintf(stderr, "\n");
    fprintf(stderr,
        "--------------------------------------------\n");
    fprintf(stderr,
        "Verdict: All issues are working as intended.\n");
    fprintf(stderr,
        "         Do not report.\n");
    fprintf(stderr,
        "============================================\n");

    mem_cleanup();
    exit(0);
}