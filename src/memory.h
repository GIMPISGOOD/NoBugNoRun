/*
 * BugNote v0.0.2-pre-exploit
 * memory.h - Memory model: note_buf, output_flag, heap, address mapping
 *
 * "The struct layout IS the vulnerability."
 */

#ifndef BUGNOTE_MEMORY_H
#define BUGNOTE_MEMORY_H

#include <stddef.h>

/*
 * Core memory layout.
 * #pragma pack(1) ensures output_flag is PHYSICALLY adjacent to note_buf.
 * When NOTE overflows byte 16, it naturally overwrites output_flag.
 * No bounds check needed. The compiler does the dirty work for us.
 */
#pragma pack(push, 1)
typedef struct {
    char note_buf[16];           /* 0x00 - 0x0F */
    unsigned char output_flag;   /* 0x10 - physically adjacent */
} BugNoteMemory;
#pragma pack(pop)

/*
 * Heap object: the UAF / type-confusion target.
 * type is a function pointer initialized to 0xDEAD.
 * FIXME calls it directly. Pray it points somewhere valid.
 */
typedef struct {
    void (*type)(void);   /* offset 0, 8 bytes on x64 */
    void *data;           /* offset 8, 8 bytes on x64 */
} HeapObject;

/* Global state */
extern BugNoteMemory mem;
extern unsigned char wp;             /* F2: unsigned char, wraps at 255 */
extern HeapObject **heap;
extern int heap_count;
extern int heap_capacity;
extern HeapObject *leak_ptr;         /* Real pointer. Leaked. On purpose. */

/* Functions */
void mem_init(void);
void mem_write(int addr, int val);
int  mem_read(int addr);
void mem_cleanup(void);

#endif /* BUGNOTE_MEMORY_H */