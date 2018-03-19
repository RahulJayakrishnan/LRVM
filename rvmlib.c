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
    struct stat sb;
    if((myData.storefd = open(directory, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR)) < 0) {
        fprintf(stdout, "Unable to open the file | STORE\n");
    }

    if(verbose)
        fprintf(stdout, "Initialized with backing store\n");

    fstat(directory, &sb);
    if(sb.st_size == 0) {
        myData.first_init = 1;

        if(verbose)
            fprintf(stdout, "Backing store is empty\n");
    }

    return myData;
}



void rvm_verbose(int enable_fag) {
    verbose = 1;
    fprintf(stdout, "Verbose Enabled\n");
}

void *rvm_map(rvm_t rvm, const char *segname, int size_to_create) {
    void * addr;
    int log;

    if((addr = mmap(NULL, size_to_create, PROT_READ | PROT_WRITE, MAP_SHARED, rvm.logfd, 0)) < 0) {
        fprintf(stdout, "Unable to map virtual memory to disk");
    }
    if(verbose) {
        fprintf(stdout, "%p | Mapped memory segment at for %d bytes\n", addr, size_to_create);
    }

    return addr;
}

void rvm_unmap(rvm_t rvm, void *segbase) {
    munmap((void *)segbase, rvm.dataSize);
    if(verbose) {
        fprintf(stdout, "%p | Unmapped segment\n", segbase);
    }
}
