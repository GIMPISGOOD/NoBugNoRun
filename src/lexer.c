/*
 * BugNote v0.0.2-pre-exploit
 * lexer.c - Lexical analysis implementation
 *
 * Keywords are case-insensitive. String params use double quotes.
 * No escape handling. If your string contains a quote, congrats,
 * you found an injection vulnerability.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "engine.h"

/* MSVC-compatible case-insensitive comparison */
#ifdef _MSC_VER
#define strncasecmp _strnicmp
#define strcasecmp  _stricmp
#endif

/*
 * Check if line starts with keyword (case-insensitive),
 * followed by whitespace or end-of-line.
 */
static int match_keyword(const char *line, const char *kw)
{
    size_t kwlen = strlen(kw);
    if (strncasecmp(line, kw, kwlen) != 0)
        return 0;
    char next = line[kwlen];
    return (next == '\0' || next == ' ' || next == '\t' || next == '\r');
}

/*
 * Extract string argument from: KEYWORD "text"
 * No escape handling. Second quote terminates. Period.
 */
static const char *extract_string(const char *p)
{
    static char buf[256];
    const char *start, *end;
    size_t len;

    while (*p == ' ' || *p == '\t') p++;

    if (*p != '"') {
        buf[0] = '\0';
        return buf;
    }

    start = p + 1;
    end = strchr(start, '"');
    if (!end) {
        /* No closing quote - take rest of line (injection vuln) */
        end = start + strlen(start);
    }

    len = (size_t)(end - start);
    if (len >= sizeof(buf)) len = sizeof(buf) - 1;
    memcpy(buf, start, len);
    buf[len] = '\0';
    return buf;
}

/*
 * Extract raw arguments after keyword (for ANNOTATE and PATCH).
 */
static const char *extract_raw_args(const char *line, const char *kw)
{
    static char buf[256];
    const char *p = line + strlen(kw);
    size_t len;

    while (*p == ' ' || *p == '\t') p++;
    len = strlen(p);
    if (len >= sizeof(buf)) len = sizeof(buf) - 1;
    memcpy(buf, p, len);
    buf[len] = '\0';
    return buf;
}

void lexer_execute(const char *line)
{
    const char *p;

    /* Skip leading whitespace */
    while (*line == ' ' || *line == '\t') line++;

    /* Skip empty lines and comments */
    if (*line == '\0' || *line == '#')
        return;

    /* Match keywords (ANNOTATE before NOTE to avoid prefix conflict) */
    if (match_keyword(line, "ANNOTATE")) {
        do_annotate(extract_raw_args(line, "ANNOTATE"));
    } else if (match_keyword(line, "NOTE")) {
        p = line + 4;
        do_note(extract_string(p));
    } else if (match_keyword(line, "LOG")) {
        do_log();
    } else if (match_keyword(line, "SEP")) {
        do_sep();
    } else if (match_keyword(line, "TODO")) {
        do_todo();
    } else if (match_keyword(line, "FIXME")) {
        do_fixme();
    } else if (match_keyword(line, "PATCH")) {
        do_patch(extract_raw_args(line, "PATCH"));
    }
    /* Unknown lines: silently ignored. This is a feature. */
}