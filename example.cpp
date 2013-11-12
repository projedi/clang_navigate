namespace nmx {
struct C {
   C(int);
   C(C const&);
   virtual ~C() { }
};

}

const int defsize = 10;

template<class T, int size = defsize>
struct W { };

template<>
struct W<nmx::C, 20> { };

template<class Y>
struct W<Y, 30> { };

nmx::C::C(int) { }

namespace nmy = nmx;

struct A : nmy::C {
   A() : nmy::C(0) { }
   int x;
   double y;
} a;

using namespace nmy;

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
