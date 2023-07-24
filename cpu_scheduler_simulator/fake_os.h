#include "fake_process.h"
#include "linked_list.h"
#pragma once


typedef struct {
  ListItem list;
  int pid;
  ListHead events;
  int current_time;
  int predicted_quantum;
  int toUpdate;
} FakePCB;

struct FakeOS;
typedef void (*ScheduleFn)(struct FakeOS* os, void* args);

typedef struct FakeOS{
  int num_cores;       
  // FakePCB* running;
   FakePCB** running_cores; // Array di PCB per i processi in esecuzione su ogni core
  ListHead ready;
  ListHead waiting;
  int timer;
  ScheduleFn schedule_fn;
  void* schedule_args;
  ListHead processes;
} FakeOS;

void FakeOS_init(FakeOS* os, int num_cores);
void FakeOS_simStep(FakeOS* os);
void FakeOS_destroy(FakeOS* os);
