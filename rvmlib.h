//
// Created by hrjanardhan on 3/18/18.
//

#ifndef PROJECT3_RVMLIB_H
#define PROJECT3_RVMLIB_H

typedef struct rvm_t {
    int logfd, storefd, first_init, dataSize;
} rvm_t;

typedef struct trans_t {

} trans_t;

/*
 * Initialization and Mapping
 */
rvm_t rvm_init(const char *directory);
void *rvm_map(rvm_t rvm, const char *segname, int size_to_create);
void rvm_unmap(rvm_t rvm, void *segbase);
void rvm_destroy(rvm_t rvm, const char *segname);

/*
 * Transactional Operations
 */
trans_t rvm_begin_trans(rvm_t rvm, int numsegs, void **segbases);
void rvm_about_to_modify(trans_t tid, void *segbase, int ofset, int size);
void rvm_commit_trans(trans_t tid);
void rvm_abort_trans(trans_t tid);


/*
 * Log Control Ops
 */
void rvm_truncate_log(rvm_t rvm);
void rvm_verbose(int enable_fag);



#endif //PROJECT3_RVMLIB_H
