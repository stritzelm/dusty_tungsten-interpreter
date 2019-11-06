#ifndef _VALUE
#define _VALUE

// OPENBRACKET_TYPE and CLOSEBRACKET_TYPE are only for the bonus
typedef enum {INT_TYPE,DOUBLE_TYPE,STR_TYPE,CONS_TYPE,NULL_TYPE,PTR_TYPE,
              OPEN_TYPE,CLOSE_TYPE,BOOL_TYPE,SYMBOL_TYPE} valueType;

struct Value {
    
    valueType type;
    union {
        int i;
        float d;
        char *s;
        void *p;
        struct ConsCell {
            struct Value *car;
            struct Value *cdr;
        } c;
        char open;
        char close;
        int b;
        char *symbol;
    };
};


// Where to you store a boolean in the union? It's up to you, but I used an int
// (0 for false, 1 for true) in my own solution.

typedef struct Value Value;

#endif
