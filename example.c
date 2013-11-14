struct A {
   int x;
   double y;
} a;

typedef struct A B;

int f(struct A a);

int f(struct A a) {
   return a.x + (int)a.y;
}

int main(int argc, char **argv) {
   a.x = 3;
   a.y = 2.2;
   B const* b;
   a.x += 8;
   struct A c = (struct A)(*b);
   struct A d = { .x = 3, .y = b->y };
   return f(a);
}
