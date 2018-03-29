//#include <stdio.h>
//#include <stdlib.h>
//#include <sys/types.h>
//#include <sys/stat.h>
//#include <unistd.h>
//#include <fcntl.h>
//#include <sys/mman.h>
//#include <string.h>
//#include <errno.h>
//#include <sys/file.h>
//#include "rvmlib.h"
//#include <iostream>
//
//using namespace std;
//
//
//typedef struct student {
//    char id;
//    char name[40];
//} student;
//
//int txn_id;
//
//char *store = "backing_store";
//rvm_t rvm;
//
//int main() {
//    rvm_verbose(0);
//    rvm = rvm_init(store);
//
//    student r = {'9', "RickAndMortyBeavisAndButtheadDexter&DEE"};
//
//    // *******************  Write data to datastore **************************
//    int newfd = open("backing_store/datastudentid", O_CREAT | O_RDWR, S_IRWXU);
//    void *newaddr;
//
//    if((newaddr = mmap(NULL, sizeof(student), PROT_READ | PROT_WRITE, MAP_SHARED, newfd, 0)) < 0) {
//        fprintf(stdout, "Unable to map virtual memory to disk");
//    }
//    write(newfd, &r, sizeof(student));
//    // ******************** Write data to datastore ***************************
//
//
//    student *addr;
//    addr = (student *)rvm_map(rvm, "studentid", sizeof(student));
//    if(addr == NULL) {
//        printf("rvm_map failed\n");
//    }
//    else {
//        printf("ID: %d\nName: %s\n", addr->id, addr->name);
//        rvm_unmap(rvm, addr);
//    }
//
//    txn_id = rvm_begin_trans(rvm, 1, (void **)&addr);
//    rvm_about_to_modify(txn_id, addr, 0, sizeof(student));
////    rvm_about_to_modify(txn_id, addr, 1, sizeof(student) - 1);
//    addr->name[0] = 'R';
//    printf("Altered string: %s\n", addr->name);
//    rvm_commit_trans(txn_id);
////    rvm_abort_trans(txn_id);
//
////    rvm_destroy(rvm, "studentid");
//    rvm_truncate_log(rvm);
//
//
//
//
//    return 0;
//}



///////////////// BASIC.C TEST CASE //////////////////////////////////////
/* basic.c - test that basic persistency works */

#include "rvmlib.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#define TEST_STRING "hello, world"
#define OFFSET2 1000


/* proc1 writes some data, commits it, then exits */
void proc1()
{
    rvm_t rvm;
    trans_t trans;
    char* segs[1];

    rvm = rvm_init("rvm_segments");
    rvm_destroy(rvm, "testseg");
    segs[0] = (char *) rvm_map(rvm, "testseg", 10000);


    trans = rvm_begin_trans(rvm, 1, (void **) segs);

    rvm_about_to_modify(trans, segs[0], 0, 100);
    sprintf(segs[0], TEST_STRING);
    rvm_about_to_modify(trans, segs[0], OFFSET2, 100);
    sprintf(segs[0]+OFFSET2, TEST_STRING);

    rvm_commit_trans(trans);

    abort();
}


/* proc2 opens the segments and reads from them */
void proc2()
{
    char* segs[1];
    rvm_t rvm;

    rvm = rvm_init("rvm_segments");

    segs[0] = (char *) rvm_map(rvm, "testseg", 10000);
    if(strcmp(segs[0], TEST_STRING)) {
        printf("ERROR: first hello not present\n");
        exit(2);
    }
    if(strcmp(segs[0]+OFFSET2, TEST_STRING)) {
        printf("ERROR: second hello not present\n");
        exit(2);
    }

    printf("OK\n");
    exit(0);
}


int main(int argc, char **argv)
{
    int pid;

    pid = fork();
    if(pid < 0) {
        perror("fork");
        exit(2);
    }
    if(pid == 0) {
        proc1();
        exit(0);
    }

    waitpid(pid, NULL, 0);

   proc2();

    return 0;
}