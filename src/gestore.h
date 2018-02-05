#ifndef GESTORE_H
#define GESTORE_H
#include "people.h"

unsigned long mcd(unsigned long a, unsigned long b);
void shm_init();
void shm_destroy();
void msq_init();
double elapsed_time();
void print_info();
void print_final_info();
person* choose_victim();
person* spawn_new_person(char* nome, unsigned long mcd);
void gen_child(person* p);
void end_match(person* p);
void accoppia(int a, int b);
void empty_queue(person* p);
void wait_for_messages();
void birth_death(int sig);
void kill_all();
void kill_person(person* p);
void segnala_start(person* p);
void set_signals();
void ready_receive(person* p);
void start();
void quit(int sig);
void init();
void debug(int sig);
void debug_person(person* p);

#endif
