//https://en.wikipedia.org/wiki/Cheney%27s_algorithm
#include "gc.h"
#include <math.h> 
#include <iostream>
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
  totalheapwords = heap_size_in_words/2;
  to = heapbase + totalheapwords;
  numobj = 0;
}

void GcSemiSpace::add(int addition) {
  heapcur = heapcur + addition;
  from = from + addition;
}

void GcSemiSpace::stackcopyhelper(intptr_t* ref, intptr_t* header, int scalar) {
  int meter = (int)header;
  for (int i = 0; i < 32; i++) {
    if ((meter & 0x1) == 0x1) {
      intptr_t* obj = (ref + i*scalar);
      *obj = (intptr_t)copy(obj);
    }
    meter = meter >> 1;
  }
}

void GcSemiSpace::copyhelper(intptr_t* ref, intptr_t* header) {
  int meter = (int)header;
  meter = meter>>1;
  for (int i = 0; i < 23; i++) {
    if ((meter & 0x1) == 0x1) {
      intptr_t* obj = ref + i;
      *obj = (intptr_t)copy(obj);
    }
    meter = meter>>1;
  }
}

void GcSemiSpace::swappointer() {
  intptr_t* temp = heapbase;
  heapbase = to;
  heapcur = to;
  to = temp;
  from = 0;
  numobj = 0;
}

void GcSemiSpace::collect(intptr_t* frame) {
  swappointer();
  while (frame != base) {
    intptr_t* argument = frame - 1;
    intptr_t* local = frame - 2;
    stackcopyhelper(frame+2,argument,1);
    stackcopyhelper(frame-3,local,-1);
    frame = (intptr_t*) *frame;
  }
  ReportGCStats(numobj,from);
}

void GcSemiSpace::addobj(intptr_t* header, intptr_t* object) {
  int size = (int)header & 0xFF000000;
  size = size/pow(2,24);
  std::cout << "size is " << size << std::endl;
  *(object-1) = ((intptr_t) heapcur) + 1;
  *heapcur = *header;
  add(1);
  for (int i = 0; i < size; i++) {
    *heapcur = *object;
    add(1);
    object = object + 1;
  } 
  numobj++;
}

intptr_t* GcSemiSpace::copy(intptr_t* object) {
  intptr_t* header;
  header = (object-1);
  if (*header & 0x1 == 1) { //a nonvalid forwarding pointer, copy me!
    addobj(header, object);
    copyhelper(object,header);
  }
  return (intptr_t*)*(object-1);
}




void GcSemiSpace::reset() {
  from = 0;
  base = NULL;
  scan = NULL;
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
  add(num_words+1);
  // This is a dummy return value to make the compiler happy. Return
  // the allocated address in your implementation.
  return object + 1;
}
