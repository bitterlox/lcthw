#include <stdio.h>

int main(int argc, char *argv[])
{
  int i = 25;
  while (i > 0) {
    printf("%d", i);
    i--;
  }

  printf("\n");

  int j = 0;
  do {
    printf("%d", j);
    j++;
  } while (j < 25);

  printf("\n");

  int k = 0;
  while (0 == 0) {
    if (k > 25) {
      break;
    }

    printf("%d", k);
    k++;
  }

  printf("\n");

  return 0;
}
