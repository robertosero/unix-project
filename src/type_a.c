#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/msg.h>
#include <string.h>
#include <errno.h>
#include "header.h"
#include "child.h"
#include "sem.h"
#include "type_a.h"



// Proprie caratteristiche
unsigned int id;
unsigned long genoma;
char* nome;
short sem_num;

unsigned int INIT_PEOPLE;
// pid del partner accoppiato
int partner;

// mcd target
unsigned long target;

// Numero di contatti rifiutati nel corrente target
unsigned long contacts;

// Divisori del proprio genoma
unsigned long* divisori;
int div_length;

int msq_match;
int msq_contact;
int msgsize;

int sem_start;
int sem_match;

char* stack[64];
int stack_length;
int debug_info;

void abbassa_target() {
  add_func("abbassa_target");
  int i = 0;
  while (i < div_length && target != divisori[i]) i++;
  if (target > 1) {
    target = divisori[i+1];
  }
  rm_func();
}


void push_contact(unsigned int id) {
  add_func("push_contact");
  contacts++;
  if (contacts >= INIT_PEOPLE) {
    abbassa_target();
    contacts = 0;
  }
  rm_func();
}

// accetta il processo B e si accoppia
void accetta(pid_t pid) {
  add_func("accetta");
  partner = pid;
  message s;
  s.data = 1;
  s.mtype = pid;
  s.pid = getpid();
  s.partner = pid;
  // Messaggio di assenso al processo B
  while (msgsnd(msq_contact,&s,msgsize,0) == -1 && errno == EINTR) continue;
  // Attende il messaggio di conferma di B
  message r;
  while (msgrcv(msq_match,&r,msgsize,getpid(),0) == -1 && errno == EINTR) continue;
  if (r.data) { // B ha confermato
    // Messaggio per il gestore
    s.mtype = getppid();
    while (msgsnd(msq_match,&s,msgsize,0) == -1 && errno == EINTR) continue;
  } else { // B e' stato ucciso quindi il gestore ha risposto al suo posto
    printf("A %d B ucciso\n",getpid());
  }
  fine_match();
  rm_func();
}

// rifiuta il processo B
void rifiuta(pid_t pid) {
  add_func("rifiuta");
  message s;
  s.data = 0;
  s.mtype = pid;
  s.pid = getpid();
  while (msgsnd(msq_contact,&s,msgsize,0) == -1 && errno == EINTR) continue;
  rm_func();
}

void ascolta() {
  add_func("ascolta");
  message r;
  while (1) {
    // ricevo il messaggio
    while (msgrcv(msq_contact,&r,msgsize,getpid(),0) == -1 && errno == EINTR) continue;
    add_match(sem_num,1);
    partner = r.pid;
    if (mcd(genoma,r.genoma) >= target) { // l'mcd corrisponde al target
      accetta(r.pid);
    } else {
      rifiuta(r.pid);
      add_match(sem_num,-1);
      push_contact(r.id);
    }
  }
  rm_func();
}

void debug(int sig) {
  printf("\n%s ",strsignal(sig));
  printf("%d {type: A, info: %d, target: %lu, genoma: %lu, partner: %d, sem: %hi, stack: [",getpid(),debug_info,target,genoma,partner,sem_num);
  for (int i = 0; i < stack_length;i++) {
    printf("%s, ",stack[i]);
  }
  printf("]}\n");
  quit(sig);
}

void init() {
  // inizializza la memoria per lo stack
  for (int i = 0;i < 64;i++) {
    stack[i] = malloc(64);
  }
  add_func("init");
  set_signals(quit,debug);
  sem_init();
  // le code di messaggi
  msq_init();
  // cerca i divisori
  trova_divisori();
  // target iniziale = genoma
  target = divisori[0];
  rm_func();
}

void quit(int sig) {
  exit(EXIT_SUCCESS);
}

void start() {
  add_func("start");
  ascolta();
  rm_func();
}

int main(int argc, char* argv[]) {
  nome = argv[1];
  genoma = strtoul(argv[2],NULL,10);
  id = strtoul(argv[3],NULL,10);
  sem_num = strtoul(argv[4],NULL,10);
  INIT_PEOPLE = strtoul(argv[5],NULL,10);
  init();
  // Segnala al gestore di essere pronto e attende lo start
  ready();
  start();
}
