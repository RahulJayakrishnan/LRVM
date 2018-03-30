/* Test rvm_truncate_log() correctly removes log and applies
changes to segments. */

#include "rvm.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>

#define TEST_STRING1 "hello, world"
#define TEST_STRING2 "bleg!"
#define OFFSET2 1000


int main(int argc, char **argv)
{
//    rvm_t rvm;
//    char *seg;
//    void *segs[1];
//    trans_t trans;
//
//    // rvm = rvm_init(__FILE__ ".d");
//    rvm = rvm_init("rvm_segments");
//
//    rvm_destroy(rvm, "testseg");
//
//    segs[0] = (char *) rvm_map(rvm, "testseg", 10000);
//    seg = (char *) segs[0];
//
//    trans = rvm_begin_trans(rvm, 1, segs);
//    rvm_about_to_modify(trans, seg, 0, 100);
//    sprintf(seg, TEST_STRING1);
//
//    rvm_about_to_modify(trans, seg, OFFSET2, 100);
//    sprintf(seg+OFFSET2, TEST_STRING2);
//
//    rvm_commit_trans(trans);
//
//
//    printf("Before Truncation:\n");
//    // system("ls -l " __FILE__ ".d");
//    system("ls -l rvm_segments");
//
//    printf("\n\n");
//
//    rvm_truncate_log(rvm);
//
//    printf("\nAfter Truncation:\n");
//    // system("ls -l " __FILE__ ".d");
//    system("ls -l rvm_segments");
//
//    rvm_unmap(rvm, seg);

    int intelfd, notefd, total_size = 0;
    void *intel, *note;
    struct stat int_buf, not_buf;
    intelfd = open("intel.mp3", O_RDWR, S_IRWXU);
    notefd = open("note.mp3", O_RDWR, S_IRWXU);

    fstat(intelfd, &int_buf);
    fstat(notefd, &not_buf);

    intel = mmap(NULL, int_buf.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, intelfd, 0);
    printf("inte.mp3 size: %d\n", int_buf.st_size);
    total_size += int_buf.st_size;

    note = mmap(NULL, not_buf.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, notefd, 0);
    printf("note.mp3 size: %d\n", not_buf.st_size);
    total_size += not_buf.st_size;

    printf("Total Size: %d\n", total_size);

    rvm_t rvm;
    void *seg;
    trans_t trans;
    rvm = rvm_init("rvm_segments");
    rvm_destroy(rvm, "testseg");

    seg = rvm_map(rvm, "testseg", total_size);

    trans = rvm_begin_trans(rvm, 1, &seg);
    rvm_about_to_modify(trans, seg, 0, int_buf.st_size);
    memcpy(seg, intel, int_buf.st_size);

    rvm_about_to_modify(trans, seg, int_buf.st_size, 20000);
    memcpy(seg + int_buf.st_size, note + 150, 20000);

    rvm_commit_trans(trans);

    rvm_truncate_log(rvm);
    return 0;
}
