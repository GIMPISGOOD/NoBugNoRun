/*
 * BugNote v0.0.2-pre-exploit
 * cve.h - CVE reporting system with absurd CVSS scores
 *
 * "Every vulnerability deserves a CVE. Even the ones we ship on purpose."
 */

#ifndef BUGNOTE_CVE_H
#define BUGNOTE_CVE_H

/* Global state */
extern int cve_counter;
extern int cve_start_value;
extern int exploit_detected;
extern int combo_counter;
extern int combo_max;
extern char last_cve_type[64];
extern int cve_history[64];
extern int cve_history_count;

/* Functions */
void cve_init(void);
void cve_set_start(int start);
void cve_report(const char *type, const char *desc, const char *cvss);
void cve_update_combo(const char *type);

#endif /* BUGNOTE_CVE_H */