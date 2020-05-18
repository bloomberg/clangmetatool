extern int vararg_func(int x, ...);
extern int func(int x, int y, int z);

extern char _GLOBALARRAY[256];

#define M_ARRAY _GLOBALARRAY

#define BEFORE_EXPR(x) 1 + M_ARRAY[x]

#define NESTED2 M_ARRAY
#define NESTED(x) NESTED2[x]

#define EXPR(x) M_ARRAY[x]
#define EXTRA_CHARS(x) ( (        \
                        (( M_ARRAY  \
                           [ x ] ) \
                       ) ))

#define AFTER_EXPR(x) M_ARRAY[x] + 1

#define INNER_MACRO(x, y) (x + y)
#define OUTER_MACRO(x) (INNER_MACRO(x, M_ARRAY[0]))

#define ONE 1
#define FORWARD_TO_EXPR(x) EXPR(x)
#define ALIAS_FOR_EXPR EXPR(1)
#define M_ARRAY_SECOND M_ARRAY[ONE]

#define NONVARARG_TO_VARARG(x) vararg_func(x)
#define VARARG_TO_VARARG_DIRECT(...) vararg_func(__VA_ARGS__)
#define VARARG_TO_VARARG_PASTED(x, y, ...) vararg_func(x, y, ##__VA_ARGS__)
#define VARARG_TO_NONVARARG_DIRECT(...) func(__VA_ARGS__, 0, 1)
#define VARARG_TO_NONVARARG_PASTED(...) func(0 ## 1, ##__VA_ARGS__, 1)

#define MACRO_AS_ARG(array_like_macro_name) \
  array_like_macro_name[0] + array_like_macro_name[1] + array_like_macro_name[2]


int positive()
{
  return
      BEFORE_EXPR(0)
    + AFTER_EXPR(4)
    + OUTER_MACRO(6)
    + VARARG_TO_VARARG_PASTED(0, 1, M_ARRAY[1])
    + VARARG_TO_NONVARARG_PASTED(M_ARRAY[1]);
}

int negative() {
  return
      NESTED(1)
    + EXPR(0)
    + EXTRA_CHARS(2)
    + INNER_MACRO(5, M_ARRAY[0])
    + FORWARD_TO_EXPR(0)
    + ALIAS_FOR_EXPR
    + M_ARRAY_SECOND
    + NONVARARG_TO_VARARG(M_ARRAY[1])
    + VARARG_TO_VARARG_DIRECT(0, M_ARRAY[1])
    + VARARG_TO_NONVARARG_DIRECT(M_ARRAY[1]);

}

int falseNegative() {
  return MACRO_AS_ARG(M_ARRAY);
}
