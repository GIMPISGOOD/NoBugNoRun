/*
 * BugNote v0.0.2-pre-exploit
 * lexer.h - Lexical analysis: keyword recognition, parameter extraction
 *
 * "If it parses, it executes. If it doesn't parse, it's a feature."
 */

#ifndef BUGNOTE_LEXER_H
#define BUGNOTE_LEXER_H

/*
 * Parse a single line and execute the corresponding keyword.
 * Unknown lines are silently ignored (but the parser itself
 * has vulnerabilities. We just don't document them.)
 */
void lexer_execute(const char *line);

#endif /* BUGNOTE_LEXER_H */