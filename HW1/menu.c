#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct fun_desc {
  char *name;
  char (*fun)(char);
};
// MAP FUNCTION AS REQUEST
char* map(char *array, int array_length, char (*f)(char)) {
  char* mapped_array = (char*)malloc(array_length * sizeof(char));
  for (int i = 0; i < array_length; i++) {
    mapped_array[i] = f(array[i]);
  }
  return mapped_array;
}
//MY GET FUNCTION
char my_get(char c) {
  return fgetc(stdin);
}

char cprt(char c) {
  if (c >= 0x20 && c <= 0x7E) {
    printf("%c\n", c);
  } else {
    printf(".\n");
  }
  return c;
}
// ENCRYPT FUNCTION
char encrypt(char c) {
  if (c >= 0x20 && c <= 0x4E) {
    c = c + 0x20;
  }
  return c;
}

char decrypt(char c) {
  if (c <= 0x7E && c >= 0x40) {
    c = c - 0x20;
  }
  return c;
}
// PRINT
char xoprt(char c) {
  if (c <= 0x7E && c >= 0x20) {
    printf("%x\t%o\n", c, c);
  } else {
    printf(".\t.\n");
  }
  return c;
}
// PRINTING MENU
void PrintTheMenu(struct fun_desc *menu, int menuSize) {
  printf("Select operation from the following menu (ctrl^D for exit):\n");
  for (int i = 0; i < menuSize; i++) {
    printf("%d) %s\n", i, menu[i].name);
  }
  printf("option: ");
}

int main(int argc, char **argv) {
  char user_array[200];
  char* carray = calloc(5, sizeof(char));

  struct fun_desc menu[] = {
    { "RETURN STRING", my_get }, { "PRINT STR", cprt }, { "EncryptION", encrypt }, { "DecryptION", decrypt }, { "Prnt Hexadecimal and Oct", xoprt },
    { NULL, NULL }
  };
  int The_menu_langth = sizeof(menu) / sizeof(struct fun_desc) - 1;

  while (1) {
    PrintTheMenu(menu, The_menu_langth);
    if (fgets(user_array, 200, stdin) == NULL) break;

    int MENU_OPT;
    if (sscanf(user_array, "%d\n", &MENU_OPT) != 1) continue;

    if (MENU_OPT < 0 || MENU_OPT >= The_menu_langth) {
      printf("Not within bounds\n");
      break;
    }
    printf("Within bounds\n");

    char* VAL = carray;
    carray = map(carray, 5, menu[MENU_OPT].fun);
    free(VAL);

    printf("DONE.\n");
  }

  free(carray);
  return 0;
}
