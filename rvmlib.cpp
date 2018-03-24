//
// Created by hrjanardhan on 3/18/18.
//
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <errno.h>
#include <sys/file.h>
#include "rvmlib.h"
#include <string.h>
#include <string>
#include <iosfwd>
#include <vector>
#include <iterator>

using namespace std;
int verbose;
vector <in_mem> localstore;

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

void rvm_verbose(int enable_fag) {
    verbose = 1;
    fprintf(stdout, "Verbose Enabled\n");
}

void *rvm_map(rvm_t rvm, const char *segname, int size_to_create) {
    void * addr;
    struct stat buf;
    int logfd, datafd;
    if(segname == NULL) {
        return NULL;
    }

    char l_filepath[40], d_filepath[40];
    char l_dest[40] = "/log";
    char d_dest[40] = "/data";

    strncpy(l_filepath, rvm.directory, 40);
    strncpy(d_filepath, rvm.directory, 40);
    strcat(l_dest, segname);
    strcat(d_dest, segname);
    strcat(l_filepath, l_dest);
    strcat(d_filepath, d_dest);

    if(stat(l_filepath, &buf) == 0) {
        printf("%s exists\n", l_filepath);
    }
    else {
        printf("%s doesn't exist\n", l_filepath);
        // Create the file with the specified siez
        printf("File created\n");
        // TRUNCATE REQD?
    }

    logfd = open(l_filepath, O_CREAT | O_RDWR, S_IRWXU);
    datafd = open(d_filepath, O_CREAT | O_RDWR, S_IRWXU);
    printf("DFILE: %s\n", d_filepath);


    vector <in_mem>::iterator it;
    if(localstore.size()) {
        for(it = localstore.begin(); it != localstore.end(); it++) {
            if(!strcmp(it->segname, segname)) {
                printf("Segment already mapped\n");
                return NULL;
            }
        }
    }
    else {
        printf("Mapping doesn't exist\n");
    }

    if((addr = mmap(NULL, size_to_create, PROT_READ | PROT_WRITE, MAP_SHARED, datafd, 0)) < 0) {
        fprintf(stdout, "Unable to map virtual memory to disk");
    }


    in_mem temp = {2, NULL, 23, "studentid"};
    localstore.push_back(temp);
//
//    if((addr = mmap(NULL, size_to_create, PROT_READ | PROT_WRITE, MAP_SHARED, rvm.logfd, 0)) < 0) {
//        fprintf(stdout, "Unable to map virtual memory to disk");
//    }
//    printf("ERRNO: %d\n", errno);
//    if(verbose) {
//        fprintf(stdout, "%p | Mapped memory segment at for %d bytes\n", addr, size_to_create);
//    }

    return addr;
}
//
//void rvm_unmap(rvm_t rvm, void *segbase) {
//
//}
