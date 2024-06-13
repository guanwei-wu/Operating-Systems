#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

// for mp3
uint64
sys_thrdstop(void)
{
  int delay;
  uint64 context_id_ptr;
  uint64 handler, handler_arg;
  if (argint(0, &delay) < 0)
    return -1;
  if (argaddr(1, &context_id_ptr) < 0)
    return -1;
  if (argaddr(2, &handler) < 0)
    return -1;
  if (argaddr(3, &handler_arg) < 0)
    return -1;

  //struct proc *proc = myproc();

  //TODO: mp3
  struct proc *proc = myproc();
  proc->delay = delay;
  proc->thrdstopping = 1;
  proc->num_ticks = 0;
  proc->handler_ptr = handler;
  proc->handler_arg = handler_arg;

  int find_idle = 0;
  int context_id = 0;
  // if (copyin(proc->pagetable, (char*)&(context_id), context_id_ptr, proc->sz) == 0){
  if (copyin(proc->pagetable, (char*)&(context_id), context_id_ptr, sizeof(int)) == 0){
    if (context_id == -1){
      for (int i=0; i < MAX_THRD_NUM; i++){
        if (proc->context_idle[i] == 1){
          proc->context_idle[i] = 0;
          proc->context_id = i;
          copyout(proc->pagetable, context_id_ptr, (char*)&(i), sizeof(int));
          find_idle = 1;
          break;
        }
      }
      if (find_idle == 0){
        return -1;
      }
    }
    else{
      proc->context_id = context_id;
      // copyout(proc->pagetable, context_id_ptr, (char*)&(context_id), proc->sz);
      copyout(proc->pagetable, context_id_ptr, (char*)&(context_id), sizeof(int));
    }
  }

  return 0;
}

// for mp3
uint64
sys_cancelthrdstop(void)
{
  int context_id, is_exit;
  if (argint(0, &context_id) < 0)
    return -1;
  if (argint(1, &is_exit) < 0)
    return -1;

  if (context_id < 0 || context_id >= MAX_THRD_NUM) {
    return -1;
  }

  //struct proc *proc = myproc();

  //TODO: mp3
  struct proc *proc = myproc();
  proc->thrdstopping = 0;

  if (is_exit == 0){
    int i = proc->context_id;
    memmove( &(proc->context_data[i]), (proc->trapframe), sizeof(struct trapframe) );
    // (proc->context_data[i]) = *(proc->trapframe);
  }
  else if (is_exit == 1){
    proc->context_idle[context_id] = 1;
  }

  return proc->num_ticks;
  // return 0;
}

// for mp3
uint64
sys_thrdresume(void)
{
  int context_id;
  if (argint(0, &context_id) < 0)
    return -1;

  //struct proc *proc = myproc();

  //TODO: mp3
  struct proc *proc = myproc();
  proc->context_id = context_id;

  int j = proc->context_id;
  memmove( (proc->trapframe), &(proc->context_data[j]), sizeof(struct trapframe) );
  // (proc->trapframe) = &(proc->context_data[j]);

  return 0;
}
