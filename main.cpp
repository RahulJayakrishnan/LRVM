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

char *store = "backing_store";
rvm_t rvm;

int main() {
    rvm = rvm_init(store);
    printf("Directory Name: %s\n", rvm.directory);

    rvm_map(rvm, "studentid", 50);

    return 0;
}