//
// Created by hrjanardhan on 3/18/18.
//
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include "rvm.h"
#include <string>
#include <vector>
#include <iterator>
#include <dirent.h>
#include <iostream>

using namespace std;
int verbose;
vector <in_mem> localstore, undo;
trans_t tid = -1;
int Maxsegnum=0;

rvm_t rvm_init(const char *directory) {
    rvm_t myData;
    struct stat buf;

    if(stat(directory, &buf) == 0) {
        if(verbose)
            printf("rvm_init - Directory %s exists\n", directory);
    }

    else {
        mkdir(directory, S_IRWXU);
        if(verbose) {
            printf("rvm_init - Directory %s does not exist | Directory Created\n", directory);
        }
    }
    myData.directory = directory;
    return myData;
}

void rvm_verbose(int enable_fag) {
    verbose = enable_fag;
    if(enable_fag)
        fprintf(stdout, "Verbose Enabled\n");
    else
        fprintf(stdout, "Verbose Disabled\n");
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
        if(verbose) {
            printf("rvm_map - Log %s exists\n", l_filepath);
        }
    }
    else {
        if(verbose) {
            printf("rvm_map - Log %s doesn't exist | Log file created\n", l_filepath);
        }
    }

    logfd = open(l_filepath, O_CREAT | O_RDWR, S_IRWXU);
    datafd = open(d_filepath, O_CREAT | O_RDWR, S_IRWXU);
    rvm_truncate_log(rvm);
    vector <in_mem>::iterator it;
    if(localstore.size()) {
        for(it = localstore.begin(); it != localstore.end(); it++) {
            if(!strcmp(it->segname, segname)) {
                if(verbose)
                    printf("rvm_map - Segment %s already mapped\n", segname);
                return NULL;
            }

            if(verbose){
                printf("rvm_map - Mapping for %s doesn't exist\n", segname);
            }
            truncate(d_filepath, size_to_create);
        }
    }
    else {
        if(verbose) {
            printf("rvm_map - Mapping for %s doesn't exist\n", segname);
        }
        truncate(d_filepath, size_to_create);
    }

    // Read the logfile's size -> if it's non zero, truncate(make it consistent and clear logs) and then proceed
    fstat(logfd, &buf);

    if((addr = mmap(NULL, size_to_create, PROT_READ | PROT_WRITE, MAP_SHARED, datafd, 0)) < 0) {
        fprintf(stdout, "Unable to map virtual memory to disk");
    }

    in_mem disk_read;
// = {-1, NULL, NULL, size_to_create, segname};

    disk_read.tid = -1;
    disk_read.segdata = NULL;
    disk_read.original = NULL;
    disk_read.segsize = size_to_create;
    disk_read.segname = segname;
    disk_read.being_modified = false;
    disk_read.offset = 0;
    disk_read.mod_size = 0;

    fstat(datafd, &buf);
    if(buf.st_size >= size_to_create) {
        disk_read.segdata = malloc(size_to_create);
        memcpy(disk_read.segdata, addr, size_to_create);
        strncpy(disk_read.l_filepath, l_filepath, 40);
        strncpy(disk_read.d_filepath, d_filepath, 40);
        localstore.push_back(disk_read);
        munmap(addr, size_to_create);
        return disk_read.segdata;
    }
    else {
        if(verbose) {
            printf("rvm_map - Trying to read more data than filesize\n");
        }
    }

    return NULL;
}

void rvm_unmap(rvm_t rvm, void *segbase) {
    vector <in_mem>::iterator iter;
    for(iter = localstore.begin(); iter <= localstore.end(); iter++) {
        if(segbase == iter->segdata && !iter->invalid) {
            if(verbose) {
                printf("rvm_unmap - Unmapped: %p\n", segbase);
            }
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
                printf("rvm_unmap - Cannot destroy a mapped segment %s\n", segname);
                return;
            }
        }
    }
    else {
        remove(l_filepath);
        remove(d_filepath);
        printf("rvm_unmap - Segment name %s and its backing store destroyed successfully\n", segname);
    }
}

trans_t rvm_begin_trans(rvm_t rvm, int numsegs, void **segbases) {
    int counter = 0;
    for(int i = 0; i < numsegs; ++i) {
        vector <in_mem>::iterator iter;
        for(iter = localstore.begin(); iter <= localstore.end(); iter++) {
            if(iter->segdata == segbases[i] && !iter->invalid) {
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
            Maxsegnum=numsegs;
            for(iter = localstore.begin(); iter <= localstore.end(); iter++) {
                if(iter->tid == tid) {
//                    ++iter->tid;

                }
            }
        }
        ++tid;
        return tid;
    }
}

void rvm_about_to_modify(trans_t t_id, void *segbase, int offset, int size) {
    if(t_id < 0) {
        printf("rvm_about_to_modify failed. Invalid tid\n");
        return;
    }
    vector <in_mem>::iterator iter;
    vector <in_mem>::iterator it;
    for(iter = localstore.begin(); iter <= localstore.end(); iter++) {
        if(iter->segdata == segbase && !iter->invalid) {
            printf("*\n");


//                iter->being_modified = true;
//                iter->offset = offset;
//                iter->mod_size = size;
            in_mem temp;
            temp.tid = t_id;
            temp.being_modified = false;
            temp.segname = iter->segname;
            temp.segsize = iter->segsize;
            temp.original=iter->segdata;
            temp.segdata = malloc(temp.segsize);
            temp.mod_size = iter->mod_size;
            temp.offset = iter->offset;
            memcpy(temp.segdata, iter->segdata, temp.segsize);


            undo.push_back(temp);

            iter->being_modified = true;
            temp.mod_size = size;
            temp.offset = offset;
            temp.tid = t_id;
            strcpy(temp.d_filepath, iter->d_filepath);
            strcpy(temp.l_filepath, iter->l_filepath);
            temp.segname = iter->segname;
            temp.segdata = iter->segdata;
            temp.segsize = iter->segsize;
            localstore.push_back(temp);
            break;

        }
    }

}

void rvm_commit_trans(trans_t tid) {
    if(tid < 0) {
        printf("rvm_commit_trans failed. Invalid tid");
        return;
    }

    vector <in_mem>::iterator iter;
    for(iter = localstore.begin(); iter <= localstore.end(); iter++) {
        if(iter->tid == tid) {
            log_data ldata;
            iter->being_modified = false;
            ldata.tid = tid;
            strcpy(ldata.d_filepath, iter->d_filepath);
            ldata.mod_size = iter->mod_size;
            ldata.offset = iter->offset;
            strcpy(ldata.flag, "\t\n\t\n");

            // open, mmap, append, seek, close
            int fd = open(iter->l_filepath, O_CREAT | O_RDWR, S_IRWXU);
            lseek(fd, 0, SEEK_END);
            write(fd, &ldata, sizeof(ldata));
            lseek(fd, 0, SEEK_END);
            write(fd, iter->segdata + iter->offset, iter->mod_size);
            lseek(fd, 0, SEEK_END);
            valid_byte valid = '1';
            write(fd, &valid, 1);
        }
    }
    for(iter = localstore.begin(); iter <= localstore.end(); iter++) {
        if(iter->tid==-1){
            iter->being_modified=false;
        }
        else if(iter->tid==tid)
            iter->invalid=true;
    }

}

void rvm_abort_trans(trans_t tid) {
    if(tid < 0) {
        printf("rvm_abort_trans failed. Invalid tid\n");
        return;
    }
    vector <in_mem>::iterator iter;
//    for(iter = localstore.begin(); iter < localstore.end(); iter++) {
//        if(iter->tid == -1) {
//            localstore.erase(iter);
//        }
//    }
    const char* segn;
    for(iter = undo.begin(); iter <= undo.end();iter++) {
        if(iter->tid == tid) {

            memcpy(iter->original,iter->segdata,iter->segsize);
            --Maxsegnum;
            if(Maxsegnum==0)
                break;
        }
    }
    if(verbose) {
        printf("rvm_abort_trans - Aborted transaction\n");
    }
}

void rvm_truncate_log(rvm_t rvm) {
    struct dirent *dir;
    DIR *dr = opendir(rvm.directory);
    if(dr == NULL) {
        printf("Directory doesn't exist\n");
        return;
    }

    char l_filepath[40], d_filepath[40], p_filepath[40];
    char l_dest[40] = "/log";
    char d_dest[40] = "/data";

    strncpy(l_filepath, rvm.directory, 40);
    strncpy(d_filepath, rvm.directory, 40);
    strncpy(p_filepath, rvm.directory, 40);

    while((dir = readdir(dr)) != NULL) {
        for(int jj = 0; jj < 40; ++jj) {
            l_dest[jj] = '\0';
            d_dest[jj] = '\0';
            l_filepath[jj] = '\0';
            d_filepath[jj] = '\0';
            p_filepath[jj] = '\0';
        }
        strncpy(l_filepath, rvm.directory, 40);
        strncpy(d_filepath, rvm.directory, 40);
        strncpy(p_filepath, rvm.directory, 40);
        strncpy(l_dest, "/log", 4);
        strncpy(d_dest, "/data", 5);

        string dir_name = dir->d_name;
        string log_str = "log";
        string str_segname;
        struct stat logbuf, databuf;
        if(dir_name.find(log_str, 0) != std::string::npos) {
            str_segname = dir_name.substr(3);
            const char *segname = str_segname.c_str();
            strcat(l_dest, segname);
            strcat(d_dest, segname);
            strcat(l_filepath, l_dest);
            strcat(d_filepath, d_dest);
            strcat(p_filepath, "/ghost");
//            printf("l_filepath: %s\n", l_filepath);
//            printf("d_filepath: %s\n", d_filepath);
//            printf("p_filepath: %s\n", p_filepath);

            int logfd = open(l_filepath, O_RDWR, S_IRWXU), count = 0, skip_size = 0;
            int datafd = open(d_filepath, O_RDWR, S_IRWXU), ghost_fd;

            if(logfd < 0 || datafd < 0) return;

            fstat(logfd, &logbuf);
            fstat(datafd, &databuf);

            if(logbuf.st_size) {
                printf("rvm_truncate_log - Truncating log %s\n", l_filepath);
                log_data temp;
                void *logaddr, *dataddr, *logend, *runner, *payload, *valid_ptr, *temp_store, *ghost_addr;

                if((logaddr = mmap(NULL, logbuf.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, logfd, 0)) < 0) {
                    fprintf(stdout, "Unable to map virtual memory to disk");
                }

                logend = logaddr + logbuf.st_size;

                if((dataddr = mmap(NULL, databuf.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, datafd, 0)) < 0) {
                    fprintf(stdout, "Unable to map virtual memory to disk");
                }

                void *start_addr = logaddr;
                while(logbuf.st_size && logaddr < (start_addr + logbuf.st_size)) {

                    if(!strcmp(((log_data *)logaddr)->flag, "\t\n\t\n")) {
                        memcpy(&temp, logaddr, sizeof(log_data));
                        skip_size = sizeof(log_data) + temp.mod_size;
                        valid_ptr = logaddr + skip_size;

                        if(*(char *)valid_ptr != '1') {

                            //// Move till you find \n
                            count = 1;

                            while(strcmp(((log_data *)(++logaddr))->flag, "\t\n\t\n") && logaddr < start_addr + logbuf.st_size) {
                                count++;
                            }

                            //// \n found. Begin purging
                            if(!strcmp(((log_data *)logaddr)->flag, "\t\n\t\n")) {
                                if(verbose) {
                                    printf("rvm_truncate_log - Truncating\n");
                                }
//

                                temp_store = malloc((size_t)(logbuf.st_size - count));
                                memcpy(temp_store, logaddr, (size_t)(logbuf.st_size - count));
                                ghost_fd = open(p_filepath, O_CREAT | O_RDWR, S_IRWXU);
                                ghost_addr = mmap(NULL, logbuf.st_size - count, PROT_READ | PROT_WRITE, MAP_SHARED, ghost_fd, 0);
                                write(ghost_fd, temp_store, logbuf.st_size - count);
                                free(temp_store);
                                close(logfd);
                                close(ghost_fd);
                                remove(l_filepath);
                                rename(p_filepath, l_filepath);
//                              remove(p_filepath);
                                logfd = open(l_filepath, O_RDWR, S_IRWXU);
                                fstat(logfd, &logbuf);
                                logaddr = mmap(NULL, logbuf.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, logfd, 0);
                                start_addr = logaddr;
                            }

                            else if(logaddr >= start_addr + logbuf.st_size) {
                                ///// DELETE THE FILE
                                remove(l_filepath);
                            }
                        }
                        else {
                            // Write to data store and purge previous
                            memcpy(&temp, logaddr, sizeof(log_data));
                            runner = logaddr;
                            runner += sizeof(log_data);
                            payload = (char *)malloc(temp.mod_size);
                            memcpy(payload, runner, temp.mod_size);

                            // Writing to datafile
                            lseek(datafd, temp.offset, SEEK_SET);
                            write(datafd, payload, temp.mod_size);

                            // Purging logfile

                            count = 1;

                            while(strcmp(((log_data *)(++logaddr))->flag, "\t\n\t\n") && logaddr < start_addr + logbuf.st_size) {
                                count++;
                            }

                            temp_store = malloc((size_t)(logbuf.st_size - count));
                            memcpy(temp_store, logaddr, (size_t)(logbuf.st_size - count));
                            ghost_fd = open(p_filepath, O_CREAT | O_RDWR, S_IRWXU);
                            ghost_addr = mmap(NULL, logbuf.st_size - count, PROT_READ | PROT_WRITE, MAP_SHARED, ghost_fd, 0);
                            write(ghost_fd, temp_store, logbuf.st_size - count);
                            free(temp_store);
                            close(logfd);
                            close(ghost_fd);
                            remove(l_filepath);
                            rename(p_filepath, l_filepath);
                            remove(p_filepath);
                            logfd = open(l_filepath, O_RDWR, S_IRWXU);
                            fstat(logfd, &logbuf);
                            logaddr = mmap(NULL, logbuf.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, logfd, 0);
                            start_addr = logaddr;
                        }
                    }
                    fstat(logfd, &logbuf);
                }
            }
        }
    }
    closedir(dr);

}