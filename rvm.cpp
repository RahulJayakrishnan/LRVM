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
#include "rvm.h"
#include <string.h>
#include <string>
#include <iosfwd>
#include <vector>
#include <iterator>
#include <dirent.h>
#include <iostream>

using namespace std;
int verbose;
vector <in_mem> localstore, undo;
trans_t tid = -1;

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
            printf("Directory does not exist\n");
            printf("Directory Created\n");
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
            printf("rvm_map - Log %s doesn't exist\n", l_filepath);
            // Create the file with the specified size
            printf("rvm_map - File created\n");
            // TRUNCATE REQD?
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
                    printf("rvm_map - Segment already mapped\n");
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

    in_mem disk_read = {-1, NULL,NULL, size_to_create, segname};
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
    for(iter = localstore.begin(); iter <= localstore.end(); iter++) {
        if(iter->segdata == segbase) {
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
    log_data ldata;
    vector <in_mem>::iterator iter;
    for(iter = localstore.begin(); iter <= localstore.end(); iter++) {
        if(iter->tid == tid) {
            iter->being_modified = false;
            ldata.tid = tid;
            strcpy(ldata.d_filepath, iter->d_filepath);
            ldata.mod_size = iter->mod_size;
            ldata.offset = iter->offset;
            ldata.flag = '\n';

            // open, mmap, append, seek, close
            int fd = open(iter->l_filepath, O_CREAT | O_RDWR, S_IRWXU);
            printf("BEGIN: %d\n", fd);
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
            iter->segdata=NULL;
    }

}

void rvm_abort_trans(trans_t tid) {
    if(tid < 0) {
        printf("rvm_abort_trans failed. Invalid tid\n");
        return;
    }
    vector <in_mem>::iterator iter;
    for(iter = localstore.begin(); iter < localstore.end(); iter++) {
        if(iter->tid == -1) {
            localstore.erase(iter);
        }
    }

    for(iter = undo.begin(); iter <= undo.end(); iter++) {
        if(iter->tid == tid) {

           memcpy(iter->original,iter->segdata,iter->segsize);
            break;// Memleak might be there if about_to_modify is called multiple times for the same segment
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
        string dir_name = dir->d_name;
        string log_str = "log";
        string str_segname;
        struct stat logbuf, databuf;
        if(dir_name.find(log_str, 0) != std::string::npos) {
            cout<<dir_name<<endl;
            str_segname = dir_name.substr(3);
            cout<<str_segname<<endl;
            const char *segname = str_segname.c_str();
            strcat(l_dest, segname);
            strcat(d_dest, segname);
            strcat(l_filepath, l_dest);
            strcat(d_filepath, d_dest);
            strcat(p_filepath, "/ghost");
            printf("l_filepath: %s\n", l_filepath);
            printf("d_filepath: %s\n", d_filepath);
            printf("p_filepath: %s\n", p_filepath);

            int logfd = open(l_filepath, O_RDWR, S_IRWXU), count = 0, skip_size = 0;
            int datafd = open(d_filepath, O_RDWR, S_IRWXU), ghost_fd;

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


                printf("Size of file: %d\n", logbuf.st_size);
                void *start_addr = logaddr;
                while(logbuf.st_size && logaddr < (start_addr + logbuf.st_size)) {
                    if(*(char *)logaddr == '\n') {
                        memcpy(&temp, logaddr, sizeof(log_data));
                        skip_size = sizeof(log_data) + temp.mod_size;
                        valid_ptr = logaddr + skip_size;

                        if(*(char *)valid_ptr != '1') {

                            //// Move till you find \n
                            count = 1;

                            while(*(char *)(++logaddr) != '\n' && logaddr < start_addr + logbuf.st_size) {
                                count++;
                            }

                            printf("Count value: %d\n", count);

                            //// \n found. Begin purging
                            if(*(char *)(logaddr) == '\n') {
                                printf("NEED TO PURGE\n");
//                                printf("Current address %p, Real End Address %p\n", logaddr, logend);
//                                printf("Value at End Value %c\n", *(char *)(logend - 1));
//                                printf("Added address %p\n", logaddr + (logbuf.st_size - count));
//                                printf("To be copied: %d bytes\n", logbuf.st_size - count);
//                                printf("Temp_Store Address: %p\n", temp_store);

                                temp_store = malloc((size_t)(logbuf.st_size - count));
//                                printf("Temp_Store Address: %p\n", temp_store);
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

                            while(*(char *)(++logaddr) != '\n' && logaddr < start_addr + logbuf.st_size) {
                                count++;
                            }

                            printf("Count value: %d\n", count);

                            //// \n found. Begin purging
//                            if(*(char *)(logaddr) == '\n') {
                                printf("NEED TO PURGE\n");
//                                printf("Current address %p, Real End Address %p\n", logaddr, logend);
//                                printf("Value at End Value %c\n", *(char *)(logend - 1));
//                                printf("Added address %p\n", logaddr + (logbuf.st_size - count));
//                                printf("To be copied: %d bytes\n", logbuf.st_size - count);
//                                printf("Temp_Store Address: %p\n", temp_store);

                                temp_store = malloc((size_t)(logbuf.st_size - count));
//                                printf("Temp_Store Address: %p\n", temp_store);
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
                            //}
                        }
//                        break; ////////////////// REMOVE THIS BREAK /////////////////////////////
                    }
//                    logaddr++;
                    fstat(logfd, &logbuf);
                }

                //// Should be spliced up
//                memcpy(&temp, logaddr, sizeof(log_data));
//                runner = logaddr;
//                printf("Runner start: %p logaddr start: %p temp addr: %p\n", runner, logaddr, &temp);
//                runner += sizeof(log_data);
//                printf("Runner end: %p\n", runner);
//                printf("Logs data size: %d\n", sizeof(log_data));
//                payload = (char *)malloc(temp.mod_size);
//                memcpy(payload, runner, temp.mod_size);
//
//
//                // Writing to datafile
//                lseek(datafd, temp.offset, SEEK_SET);
//                write(datafd, payload, temp.mod_size);


//                //TESTING
//                int newfd = open("newfile", O_CREAT | O_RDWR, S_IRWXU);
//                void *newaddr;
//
//                if((newaddr = mmap(NULL, logbuf.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, newfd, 0)) < 0) {
//                        fprintf(stdout, "Unable to map virtual memory to disk");
//                }
//                printf("TEMP MODSIZE: %d\n", temp.mod_size);
//                write(newfd, payload, temp.mod_size);
//                //TESTING

            }


            printf("LOG EXISTS\n");
        }
    }
    closedir(dr);

}