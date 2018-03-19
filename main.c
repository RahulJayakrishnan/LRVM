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

#define SIZE 10

struct data {
    int id1;
    int id2;
    int id[SIZE];
    char name[SIZE];
};

char *store = "../bstore.txt";
char *segname = "../log.txt";

void app_init(int bstore) {
    struct stat sb;
    struct data dat;

    for(int ii = 0; ii < SIZE; ii++) {
        dat.id[ii] = ii;
        dat.name[ii] = 65 + ii;
    }
    write(bstore, &dat, sizeof(struct data));
}

int main(int argc, const char *argv[])
{
    int log, bstore;
    struct data dat, dat2;
    void * addr;
    rvm_t rvmData;

    // Verbose log is enabled by cmd args
    if(argc == 2) {
        rvm_verbose(1);
    }

    // Initializing backing store
    rvmData = rvm_init(store);
    if(rvmData.first_init) {
        app_init(rvmData.storefd);
    }

    // ATTENTION: What is a segname? Why send rvmData when we are sending segname and size explicitly???
    if((log = open(segname, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR)) < 0) {
        fprintf(stdout, "Unable to open the file\n");
    }

    rvmData.logfd = log;
    rvmData.dataSize = 2*sizeof(struct data);
    addr = rvm_map(rvmData, log, 2*sizeof(struct data));





//    // MMAP-ing
//    if((addr = mmap(NULL, 128, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) < 0) {
//        fprintf(stdout, "Unable to map virtual memory to disk");
//    }
//
//    memcpy(addr, &dat2.name[0], (size_t)5);

    rvm_unmap(rvmData, addr);
    return 0;
}