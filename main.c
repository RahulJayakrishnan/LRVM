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
#include <rvmlib.h>

#define SIZE 10

struct data {
    int id1;
    int id2;
    int id[SIZE];
    char name[SIZE];
};

void app_init(int bstore) {
    struct stat sb;
    struct data dat;
    fstat(bstore, &sb);
    if(sb.st_size < sizeof(struct data)) {
        fprintf(stdout, "Backing store is empty\n");
    }
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

    if((bstore = open("../bstore.txt", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR)) < 0) {
        fprintf(stdout, "Unable to open the file\n");
    }

    if((log = open("../log.txt", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR)) < 0) {
        fprintf(stdout, "Unable to open the file\n");
    }

    app_init(bstore);
    close(bstore);
    close(log);


//    // MMAP-ing
//    if((addr = mmap(NULL, 128, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) < 0) {
//        fprintf(stdout, "Unable to map virtual memory to disk");
//    }
//
//    memcpy(addr, &dat2.name[0], (size_t)5);

    return 0;
}