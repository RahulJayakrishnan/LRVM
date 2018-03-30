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

    int intelfd, notefd, total_size = 0;
    void *intel, *note;
    struct stat int_buf, not_buf;
    intelfd = open("../testcase/intel.mp3", O_RDWR, S_IRWXU);
    notefd = open("../testcase/nokia.mp3", O_RDWR, S_IRWXU);

    fstat(intelfd, &int_buf);
    fstat(notefd, &not_buf);

    intel = mmap(NULL, int_buf.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, intelfd, 0);
    printf("intel.mp3 size: %d\n", int_buf.st_size);
    total_size += int_buf.st_size;

    note = mmap(NULL, not_buf.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, notefd, 0);
    printf("nokia.mp3 size: %d\n", not_buf.st_size);
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

    rvm_about_to_modify(trans, seg, int_buf.st_size, 435512);
    memcpy(seg + int_buf.st_size, note, 435512);

    rvm_commit_trans(trans);

    rvm_truncate_log(rvm);
    return 0;
}
