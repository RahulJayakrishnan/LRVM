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
    int id;
    char name[40];
} student;


char *store = "backing_store";
rvm_t rvm;

int main() {
    rvm = rvm_init(store);
    printf("Directory Name: %s\n", rvm.directory);

    student r = {123, "Rick and Morty!"};
    student *addr;
    addr = (student *)rvm_map(rvm, "studentid", sizeof(student));
    if(addr == NULL) {
        printf("rvm_map failed\n");
    }
    else {
        printf("ID: %d\nName: %s\n", addr->id, addr->name);
        rvm_unmap(rvm, addr);
    }

//    rvm_destroy(rvm, "studentid");

    /* To make seg section */
//    addr = (student *)rvm_map(rvm, "studentid", sizeof(student));
//    if(addr == NULL) {
//        printf("rvm_map failed\n");
//    }
//    else {
//        printf("ID: %d\nName: %s\n", addr->id, addr->name);
//    }


//    addr = (student *)rvm_map(rvm, "studentid", sizeof(student));


//    addr = (student *)rvm_map(rvm, "studentid", sizeof(student));
//    student *addr;
//    int datafd;
//    student r = {123, "Rick and Morty!"};
//    datafd = open("backing_store/datastudentid", O_CREAT | O_RDWR, S_IRWXU);
//    write(datafd, &r, sizeof(student));
//
//
//    addr = (student *)rvm_map(rvm, "studentid", sizeof(student));
//    printf("ID: %d\nName: %s\n", addr->id, addr->name);


    return 0;
}