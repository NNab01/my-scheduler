#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "fake_os.h"

FakeOS os;

/*typedef struct {
  int quantum;
} SchedRRArgs;*/

typedef struct {
  int quantum;
  float alpha;
} SchedSJBArgs;

void schedSJFP(FakeOS* os, void* args_) {
  SchedSJBArgs* args = (SchedSJBArgs*)args_;

  // look for the first process in ready
  // if none, return
  if (!os->ready.first)
    return;

  FakePCB* pcb = (FakePCB*)List_popFront(&os->ready);
  int core_index = 0;


  // Find the first available core to assign the process
  for (int i = 0; i < MAX_CORES; i++) {
    if (!os->running_cores[i]) {
      core_index = i;
      break;
    }
  }

  os->running_cores[core_index] = pcb;

  assert(pcb->events.first);
  ProcessEvent* e = (ProcessEvent*)pcb->events.first;
  assert(e->type == CPU);
  // Calcola il nuovo quantum previsto secondo:  q(t+1) = a * q_current + (1-a) * q(t)
  int predicted_quantum = (args->alpha) * (e->duration) + (1 - args->alpha) * (args->quantum);
  args->quantum = predicted_quantum;

  if (e->duration > args->quantum) {
    ProcessEvent* qe = (ProcessEvent*)malloc(sizeof(ProcessEvent));
    qe->list.prev = qe->list.next = 0;
    qe->type = CPU;
    qe->duration = args->quantum;
    e->duration -= args->quantum;
    List_pushFront(&pcb->events, (ListItem*)qe);
  }


printf("Nuovo quantum previsto: %d\n", predicted_quantum);

}



/*void schedRR(FakeOS* os, void* args_) {
  SchedRRArgs* args = (SchedRRArgs*)args_;

  // look for the first process in ready
  // if none, return
  if (!os->ready.first)
    return;

  FakePCB* pcb = (FakePCB*)List_popFront(&os->ready);
  int core_index = 0;

  // Find the first available core to assign the process
  for (int i = 0; i < MAX_CORES; i++) {
    if (!os->running_cores[i]) {
      core_index = i;
      break;
    }
  }

  os->running_cores[core_index] = pcb;

  assert(pcb->events.first);
  ProcessEvent* e = (ProcessEvent*)pcb->events.first;
  assert(e->type == CPU);

  // look at the first event
  // if duration > quantum
  // push front in the list of event a CPU event of duration quantum
  // alter the duration of the old event subtracting quantum
  if (e->duration > args->quantum) {
    ProcessEvent* qe = (ProcessEvent*)malloc(sizeof(ProcessEvent));
    qe->list.prev = qe->list.next = 0;
    qe->type = CPU;
    qe->duration = args->quantum;
    e->duration -= args->quantum;
    List_pushFront(&pcb->events, (ListItem*)qe);
  }
}*/


int main(int argc, char** argv) {
  FakeOS_init(&os);
  SchedSJBArgs ssjb_args;
  ssjb_args.quantum=5;
  ssjb_args.alpha=0.5;
  os.schedule_args=&ssjb_args;
  os.schedule_fn=schedSJFP;

  /*SchedRRArgs srr_args;
  srr_args.quantum=5;
  os.schedule_args=&srr_args;
  os.schedule_fn=schedRR;*/
  
  for (int i=1; i<argc; ++i){
    FakeProcess new_process;
    int num_events=FakeProcess_load(&new_process, argv[i]);
    printf("loading [%s], pid: %d, events:%d",
           argv[i], new_process.pid, num_events);
    if (num_events) {
      FakeProcess* new_process_ptr=(FakeProcess*)malloc(sizeof(FakeProcess));
      *new_process_ptr=new_process;
      List_pushBack(&os.processes, (ListItem*)new_process_ptr);
    }
  }
  printf("num processes in queue %d\n", os.processes.size);

  while (1) {
    int cpu_libera = 1; // Flag per verificare se tutti i core sono inattivi

    // Controlla se ci sono processi in esecuzione su tutti i core
    for (int i = 0; i < MAX_CORES; i++) {
      if (os.running_cores[i]) {
        cpu_libera = 0;
        break;
      }
    }

    // Se non ci sono processi in esecuzione su tutti i core e non ci sono più processi in coda, esci dal ciclo
    if (cpu_libera && !os.ready.first && !os.waiting.first && !os.processes.first) {
      break;
    }

     // Esegui il passo di simulazione del sistema operativo
  FakeOS_simStep(&os);
  }
}
