#include "kernel_shared.h"
#include "kernel.h"

void Test_strlen() {
  char* p;

  p = "a";
  Assert(strlen(p) == 1);
  p = "four";
  Assert(strlen(p) == 4);
  p = "asdf1234asdf12345";
  Assert(strlen(p) == 17);
}

void Test_strcmp() {
  char* p;
  char* q;

  p = "first";
  q = "second";
  Assert(strcmp(p, q) != 0);
  q = "first123";
  Assert(strcmp(p, q) != 0);
  q = "first";
  Assert(strcmp(p, q) == 0);
  p = "dev";
  q = "d9v";
  Assert(strcmp(p, q) != 0);
  Assert(strcmp(NULL, NULL) == 0);
}

void Test_strcpy() {
  char* p;
  char buf[64];

  p = "test";
  Assert(strcpy(buf, p, sizeof(buf)) == S_OK);
  Assert(buf[4] == 0);
  Assert(strcpy(buf, NULL, sizeof(buf)) == S_FAIL);
  Assert(strcpy(NULL, buf, sizeof(buf)) == S_FAIL);
  Assert(strlen(buf) == strlen(p));
  Assert(strcmp(buf, p) == 0);
  Assert(strcmp(buf, "test") == 0);
  Assert(strcmp(buf, "test2") == 1);
}

void Test_sprintf() {
  char buf[64];
  unsigned int u = 99;
  Assert(sprintf(sizeof(buf), buf, "asdf") == S_OK);
  Assert(strcmp(buf, "asdf") == 0);

  Assert(sprintf(sizeof(buf), buf, "%s", "hello") == S_OK);
  Assert(strcmp(buf, "hello") == 0);

  Assert(sprintf(sizeof(buf), buf, "%d", 12345) == S_OK);
  Assert(strcmp(buf, "12345") == 0);

  Assert(sprintf(sizeof(buf), buf, "hello %s goodbye %d world", "asdf", 1234) ==
         S_OK);
  Assert(strcmp(buf, "hello asdf goodbye 1234 world") == 0);

  Assert(sprintf(sizeof(buf), buf, "%d", 0) == S_OK);
  Assert(strcmp(buf, "0") == 0);

  Assert(sprintf(sizeof(buf), buf, "%c", 'X') == S_OK);
  Assert(strcmp(buf, "X") == 0);

  Assert(sprintf(sizeof(buf), buf, "%u", 12345) == S_OK);
  Assert(strcmp(buf, "12345") == 0);

  Assert(sprintf(sizeof(buf), buf, "Alloc %u\n", u) == S_OK);
  Assert(strcmp(buf, "Alloc 99\n") == 0);
}

void Test_strstr() {}

void Test_isalpha() {
  for (int i = 0; i < 26; ++i) {
    Assert(isalpha(i + 'a'));
    Assert(isalpha(i + 'A'));
  }
  Assert(!isalpha(1));
  Assert(!isalpha('$'));
  Assert(!isalpha('.'));
  Assert(!isalpha(NULL));
  Assert(!isalpha(0));
}

void Test_tolower() {
  for (int i = 0; i < 26; ++i) {
    Assert(tolower(i + 'A') == i + 'a');
    Assert(tolower(i + 'a') == i + 'a');
  }
  Assert(tolower(999) == 999);
  Assert(tolower('$') == '$');
}

void Test_strtok() {
  char* p = strtok("hello world abc", ' ');
  Assert(!strcmp(p, "hello"));

  p = strtok(NULL, ' ');
  Assert(!strcmp(p, "world"));

  p = strtok(NULL, ' ');
  Assert(!strcmp(p, "abc"));

  p = strtok(NULL, ' ');
  Assert(p == NULL);

  p = strtok("111x222", 'x');
  Assert(!strcmp(p, "111"));

  p = strtok(NULL, ' ');
  Assert(!strcmp(p, "222"));

  p = strtok(NULL, ' ');
  Assert(p == NULL);

  p = strtok(NULL, '1');
  Assert(p == NULL);
}


void Test_atoi() {
  Assert(atoi(NULL) == 0);
  Assert(atoi("abcd") == 0);
  Assert(atoi("3") == 3);
  Assert(atoi(" 3") == 3);
  Assert(atoi("35") == 35);
  Assert(atoi("3598") == 3598);
  Assert(atoi("-3598") == -3598);
}

void Test_String() {
  Test_strlen();
  Test_strcmp();
  Test_strcpy();
  Test_sprintf();
  Test_strstr();
  Test_isalpha();
  Test_tolower();
  Test_strtok();
  Test_atoi();
  return;
}
