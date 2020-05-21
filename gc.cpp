//https://en.wikipedia.org/wiki/Cheney%27s_algorithm
#include "gc.h"

#include <cassert>
#include <cstdio>

#include <iomanip>
#include <sstream>

using std::unordered_set;

GcSemiSpace::GcSemiSpace(intptr_t* frame_ptr, int heap_size_in_words) {
  base = frame_ptr;
  heapbase = (intptr_t*) malloc(heap_size_in_words*4);
  heapcur = heapbase;
  from = 0;
  totalheapwords = heap_size_in_words;
}



void GcSemiSpace::collect(intptr_t* frame) {
  intptr_t* newheap = (intptr_t*) malloc(totalheapwords*4);
  heapcur = newheap;
  scan = newheap;
  from = 0;
  if (rootset.size() > 0) {
    rootset.clear();
  }
  from = 0;
  while (frame != base) {
    frame = frame - 12;
    copy(frame);
  }
  while (scan < heapcur) {
    intptr_t item = *(scan + 4);
    copy(item);
    scan = scan + 4;
  }
  heapbase = newheap;
}


intptr_t* GcSemiSpace::copy(intptr_t* object) {
  if (*(object-4) & 0x1 == 1) { //not a valid forwarding pointer, copy me!
    intptr_t* temp = heapcur;
    heapcur = heapcur + sizeof(object);
    *temp = *object;
    *(object-4) = *temp;
    return (object-4);
  }
  return NULL;
}



void GcSemiSpace::reset() {
  from = 0;
  base = NULL;
  scan = NULL;
  if (rootset.size() > 0) {
    rootset.clear();
  }
  stacksize = 0;
  heapbase = NULL;
  heapcur = NULL;
  stacksize = 0;
}

bool GcSemiSpace::checkspace(int numwords) {
  if (from + numwords + 1 >= totalheapwords) {
    return false;
  }
  return true;
}


intptr_t* GcSemiSpace::Alloc(int32_t num_words, intptr_t * curr_frame_ptr) {
  if (checkspace(num_words) == false) {
    collect(curr_frame_ptr);
  }
  if (checkspace(num_words) == false) {
    throw OutOfMemoryError();
  }
  intptr_t* object = heapcur;
  heapcur = heapcur + (num_words+1)*4;
  from = from + num_words + 1;
  // This is a dummy return value to make the compiler happy. Return
  // the allocated address in your implementation.
  return object + 4;
}
