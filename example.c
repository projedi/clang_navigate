struct A {
   int x;
   double y;
} a;

typedef struct A B;

int f(A a);

int f(A a) {
   return a.x + (int)a.y;
}

int main(int argc, char **argv) {
   a.x = 3;
   a.y = 2.2;
   B const* b;
   b.x += 8;
   A c = (A)(*b);
   A d = { .x = 3, .y = b.y };
   return f(a);
}
