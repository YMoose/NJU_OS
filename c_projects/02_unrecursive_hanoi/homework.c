#include <stdio.h>
#include <assert.h>

typedef struct {
  int pc, n;
  char from, to, via;
} Frame;

#define call(...) ({ *(++top) = (Frame) { .pc = 0, __VA_ARGS__ }; })
#define ret()     ({ top--; })
#define goto(loc) ({ f->pc = (loc) - 1; })

int g(int n);
int f(int n) {
  if (n <= 1) return 1;
  return f(n - 1) + g(n - 2);
}

int g(int n) {
  if (n <= 1) return 1;
  return f(n + 1) + g(n - 1);
}

typedef struct 
{
    int pc;
    int cur_ret;
    int n;
    int is_f;
    int* ret;
}FGFrame; 

#define FUNC_F 1
#define FUNC_G 0

#define fgcall(...) ({ *(++top) = (FGFrame) { .pc = 0, .cur_ret = 0, __VA_ARGS__ }; })
#define fgret()     ({ top--; })
#define fggoto(loc) ({ f->pc = (loc) - 1; })

int unrecursive_fg_func(int n, int is_f)
{
    FGFrame stk[64], *top = stk - 1;
    int ret = 0;
    fgcall(n, is_f, &ret);
    for (FGFrame *f; (f = top) >= stk; f->pc++) {
        n = f->n; 
        is_f = f->is_f;
        switch(f->pc) {
          case 0: if (n <= 1) { f->cur_ret = 1; goto(3); } break;
          case 1: fgcall((is_f? n - 1: n + 1), FUNC_F, &(f->cur_ret)); break;
          case 2: fgcall((is_f? n - 2: n - 1), FUNC_G, &(f->cur_ret)); break;
          case 3: *(f->ret) += f->cur_ret ;ret(); break;
          default: assert(0);
        }
    }
    return ret;
}

void hanoi(int n, char from, char to, char via) {
  Frame stk[64], *top = stk - 1;
  call(n, from, to, via);
  for (Frame *f; (f = top) >= stk; f->pc++) {
    n = f->n; from = f->from; to = f->to; via = f->via;
    switch (f->pc) {
      case 0: if (n == 1) { printf("%c -> %c\n", from, to); goto(4); } break;
      case 1: call(n - 1, from, via, to);   break;
      case 2: call(    1, from, to,  via);  break;
      case 3: call(n - 1, via,  to,  from); break;
      case 4: ret();                        break;
      default: assert(0);
    }
  }
}

int main()
{
    int n =9;
    printf("my result = %d, recur result = %d\n", unrecursive_fg_func(n, FUNC_F), f(n));
    return 0;
}