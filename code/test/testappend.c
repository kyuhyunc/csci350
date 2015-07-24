/*  testappend.c
*/

#include "syscall.h"

char buffer[100];
char str[40];
int num;

int main() {
    str[0] = 'P';
    str[1] = 'a';
    str[2] = 's';
    str[3] = 's';
    str[4] = 'e';
    str[5] = 'n';
    str[6] = 'g';
    str[7] = 'e';
    str[8] = 'r';
    str[9] = '\0';
    num = 5;
    ConcatNum2String(str, sizeof(str), num, buffer);

    Printf0(buffer, sizeof(buffer));

    Exit(0);
}
