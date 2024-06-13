#include "param.h"
#include "types.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "defs.h"
#include "proc.h"

/* NTU OS 2023 */
/* Page fault handler */

// int handle_pgfault() {
//   /* Find the address that caused the fault */
//   /* uint64 va = r_stval(); */

//   /* TODO */
//   panic("not implemented yet\n");
// }

int handle_pgfault(struct proc* p, uint64 addr) {

  // mp2_5 !!!

  pte_t *currentp = walk(p->pagetable, addr, 0);

  if (*currentp & PTE_S) {
    uint64 blockNO = PTE2BLOCKNO(*currentp);
    *currentp |= PTE_V;
    *currentp &= ~PTE_S;
    void* page = kalloc();
    // Begin OP
    begin_op();
    read_page_from_disk(ROOTDEV, (char*)page, blockNO);
    bfree_page(ROOTDEV,blockNO);
    end_op();
    // End OP
    *currentp = PA2PTE(page) | PTE_FLAGS(*currentp);
    return 0;
  }

  char *mem;
  if (addr >= p->sz) return -1;
  uint64 page_addr = PGROUNDDOWN(addr);
  mem = kalloc();
  if (mem == 0) return -1;
  memset(mem, 0, PGSIZE);
  if(mappages(p->pagetable, page_addr, PGSIZE, (uint64)mem, PTE_W|PTE_X|PTE_R|PTE_U) != 0){
    kfree(mem);
    return -1;
  }
  return 0;
}