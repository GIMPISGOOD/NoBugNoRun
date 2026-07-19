/*
 * BugNote v0.0.2-pre-exploit
 * memory.c - Memory model implementation
 *
 * "16 bytes. That's all you get. That's all you need."
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memory.h"

/* Global state definitions */
BugNoteMemory mem;
unsigned char wp = 0;            /* F2: unsigned char for integer overflow */
HeapObject **heap = NULL;
int heap_count = 0;
int heap_capacity = 0;
HeapObject *leak_ptr = NULL;

void mem_init(void)
{
    memset(&mem, 0, sizeof(mem));
    wp = 0;
    heap = NULL;
    heap_count = 0;
    heap_capacity = 0;
    leak_ptr = NULL;
}

/*
 * Address mapping:
 *   0x00 - 0x0F  -> note_buf[0..15]
 *   0x10         -> output_flag
 *   0x20 - 0x2F  -> heap[0] (16 bytes: type + data)
 *   0x30+        -> subsequent heap objects, 16 bytes each
 *   other        -> FATAL: invalid address
 */
void mem_write(int addr, int val)
{
    if (addr >= 0x00 && addr <= 0x0F) {
        mem.note_buf[addr] = (char)val;
    } else if (addr == 0x10) {
        mem.output_flag = (unsigned char)val;
    } else if (addr >= 0x20) {
        int obj_idx = (addr - 0x20) / 16;
        int field_off = (addr - 0x20) % 16;
        if (obj_idx < heap_count && heap[obj_idx] != NULL) {
            ((unsigned char *)heap[obj_idx])[field_off] = (unsigned char)val;
        } else {
            fprintf(stderr,
                "\nAccess violation at 0x%04X. "
                "This address doesn't exist. Like your security budget.\n",
                addr);
            fflush(stderr);
            exit(139);
        }
    } else {
        fprintf(stderr,
            "\nAccess violation at 0x%04X. "
            "This address doesn't exist. Like your security budget.\n",
            addr);
        fflush(stderr);
        exit(139);
    }
}

int mem_read(int addr)
{
    if (addr >= 0x00 && addr <= 0x0F) {
        return (unsigned char)mem.note_buf[addr];
    } else if (addr == 0x10) {
        return mem.output_flag;
    } else if (addr >= 0x20) {
        int obj_idx = (addr - 0x20) / 16;
        int field_off = (addr - 0x20) % 16;
        if (obj_idx < heap_count && heap[obj_idx] != NULL) {
            return ((unsigned char *)heap[obj_idx])[field_off];
        }
    }
    return 0;
}

void mem_cleanup(void)
{
    int i;
    for (i = 0; i < heap_count; i++) {
        free(heap[i]);
    }
    free(heap);
    heap = NULL;
    heap_count = 0;
    heap_capacity = 0;
    leak_ptr = NULL;
}