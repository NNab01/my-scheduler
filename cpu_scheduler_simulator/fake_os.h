#include "fake_process.h"
#include "linked_list.h"
#pragma once

#define MAX_CORES 4


typedef struct {
  ListItem list;
  int pid;
  ListHead events;
} FakePCB;

struct FakeOS;
typedef void (*ScheduleFn)(struct FakeOS* os, void* args);

typedef struct FakeOS{
  // FakePCB* running;
  FakePCB* running_cores[MAX_CORES]; // Array di PCB per i processi in esecuzione su ogni core
  ListHead ready;
  ListHead waiting;
  int timer;
  ScheduleFn schedule_fn;
  void* schedule_args;
  ListHead processes;
} FakeOS;

void FakeOS_init(FakeOS* os);
void FakeOS_simStep(FakeOS* os);
void FakeOS_destroy(FakeOS* os);
