/*
 * BugNote v0.0.2-pre-exploit
 * cve.c - CVE reporting, combo system, absurd CVSS scoring
 *
 * F4: Combo system
 * F5: Absurd CVSS scores
 */

#include <stdio.h>
#include <string.h>

#include "cve.h"

/* Global state definitions */
int cve_counter = 20250001;
int cve_start_value = 20250001;
int exploit_detected = 0;
int combo_counter = 0;
int combo_max = 0;
char last_cve_type[64] = {0};
int cve_history[64];
int cve_history_count = 0;

void cve_init(void)
{
    cve_counter = cve_start_value;
    exploit_detected = 0;
    combo_counter = 0;
    combo_max = 0;
    memset(last_cve_type, 0, sizeof(last_cve_type));
    memset(cve_history, 0, sizeof(cve_history));
    cve_history_count = 0;
}

void cve_set_start(int start)
{
    cve_start_value = start;
    cve_counter = start;
}

/*
 * F4: Combo system
 * Consecutive different vulnerability types increment combo.
 * Same type resets to 1.
 */
void cve_update_combo(const char *type)
{
    if (strcmp(type, last_cve_type) != 0) {
        combo_counter++;
        strncpy(last_cve_type, type, sizeof(last_cve_type) - 1);
        last_cve_type[sizeof(last_cve_type) - 1] = '\0';
    } else {
        combo_counter = 1;
    }

    if (combo_counter > combo_max)
        combo_max = combo_counter;

    if (combo_counter >= 3) {
        fprintf(stderr, "[COMBO x%d] ", combo_counter);
        if (combo_counter == 3) {
            fprintf(stderr,
                "Impressive. You're basically a security researcher now.\n");
        } else if (combo_counter == 4) {
            fprintf(stderr,
                "The exploit chain is getting spicy.\n");
        } else if (combo_counter == 5) {
            fprintf(stderr,
                "Are you writing an exploit or a resume?\n");
        } else if (combo_counter >= 6 && combo_counter < 8) {
            fprintf(stderr,
                "OK this is getting ridiculous.\n");
        } else {
            fprintf(stderr,
                "Please stop. The CVE database can't take it anymore.\n");
        }
    }
}

/*
 * F5: Absurd CVSS scores
 * Buffer overflow  = 3.1/Low  ("only 16 bytes")
 * Debug backdoor   = 0.0/Info ("officially doesn't exist")
 * Format string    = 2.0/Low  ("user shouldn't type %x")
 * Type confusion   = 5.0/Med  ("requires multiple steps")
 * Patch paradox    = 9.8/Crit ("patches are the most dangerous vector")
 * Integer overflow = 1.0/Low  ("256 bytes is enough")
 */
void cve_report(const char *type, const char *desc, const char *cvss)
{
    fprintf(stderr, "[CVE-%d] %s (CVSS: %s)\n", cve_counter, desc, cvss);

    /* F4: Update combo */
    cve_update_combo(type);

    /* Record history */
    if (cve_history_count < 64) {
        cve_history[cve_history_count++] = cve_counter;
    }

    cve_counter++;
    exploit_detected = 1;
}