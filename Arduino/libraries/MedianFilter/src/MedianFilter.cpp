#include "MedianFilter.h"

MedianFilter::MedianFilter(byte size) : 
  filtersize(size) 
{
  // Allocate buffer
  buffer = new int[filtersize];
  sorted = new int[filtersize];

  for(byte i = 0; i < filtersize; i++) {
    buffer[i] = 0;
    sorted[i] = 0;
  }

  index = 0;
}

MedianFilter::~MedianFilter()
{
  delete[] buffer;
  delete[] sorted;
}

void MedianFilter::add(int element)
{
  buffer[index] = element;
  index = (index + 1) % filtersize;
}

int MedianFilter::get()
{
  memcpy(sorted, buffer, filtersize*sizeof(int));
  sort(sorted, 0, filtersize);
  return sorted[byte(filtersize / 2)];
}

void MedianFilter::sort(int array[], byte from, byte upTo)
{
  byte swaps;  
  do {
    swaps=0;
    for(byte i = from; i < upTo; i++) {
      if(array[i] > array[i+1]) {
        int x = array[i+1];
        array[i+1] = array[i];
        array[i] = x;
        ++swaps;
      }
    }
    --upTo;
  } while (swaps);
}

void MedianFilter::printBuffer()
{
  if(Serial) {
    Serial.print("[ ");
    for (byte i = 0; i < filtersize; i++) {
      Serial.print(buffer[i], DEC);
      Serial.print(' ');
    }
    Serial.print(']');
    Serial.println();
  }
}

void MedianFilter::printSorted()
{
  if(Serial) {
    Serial.print("[ ");
    for (byte i = 0; i < filtersize; i++) {
      Serial.print(sorted[i], DEC);
      Serial.print(' ');
    }
    Serial.print(']');
    Serial.println();
  }
}
