#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void make_route(int * array, int size) {

  printf("size of array = %d\n", size);
  for (int i = 0; i < size; i++) {
    array[i] = rand() % size;
    printf("array[%d] = %d\n", i, array[i]);
    bool unique = false;
    while (!unique) {
      bool indifference = true;
      for (int j = 0; j < i; j++) {
        if (array[i] == array[j]) {
          indifference = false;
          array[i] = rand() % size;
          printf("array[%d] = %d\n", i, array[i]);
        }
      }
      if (indifference) unique = true;
    }
  }
}
