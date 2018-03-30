/* basic.c - test that basic persistency works */

#include "rvm.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

//typedef struct
typedef struct project {
    int project1;
    int project2;
} project;

typedef struct midterm {
    int AOS;
    int APT;
    int ACA;
    project Project;
} midterm;

typedef struct student {
    int id;
    char name[40];
    midterm marks;
} student;

/* basic.c - test that basic persistency works */

#include "rvm.h"
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
    student rick = {12, "Sick"};
    rvm_verbose(1);
    rvm_t rvm;
    trans_t trans;
    student* segs[1];

    rvm = rvm_init("rvm_segments");
    rvm_destroy(rvm, "testseg");
    segs[0] = (student *) rvm_map(rvm, "testseg", sizeof(student));

    //////// Transaction 1 | Successful Commit
    trans = rvm_begin_trans(rvm, 1, (void **) segs);

    rvm_about_to_modify(trans, segs[0], 0, sizeof(int));
    segs[0]->id = 123;

    rvm_about_to_modify(trans, segs[0], 4, 40);
    sprintf(segs[0]->name, "Rick Morty");

    rvm_about_to_modify(trans, segs[0], 44, 4);
    segs[0]->marks.AOS = 99;

    rvm_about_to_modify(trans, segs[0], 48, 4);
    segs[0]->marks.APT = 99;

    rvm_about_to_modify(trans, segs[0], 52, 4);
    segs[0]->marks.ACA = 98;

    rvm_about_to_modify(trans, segs[0], 56, 4);
    segs[0]->marks.Project.project1 = 98;

    rvm_about_to_modify(trans, segs[0], 60, 4);
    segs[0]->marks.Project.project2 = 100;

    rvm_commit_trans(trans);
    rvm_truncate_log(rvm);


    ///////// Transaction 2 | Aborted
    trans = rvm_begin_trans(rvm, 1, (void **) segs);

    rvm_about_to_modify(trans, segs[0], 0, sizeof(int));
    segs[0]->id = 420;


    rvm_about_to_modify(trans, segs[0], 4, 40);
    sprintf(segs[0]->name, "Pick");

    rvm_about_to_modify(trans, segs[0], 44, 4);
    segs[0]->marks.AOS = 0;

    rvm_about_to_modify(trans, segs[0], 48, 4);
    segs[0]->marks.APT = 0;

    rvm_about_to_modify(trans, segs[0], 56, 4);
    segs[0]->marks.Project.project1 = 0;

    rvm_about_to_modify(trans, segs[0], 60, 4);
    segs[0]->marks.Project.project2 = 0;

    abort();
}


/* proc2 opens the segments and reads from them */
void proc2()
{
    student* segs[1];
    rvm_t rvm;

    rvm = rvm_init("rvm_segments");

    segs[0] = (student *) rvm_map(rvm, "testseg", sizeof(student));
    printf("************************************************\n");
    printf("Student ID: %d | Student Name: %s\n", segs[0]->id, segs[0]->name);
    printf("AOS: %d APT: %d ACA: %d\n", segs[0]->marks.AOS, segs[0]->marks.APT, segs[0]->marks.ACA);
    printf("Project 1: %d Project 2: %d\n", segs[0]->marks.Project.project1, segs[0]->marks.Project.project2);
    if(strcmp(segs[0]->name, "Rick Morty")) {
        printf("ERROR: Name is incorrect\n");
        exit(2);
    }
    if(segs[0]->id != 123) {
        printf("ERROR: ID is incorrect\n");
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
