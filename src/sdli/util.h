#ifndef SDLI_UTIL_H
#define SDLI_UTIL_H

#define UNUSED_4(arg, ...) (void)arg
#define UNUSED_3(arg, ...) (void)arg, UNUSED_4(__VA_ARGS__, 0)
#define UNUSED_2(arg, ...) (void)arg, UNUSED_3(__VA_ARGS__, 0)
#define UNUSED_1(arg, ...) (void)arg, UNUSED_2(__VA_ARGS__, 0)
#define UNUSED(...) (UNUSED_1(__VA_ARGS__, 0))

#endif // SDLI_UTIL_H
