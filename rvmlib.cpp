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
        printf("Log %s exists\n", l_filepath);
    }
    else {
        printf("Log %s doesn't exist\n", l_filepath);
        // Create the file with the specified size
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

    // Read the logfile's size -> if it's non zero, truncate(make it consistent and clear logs) and then proceed
    fstat(logfd, &buf);
    if(!buf.st_size) {
//        rvm_truncate_log(rvm);
        // Find the segnames
    }

    if((addr = mmap(NULL, size_to_create, PROT_READ | PROT_WRITE, MAP_SHARED, datafd, 0)) < 0) {
        fprintf(stdout, "Unable to map virtual memory to disk");
    }

    in_mem disk_read = {-1, NULL, size_to_create, segname};
    fstat(datafd, &buf);
    if(buf.st_size >= size_to_create) {
        disk_read.segdata = malloc(size_to_create);
        memcpy(disk_read.segdata, addr, size_to_create);
        localstore.push_back(disk_read);
        printf("Unmap status: %d\n", munmap(addr, size_to_create));
        return disk_read.segdata;
    }
    else {
        printf("Trying to read more data than filesize\n");
    }

    return NULL;
}

void rvm_unmap(rvm_t rvm, void *segbase) {
    vector <in_mem>::iterator iter;
    printf("Begin: %p End: %p\n", localstore.begin(), localstore.end());
    for(iter = localstore.begin(); iter <= localstore.end(); iter++) {
        if(segbase == iter->segdata) {
            printf("Unmapped: %p\n", segbase);
            localstore.erase(iter);
            break;
        }
    }

}

void rvm_destroy(rvm_t rvm, const char *segname) {
    char l_filepath[40], d_filepath[40];
    char l_dest[40] = "/log", d_dest[40] = "/data";

    strncpy(l_filepath, rvm.directory, 40);
    strncpy(d_filepath, rvm.directory, 40);
    strcat(l_dest, segname);
    strcat(d_dest, segname);
    strcat(l_filepath, l_dest);
    strcat(d_filepath, d_dest);

    vector <in_mem>::iterator it;
    if(localstore.size()) {
        for(it = localstore.begin(); it <= localstore.end(); it++) {
            if(!strcmp(it->segname, segname)) {
                printf("Cannot destroy a mapped segment\n");
                return;
            }
        }
    }
    else {
        remove(l_filepath);
        remove(d_filepath);
        printf("Segment name %s and its backing store destroyed successfully\n", segname);
    }
}