#define RETURN_IF_ERROR(expr)        \
    do {                             \
        int __ret = (expr);          \
        if (__ret < 0) return __ret;\
    } while (0)

#define CHECK(cond, handler)   \
    do {                       \
        if ((cond) < 0) {         \
            handler();         \
            return -1;         \
        }                      \
    } while (0)
