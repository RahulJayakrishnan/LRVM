//
// Created by hrjanardhan on 3/18/18.
//
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <errno.h>
#include <sys/file.h>
#include "rvmlib.h"

int verbose;

rvm_t rvm_init(const char *directory) {
    rvm_t myData;
    struct stat buf;

    if(stat(directory, &buf) == 0) {
        printf("Directory exists\n");
    }

    else {
        printf("Directory does not exist\n");
        mkdir(directory, S_IRWXU);
        printf("Directory Created\n");
    }
    myData.directory = directory;
    return myData;
}

//
//
//void rvm_verbose(int enable_fag) {
//    verbose = 1;
//    fprintf(stdout, "Verbose Enabled\n");
//}
//
//void *rvm_map(rvm_t rvm, const char *segname, int size_to_create) {
//    void * addr;
//
//    if((addr = mmap(NULL, size_to_create, PROT_READ | PROT_WRITE, MAP_SHARED, rvm.logfd, 0)) < 0) {
//        fprintf(stdout, "Unable to map virtual memory to disk");
//    }
//    printf("ERRNO: %d\n", errno);
//    if(verbose) {
//        fprintf(stdout, "%p | Mapped memory segment at for %d bytes\n", addr, size_to_create);
//    }
//
//    return addr;
//}
//
//void rvm_unmap(rvm_t rvm, void *segbase) {
//
//}
