#include "types.h"
#include "riscv.h"
#include "param.h"
#include "defs.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;


  argint(0, &n);
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}


#ifdef LAB_PGTBL
int
sys_pgaccess(void)
{
  uint64 va;             // 待检测页表起始地址
  int num_pages;         // 待检测页表的页数
  uint64 access_mask;    // 记录检测结果掩码的地址

  // 从用户栈中获取参数
  argaddr(0, &va);                 // va = p->trapframe->a0
  argint(1, &num_pages);           // num_pages = p->trapframe->a1
  argaddr(2, &access_mask);        // access_mask = p->trapframe->a2
  // va(pgtbltest.c 中的 buf) 为访问的数组的逻辑地址的起始地址，需要找到该逻辑地址对应的在最低级页表中的页表项的地址，作为页表的起始地址
  va = (uint64)walk(myproc()->pagetable, va, 0);
  if (num_pages <= 0 || num_pages > 512)
  {
    return -1;
  }

  uint mask[8] = {0, 0, 0, 0, 0, 0, 0, 0};
  int i;
  for (int j = 0; j < 8; j++ ) {
    for (i = 0 + 64 * j; i < 64 * (j + 1) && i < num_pages; i++ ) {
      if (*((uint64 *)va + i) & PTE_A) {
        // 检查之后清除访问位
        *((uint64 *)va + i) = *((uint64 *)va + i) & (~PTE_A);
        mask[j] = mask[j] | (1 << i);
      }
    }
    if (i >= num_pages) break;
  }

  // 将 以&mask为起始地址，长度为sizeof(mask) 的内存数据复制到，access_mask（所对应的物理地址，需要walk函数转换）的起始地址中
  copyout(myproc()->pagetable, access_mask, (char*)&mask, sizeof(mask));
  return 0;
}
#endif

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}