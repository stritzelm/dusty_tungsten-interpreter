/*
created by dillon lanier 11-5-2019 for programming languages with dave musicant
* /

#include "tokenizer.h"
#include "value.h"
#include "talloc.h"
#include "linkedlist.h"
#include "interpreter.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>



Frame *makeFrame(Frame *parent) {
    Frame *frame = talloc(sizeof(Frame));
    (*frame).bindings = makeNull();
    (*frame).parent = parent;
    return frame;
}

Value *evalIf(Value *args, Frame *frame) {
    Value *test = car(args);
    Value *expressions = cdr(args);
    //int testresult = eval(test, frame);   //evaluate the test, get back a int bool.
    //printf("bool type %u\n",test->type );

    //if(test->type == BOOL_TYPE){
        if (test->b == 1) {
        //printf("IN THE TRUE CASE OF EVAL\n");
        //printf("%u type \n",(car(expressions))->type );
        return eval(car(expressions), frame);       //true expression
        } else {
        //printf("IN THE FALSE CASE OF EVAL IF\n");
        return eval(car(cdr(expressions)), frame);      //false expression
        }
    // }else{
    //     printf("evaluation error: condition is not a bool type\n");
    //     texit(1);
    //     return NULL;
    // }
    
}

Value *evalLet(Value *args, Frame *frame) {
    Frame *subFrame = makeFrame(frame);
    Value *bindings = car(args);
    Value *tempBinding;

    if((cdr(args))->type == NULL_TYPE){
            printf("evaluation error: no body expression\n");
            texit(1);
            return NULL;
    }

    else{

        Value *expressions = car(cdr(args));
        (subFrame->bindings)->type = CONS_TYPE;
        Value *symbol;
        Value *bindingsExp;

        //printf("type %u\n",(car(car(bindings)))->type );
        while(bindings->type != NULL_TYPE){ 
            symbol = car(car(bindings)); //takes symbol of binding
            
            bindingsExp = car(cdr(car(bindings)));

        if(symbol->type != SYMBOL_TYPE){

            tempBinding = cons(symbol, eval(bindingsExp, subFrame));  //create binding  
        } else{
            printf("evaluation error: variable is not a symbol\n");
            texit(1);
        }

        bindings = cdr(bindings); 
        subFrame->bindings = cons(tempBinding, subFrame->bindings); //adds binding to subframe
        }
        subFrame->parent = frame;
        return eval(expressions,subFrame);
    }      

}



Value *lookUpSymbol(Value *tree, Frame *frame) {
    Frame *currFrame = frame;
    Value *currBinding;
    Value *nextBinding = frame->bindings ;

    while(currFrame != NULL){ 
        currBinding = frame->bindings; //iterates through frame

        while(nextBinding->type != NULL_TYPE){ //checks in current frame for binding match
            currBinding = car(nextBinding);
            if(tree ==currBinding){
                printf("hit\n");
                return car(cdr(nextBinding));
            }
            nextBinding = cdr(nextBinding);
        }
        currFrame = currFrame->parent;
    }
    printf("ERROR: variable is not bound to anything\n");
    texit(1);
    return NULL;
}


Value *eval(Value *tree, Frame *frame) {

    Value *result = makeNull();
    //printf("HERE:%\n", tree);
    switch (tree->type) {
        case INT_TYPE:
            //printf("%i\n", tree->i);
            return tree;
            break;
        case BOOL_TYPE: 
            //printf("%i\n", tree->b);
           return tree;
           break;
         case SYMBOL_TYPE:
         printf("hit\n");
            return eval(lookUpSymbol(tree, frame), frame);
            break;
         case STR_TYPE:
            return tree;
            break;
        case CONS_TYPE: {
            Value *first = car(tree);
            //printf("THIS:%s\n", (*first).symbol);
            // if ((*first).type == NULL_TYPE) {
            //  return first;
            // }
            Value *args = cdr(tree);
            //eval(car(tree), frame);
            //eval(cdr(tree), frame);
            //Sanity and error checking on first...
            //printf("HERE WE ARE\n");
            if (strcmp(first->symbol,"if") == 0) {
                result = evalIf(args,frame);
            }
            else if (!strcmp(first->symbol,"let")) {
                result = evalLet(args,frame);
             }
            else {
                printf("ERROR: invald expression to evaluate\n");
            }
            break;
        }
        default:
            //printf("ERROR: type not accounted for by switch case!\n");
            //printf("%s\n", tree->symbol);
            return tree;
    }
    return result;
}
//helper method to print results?
void printHelper(Value* tree){
    if (tree->type == INT_TYPE) {
        printf("%i\n", tree->i);
    } else if (tree->type == STR_TYPE) {
        printf("%s\n", tree->s);
    } else if (tree->type == SYMBOL_TYPE) {
        printf("%s\n", tree->symbol);
    } else{
        printf("nothing \n");
    }
        

       
}
void interpret(Value *tree) {

    Frame *globalFrame = makeFrame(NULL);
    tree = car(tree);
    Value *results = eval(tree, globalFrame);
    printHelper(results);

}