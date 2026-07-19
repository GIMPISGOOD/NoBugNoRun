/*
 * BugNote v0.0.2-pre-exploit
 * engine.h - Vulnerability engine: 7 keywords, 14 vulnerabilities
 *
 * "Each keyword is a feature. Each feature is a vulnerability."
 */

#ifndef BUGNOTE_ENGINE_H
#define BUGNOTE_ENGINE_H

/* Initialize engine state (patch_flags etc.) */
void engine_init(void);

/* 7 keywords */
void do_note(const char *text);
void do_log(void);
void do_sep(void);
void do_todo(void);
void do_fixme(void);
void do_annotate(const char *args);
void do_patch(const char *cve_id);

/* Crash helper - outputs satirical message and terminates */
void crash_with_message(const char *msg);

#endif /* BUGNOTE_ENGINE_H */