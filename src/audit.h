/*
 * BugNote v0.0.2-pre-exploit
 * audit.h - Exit audit: NoBugNoRun check, security report
 *
 * "All issues are working as intended. Do not report."
 */

#ifndef BUGNOTE_AUDIT_H
#define BUGNOTE_AUDIT_H

/*
 * Called at program exit.
 * If no exploit detected -> NoBugNoRun error, exit(1).
 * If exploit detected -> print security audit report, exit(0).
 * This function does NOT return.
 */
void audit_run(void);

#endif /* BUGNOTE_AUDIT_H */