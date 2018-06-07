#ifndef _MEDIANFILTER_H_
#define _MEDIANFILTER_H_

#include "Arduino.h"

class MedianFilter
{
  public:
    MedianFilter(byte size = 3);
    ~MedianFilter();

    void add(int element);
    int get();
    void printBuffer();
    void printSorted();

  private:
    byte filtersize;
    byte index;

    int* buffer;
    int* sorted;

    void sort(int array [], byte from, byte to);
};

#endif // _MEDIANFILTER_H_