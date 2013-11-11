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
   B b;
   b.x += 8;
   return f(a);
}
