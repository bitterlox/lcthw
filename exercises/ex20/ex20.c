#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

void die(const char *message)
{
  if (errno) {
    perror(message);
  } else {
    printf("ERROR: %s\n", message);
  }

  exit(1);
}

// typedef creates a fake type, in this case
// for a function pointer
typedef int (*compare_cb) (int a, int b);

/**
 * A classic bubble sort function that uses a callback
 * to do the sorting
 */
int *bubble_sort(int *numbers, int count, compare_cb cmp)
{
  int temp = 0;
  int i = 0;
  int j = 0;
  int *target = malloc(count * sizeof(int));

  if (!target)
    die("memory error");

  memcpy(target, numbers, count * sizeof(int));

  for (i = 0; i < count; i++) {
    for (j = 0; j < count - 1; j++) {
      if (cmp(target[j], target[j+1]) > 0) {
        temp = target[j+1];
        target[j+1] = target[j];
        target[j] = temp;
      }
    }
  }

  // is numbers leaked?
  return target;
}

int sorted_order(int a, int b)
{
  return (a > b) - (a < b);
}


int reversed_order(int a, int b)
{
  return (a < b) - (a > b);
}

int strange_order(int a, int b)
{
  if (a == 0 || b == 0) {
    return 0;
  } else {
    return a % b;
  }
}

void test_sorting(int *numbers, int count, compare_cb cmp)
{
  int i = 0;
  int *sorted = bubble_sort(numbers, count, cmp);

  if (!sorted)
    die("failed to sort as requested");

  for (i = 0; i < count; i++) {
    printf("%d ", sorted[i]);
  }

  printf("\n");

  free(sorted);
}

void dump(compare_cb cmp) {
  unsigned char *data = (unsigned char *)cmp;

  int i = 0;

  for(i = 0; i < 25; i++) {
      printf("%02x:", data[i]);
  }

  printf("\n");
}

// void destroy(compare_cb cmp) {
//   unsigned char *data = (unsigned char *)cmp;

//   int i = 0;

//   for(i = 0; i < 25; i++) {
//     data[i] = i;
//   }

//   printf("\n");
// }

int main(int argc, char *argv[])
{
  if (argc < 2) die("usage: ex18 1 2 3 4 5");

  int count = argc - 1;
  int i = 0;

  // oooo pointer arithmetic
  char **inputs = argv + 1;

  int *numbers = malloc(sizeof(int) * count);
  if (!numbers)
    die("memory error");

  for (i = 0; i < count; i++) {
    numbers[i] = atoi(inputs[i]);
  }

  test_sorting(numbers, count, sorted_order);
  test_sorting(numbers, count, reversed_order);
  test_sorting(numbers, count, strange_order);

  free(numbers);

  printf("SORTED:");
  dump(sorted_order);

  return 0;
};
