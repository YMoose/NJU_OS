#include <am.h>
#include <stdarg.h>

void print(const char *s, ...) {
    va_list ap;
    va_start(ap, s);
    while (s) {
        for (; *s; s ++) putch(*s);
        s = va_arg(ap, const char *);
    }
    va_end(ap);
}

int main(const char *args) {
    print("\"", args, "\"", " from " __ISA__ " program!\n", NULL);
    return 0;
}