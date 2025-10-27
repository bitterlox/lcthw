#include <stdio.h>

int main(int argc, char *argv[])
{
  // create two arrays
  int ages[] = { 23, 43, 12, 89, 2 };
  char _alan[] = {'A', 'l', 'a', 'n', '\0'};
  char *alan = _alan;
  char frank[] = {'F', 'r', 'a', 'n', 'k', '\0'};
  char mary[] = {'M', 'a', 'r', 'y', '\0'};
  char john[] = {'J', 'o', 'h', 'n', '\0'};
  char lisa[] = {'L', 'i', 's', 'a', '\0'};
  char *names[] = { alan, frank, mary, john, lisa };

  // get the size of ages
  int count = sizeof(ages) / sizeof(int);
  int i = 0;

  printf("ages %ld ", sizeof(ages));
  printf("int %ld ", sizeof(int));
  printf("%ld \n", sizeof(ages) / sizeof(int));

  printf("names %ld ", sizeof(names));
  printf("char %ld ", sizeof(char *));
  printf("%ld \n", sizeof(names) / sizeof(char *));

  // first way using indexing
  for (i = 0; i < count; i++) {
    printf("%s has %d years alive.\n", names[i], ages[i]);
  }

  printf("---\n");

  // setup pointers to the start of the array
  int *cur_age = ages;
  char **cur_name = names;


  // second way using pointers
  for (i = 0; i < count; i++) {
    printf("%s is %d years old.\n", *(cur_name+i), *(cur_age+i));
    printf("%b is %d years old.\n", *(cur_name+i), *(cur_age+i));
    printf("(*%p)->%p is (*%p)->%p years old.\n", cur_name+i,*(cur_name+i), cur_age+i, *(cur_age+i));
  }


  // i want to see if i can get the same address out of cur_age
  // as i get from the above printf
  // keep in mind that since we cast it to an int each increment will
  // go forward by 4 bytes instead of 8 due to different sizing between
  // int and *char
  
  // what this does:
  // strip type information from names; you had a an
  // array of character pointers
  // (or a pointer to character pointers, **char)
  // you get a pointer to integers (*int)
  // so you get a pointers to integers which
  // happen to be memory addresses
  int *_cur_age = (int *)names;
  //
  

  for (i = 0; i < count; i++) {
    // an int is less wide than a character pointer tho
    // so we need to figure out by how much
    int end_char = sizeof(char *) / sizeof(int);

    printf("end_char: %d.\n", end_char);

    // ok so now i get it. this _ptr here is:
    // base pointer pointing to array of ints *_cur_age
    // we do some arithmetic and get an address offset from that base ptr
    // at this address another pointer is stored
    // which points to our original characters
    // to get the characters back we need to dereference this second
    // pointer, but up until now i was derefencing only the first one
    // and interpreting that as a string
    int *_ptr = _cur_age+(i*end_char);
    printf("ptr: %p. \n", _ptr);
    printf("ptr: %p. \n", *_ptr);

    char *str = *(char **)(_ptr);
    printf("str: %s. \n", str);

    // the pointer is correct but i don't know how to dereference
    // it safely yet. i need to short-circuit the compiler thing
    // that auto calculates offsets
    // if it try to deref str it segfaults
  }

  printf("---\n");

  // third way, pointers are just arrays
  for (i = 0; i < count; i++) {
    printf("%s is %d years old again.\n", cur_name[i], cur_age[i]);
  }

  printf("---\n");

  // fourth way with pointers in a stupid complex way
  for (cur_name = names, cur_age = ages; (cur_age - ages) < count; cur_name++, cur_age++) {
    printf("%s lived %d years thus far.\n", *cur_name, *cur_age);
  }

  return 0;
}
