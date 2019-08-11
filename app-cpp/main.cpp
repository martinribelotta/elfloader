#include "sysent.h"

class testclass {
public:
  // Default Constructor
  int i;
  testclass() {
    i = 3;
    syscalls.printf("Construct world\n");
  }
  ~testclass() {
    syscalls.printf("Destruct world\n");
  }
  void banner(void) {
    syscalls.printf("Hello banner (%d)\n", i);
  }
  void doit(void) {
    i++;
    syscalls.printf("Doing it (%d)\n", i);
  }
};

static testclass test_instance;

extern "C" {

void _start(void) {
  syscalls.printf("Hello main\n");
  test_instance.banner();
  syscalls.printf("Bye main\n");
}

void doit(void) {
  test_instance.doit();
}

}

