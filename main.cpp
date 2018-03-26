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
#include <iostream>

using namespace std;


typedef struct student {
    char id;
    char name[40];
} student;

int txn_id;

char *store = "backing_store";
rvm_t rvm;

int main() {
    rvm_verbose(0);
    rvm = rvm_init(store);

    student r = {'9', "RickAndMortyBeavisAndButtheadDexter&DEE"};

    // *******************  Write data to datastore **************************
    int newfd = open("backing_store/datastudentid", O_CREAT | O_RDWR, S_IRWXU);
    void *newaddr;

    if((newaddr = mmap(NULL, sizeof(student), PROT_READ | PROT_WRITE, MAP_SHARED, newfd, 0)) < 0) {
        fprintf(stdout, "Unable to map virtual memory to disk");
    }
    write(newfd, &r, sizeof(student));
    // ******************** Write data to datastore ***************************


    student *addr;
    addr = (student *)rvm_map(rvm, "studentid", sizeof(student));
    if(addr == NULL) {
        printf("rvm_map failed\n");
    }
    else {
        printf("ID: %d\nName: %s\n", addr->id, addr->name);
        rvm_unmap(rvm, addr);
    }

    txn_id = rvm_begin_trans(rvm, 1, (void **)&addr);
//    rvm_about_to_modify(txn_id, addr, 0, sizeof(student));
    rvm_about_to_modify(txn_id, addr, 1, sizeof(student) - 1);
    addr->name[0] = 'H';
    printf("Altered string: %s\n", addr->name);
    rvm_commit_trans(txn_id);
//    rvm_abort_trans(txn_id);

//    rvm_destroy(rvm, "studentid");
    rvm_truncate_log(rvm);



    return 0;
}