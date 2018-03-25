//
// Created by hrjanardhan on 3/24/18.
//

#ifndef PROJECT3CPP_RVMLIB_H
#define PROJECT3CPP_RVMLIB_H

//
// Created by hrjanardhan on 3/22/18.
//

#ifndef PROJECT3_NEW_RVMLIB_H
#define PROJECT3_NEW_RVMLIB_H
typedef struct rvm_t {
    const char *directory;
} rvm_t;

typedef struct local_store {
    int tid;
    void *segdata;
    int segsize;
    const char *segname;
} in_mem;

/*
 * Initialization and Mapping
 */
rvm_t rvm_init(const char *directory);
void *rvm_map(rvm_t rvm, const char *segname, int size_to_create);
void rvm_unmap(rvm_t rvm, void *segbase);
void rvm_destroy(rvm_t rvm, const char *segname);

///*
// * Transactional Operations
// */
//trans_t rvm_begin_trans(rvm_t rvm, int numsegs, void **segbases);
//void rvm_about_to_modify(trans_t tid, void *segbase, int ofset, int size);
//void rvm_commit_trans(trans_t tid);
//void rvm_abort_trans(trans_t tid);
//
//
///*
// * Log Control Ops
// */
//void rvm_truncate_log(rvm_t rvm);
void rvm_verbose(int enable_fag);



#endif //PROJECT3_RVMLIB_H


#endif //PROJECT3CPP_RVMLIB_H
