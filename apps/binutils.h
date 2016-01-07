#ifndef BINUTILS_INCLUDED
#define BINUTILS_INCLUDED



int bin_ls(void **args);
int bin_ln(void **args);
int bin_rm(void **args);
int bin_mkdir(void **args);
int bin_touch(void **args);
int bin_echo(void **args);
int bin_cat(void **args);
int bin_dice(void **args);
int bin_random(void **args);
int bin_dirname(void **args);
int bin_tee(void **args);
int bin_true(void **args);
int bin_false(void **args);
int bin_arch(void **args);
int bin_wc(void **args);
int bin_gyro(void **args);
int bin_acc(void **args);
int bin_mag(void **args);
int bin_cut(void **args);
int bin_morse(void **args);
int bin_catch_me(void **args);
int bin_mem_fault(void **args);
int bin_kmem_fault(void **args);
int bin_kill(void **args);
int bin_test_realloc(void **args);
int bin_test_doublefree(void **args);
#endif
