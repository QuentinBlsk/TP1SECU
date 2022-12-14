#define _GNU_SOURCE
#include <unistd.h>
#include <sys/mman.h>
#include <err.h>
#include <stdio.h>

unsigned char mcbuf[0x1000];

int main(void) {
  if (mmap((void*)0x66000000, 0x20000000000, PROT_NONE, MAP_SHARED | MAP_ANONYMOUS | MAP_HUGETLB | MAP_NORESERVE, -1, 0) == MAP_FAILED)
    err(1, "mmap");

  for (int i=0; i<10000; i++) {
    if (mincore((void*)0x86000000, 0x1000000, mcbuf))
      perror("mincore");
    write(1, mcbuf, 0x1000);
  }
}