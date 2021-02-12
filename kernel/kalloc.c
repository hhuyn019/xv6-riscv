// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_begin, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

void* pa_begin = 0;
void* ref_begin = 0;

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem;

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  // freerange(end, (void*)PHYSTOP);
   uint64 total_pages = ((uint64)PHYSTOP - (uint64)end) / PGSIZE *
                       sizeof(uint32);
  ref_begin = (void*)PGROUNDUP((uint64)end);
  pa_begin = (void*)PGROUNDUP((uint64)ref_begin + total_pages);
  memset(ref_begin, 0, pa_begin - ref_begin);
  freerange(pa_begin, (void*)PHYSTOP);
}

uint32* get_rc(void* pa) {
  uint32* p = ref_begin;
  return &p[(pa - pa_begin) / PGSIZE];
}

uint32 ref_inc(void* pa) {
  return ++(*get_rc(pa));
}

uint32 ref_dec(void* pa) {
  return --(*get_rc(pa));
}

void
freerange(void *pa_begin, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_begin);
  // for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE) 
    // kfree(p);
      for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE) {
    ref_inc(p);
    kfree(p);
  }
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

     acquire(&kmem.lock);
  if (ref_dec(pa) != 0) {
    release(&kmem.lock);
    return;
  }

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  // acquire(&kmem.lock);
  r->next = kmem.freelist;
  kmem.freelist = r;
  release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r) {
    kmem.freelist = r->next;
   ref_inc(r);
  }
  release(&kmem.lock);

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}
