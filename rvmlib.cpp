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
vector <in_mem> localstore, undo;
trans_t tid = -1;

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
        truncate(d_filepath, size_to_create);
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
        strncpy(disk_read.l_filepath, l_filepath, 40);
        strncpy(disk_read.d_filepath, d_filepath, 40);
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

trans_t rvm_begin_trans(rvm_t rvm, int numsegs, void **segbases) {
    int counter = 0;
    for(int i = 0; i < numsegs; ++i) {
        vector <in_mem>::iterator iter;
        for(iter = localstore.begin(); iter <= localstore.end(); iter++) {
            if(iter->segdata == segbases[i]) {
                if(iter->being_modified) {
                    printf("rvm_begin_trans failed. One or more segments are being modified\n");
                    return -1;
                }
                ++counter;
            }
        }
    }
    if(counter != numsegs) {
        printf("rvm_begin_trans failed. One or more segments not mapped\n");
        return -1;
    } else {
        for(int i = 0; i < numsegs; ++i) {
            vector <in_mem>::iterator iter;
            for(iter = localstore.begin(); iter <= localstore.end(); iter++) {
                if(iter->tid == tid) {
                    ++iter->tid;
                    printf("TID : %d\n", iter->tid);
                }
            }
        }
        ++tid;
        printf("TID : %d\n", tid);
        return tid;
    }
}

void rvm_about_to_modify(trans_t t_id, void *segbase, int offset, int size) {
    if(t_id < 0) {
        printf("rvm_about_to_modify failed. Invalid tid\n");
        return;
    }
    vector <in_mem>::iterator iter;
    for(iter = localstore.begin(); iter <= localstore.end(); iter++) {
        if(iter->segdata == segbase) {
            if(iter->being_modified || iter->tid != t_id) {
                return;
            }
            else {
                iter->being_modified = true;
                iter->offset = offset;
                iter->mod_size = size;
                in_mem temp;
                temp.tid = t_id;
                temp.being_modified = false;
                temp.segname = iter->segname;
                temp.segsize = iter->segsize;
                temp.segdata = malloc(temp.segsize);
                temp.mod_size = iter->mod_size;
                temp.offset = iter->offset;
                memcpy(temp.segdata, iter->segdata, temp.segsize);
                undo.push_back(temp);
                break;
            }
        }
    }

}

void rvm_commit_trans(trans_t tid) {
    if(tid < 0) {
        printf("rvm_commit_trans failed. Invalid tid");
        return;
    }
    log_data ldata;
    vector <in_mem>::iterator iter;
    for(iter = localstore.begin(); iter <= localstore.end(); iter++) {
        if(iter->tid == tid) {
            ldata.tid = tid;
            strcpy(ldata.d_filepath, iter->d_filepath);
            ldata.mod_size = iter->mod_size;
            ldata.offset = iter->offset;

            // open, mmap, append, seek, close
            int fd = open(iter->l_filepath, O_CREAT | O_RDWR, S_IRWXU);
            printf("BEGIN: %d\n", fd);
            lseek(fd, 0, SEEK_END);
            write(fd, &ldata, sizeof(ldata));
            lseek(fd, 0, SEEK_END);
            write(fd, iter->segdata + iter->offset, iter->mod_size);
        }
    }
}