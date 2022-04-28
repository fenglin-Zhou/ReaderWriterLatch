#include <iostream>

#include "rwlatch.h"

int main(void) {
  ReaderWriterLatch rwl;
  rwl.RLock();
  return 0;
}