#include "example.h"

typedef struct A B;

int f(struct A a);

struct A g() {
   B b = { 3, 0.5 };
   return (struct A) b;
}

int f(struct A a) {
   return a.x + (int)a.y;
}

int main(int argc, char **argv) {
label:   a.x = 3;
   a.y = 2.2;
   B const* b;
   a.x += 8;
   struct A c = (struct A)(*b);
   struct A d = { .x = 3, .y = b->y };
   argc += 5;
   goto label;
   return f(a);
}
