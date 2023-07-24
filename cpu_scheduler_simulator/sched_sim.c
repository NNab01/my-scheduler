#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "fake_os.h"

FakeOS os;

typedef struct {
  float alpha;
} SchedSJBArgs;

void updatePredictedQuantum(ListHead* ready, SchedSJBArgs* args){
    ListItem* aux = ready->first;
    while(aux){
        FakePCB* pcb = (FakePCB*)aux;
        if(pcb->toUpdate==1){
            pcb->predicted_quantum = args->alpha * pcb->current_time + (1 - args->alpha) * pcb->predicted_quantum;
            pcb->toUpdate=0;
            pcb->current_time=0;
        }
        aux=aux->next;
    }
    return;
}

void schedSJFP(FakeOS* os, void* args_) {
  SchedSJBArgs* args = (SchedSJBArgs*)args_;

  updatePredictedQuantum(&os->ready, args);
  // look for the first process in ready
  // if none, return
  if (!os->ready.first)
    return;

  FakePCB* minPcb = NULL;
  ListItem* aux = os->ready.first;
  while (aux) {
    FakePCB* pcb = (FakePCB*)aux;
    if (!minPcb || pcb->predicted_quantum < minPcb->predicted_quantum) {
      minPcb = pcb;
    }
    aux = aux->next;
  }

  // Remove the process with the minimum duration from the ready queue

  FakePCB* pcb = (FakePCB*)List_detach(&os->ready, (ListItem*)minPcb);
  int core_index = 0;

  // Find the first available core to assign the process
  for (int i = 0; i < os->num_cores; i++) {
    if (!os->running_cores[i]) {
      core_index = i;
      break;
    }
  }

  os->running_cores[core_index] = pcb;

  assert(pcb->events.first);
  ProcessEvent* e = (ProcessEvent*)pcb->events.first;
  assert(e->type == CPU);
  if (e->duration > pcb->predicted_quantum) {
    ProcessEvent* qe = (ProcessEvent*)malloc(sizeof(ProcessEvent));
    qe->list.prev = qe->list.next = 0;
    qe->type = CPU;
    qe->duration = pcb->predicted_quantum;
    e->duration -= pcb->predicted_quantum;
    List_pushFront(&pcb->events, (ListItem*)qe);
  }

}



int main(int argc, char** argv) {
  int num_cores= atoi(argv[1]);
  FakeOS_init(&os, num_cores);
  SchedSJBArgs ssjb_args;
  ssjb_args.alpha=0.5;
  os.schedule_args=&ssjb_args;
  os.schedule_fn=schedSJFP;
  for (int i=2; i<argc; ++i){
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
  printf("num cores: %d\n", num_cores);
  while (1) {
    int cpu_libera = 1; // Flag per verificare se tutti i core sono inattivi

    // Controlla se ci sono processi in esecuzione su tutti i core
    for (int i = 0; i < num_cores; i++) {
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
