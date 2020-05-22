//https://en.wikipedia.org/wiki/Cheney,%27s_algorithm
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
  std::cout << "from and to each have this many words: " << totalheapwords << std::endl;
  to = heapbase + totalheapwords;
  numobj = 0;
  std::cout << "Bounds include" << heapcur << " and " << to << std::endl;
}


void GcSemiSpace::add(int addition) {
  heapcur = heapcur + addition;
  from = from + addition;
}

void GcSemiSpace::stackcopyhelper(intptr_t* ref, intptr_t* header, int scalar) {
  std::cout << "stackcopy helper begin" << std::endl;
  int meter = (int)*header;
  for (int i = 0; i < 32; i++) {
    if ((meter & 0x1) == 0x1) {
      intptr_t* obj = (ref + i*scalar);
      intptr_t temp = (intptr_t)copy((intptr_t*)*obj);
      std::cout << "went ok " << std::endl;
      std::cout << "MY POINTER IS" << obj << std::endl;
      std::cout << "pointer deref is" << *obj << std::endl;
      *obj = temp;
      std::cout << "I'm good" << std::endl;
    }
    meter = meter >> 1;
  }
  std::cout << "stackcopyhelper done" << std::endl;
}

void GcSemiSpace::copyhelper(intptr_t* ref, intptr_t header) {
  std::cout << "enter copyhelper" << std::endl;
  int meter = (int)header;
  std::cout << (intptr_t*)header << std::endl;
  std::cout << ref << std::endl;
  meter = meter>>1;
  for (int i = 0; i < 23; i++) {
    if ((meter & 0x1) == 0x1) {
      intptr_t* obj = ref + i;
      std::cout << "the value of obj is" << obj << std::endl;
      intptr_t temp = (intptr_t)copy((intptr_t*)*obj);
      std::cout << "Cookies" << std::endl;
      *obj = temp;
      std::cout << "Completion" << std::endl;
    }
    meter = meter>>1;
  }
  std::cout << "exit copyhelper" << std::endl;
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
    std::cout << "Begin frame start" << std::endl;
    intptr_t* argument = frame - 1;
    intptr_t* local = frame - 2;
    stackcopyhelper(frame+2,argument,1);
    stackcopyhelper(frame-3,local,-1);
    std::cout << "We done yet?" << std::endl;
    frame = (intptr_t*) *frame;
    std::cout << "now it gets good, frame end" << std::endl;
  }
  std::cout << "Done collecting!" << std::endl;
  ReportGCStats(numobj,from);
  std::cout << "Ive snitched on myself" << std::endl;
}

void GcSemiSpace::addobj(intptr_t* header, intptr_t* object) {
  std::cout << "begin add obj" << std::endl;
  int size = (int)*header & 0xFF000000;
  //std::cout << *header << std::endl;
  //std::cout << "Currsize is" << size << std::endl;
  size = size/pow(2,24);
  std::cout << "new size is " << size << std::endl;
  *heapcur = *header;
  add(1);
  *(header) = ((intptr_t) (heapcur));
  std::cout << "new pointer is" << (intptr_t*)*header << std::endl;
  for (int i = 0; i < size; i++) {
    *heapcur = *object;
    add(1);
    object = object + 1;
  } 
  numobj++;
  std::cout << "sure" << std::endl;
}


intptr_t* GcSemiSpace::copy(intptr_t* object) {
  if ((object == NULL)) {
	std::cout << "Not legit pointer" << std::endl;
	return object;
  }
  std::cout << "the copy address is " << object << std::endl;
  intptr_t* header;
  header = (object-1);
  std::cout << "header is" << header << std::endl;
  if ((*header & 0x1) == 0x0) {
	return ((intptr_t*)*header);
  }
  std::cout << "begin copy" << header << std::endl;
  intptr_t temp = *header;
  std::cout << "old heaed" << *header << std::endl;
  addobj(header, object);
  std::cout << "new head" << *header << std::endl;
  copyhelper((intptr_t*)*header,temp);
  std::cout << "end copy" << std::endl;
  std::cout << (intptr_t*)(*header) << std::endl;
  return (intptr_t*)(*header);
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
  if (from + numwords + 1 > totalheapwords) {
    return false;
  }
  return true;
}


intptr_t* GcSemiSpace::Alloc(int32_t num_words, intptr_t * curr_frame_ptr) {
  std::cout << "I wanna allocate this many words: " << num_words << std::endl;
  std::cout << "im currently at from = " << from << std::endl;
  if (checkspace(num_words) == false) {
    collect(curr_frame_ptr);
  }
  if (checkspace(num_words) == false) {
    throw OutOfMemoryError();
  }
  intptr_t* object = heapcur;
  add(num_words+1);
  numobj++;
  // This is a dummy return value to make the compiler happy. Return
  // the allocated address in your implementation.
  return object + 1;
}
