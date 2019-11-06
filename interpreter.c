/*
created by dillon lanier 11-5-2019 for programming languages with dave musicant
*/

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
    Value *test = eval(car(args), frame);
    Value *expressions = cdr(args);
    if(test->type == BOOL_TYPE){
        if (test->i == 1) {
            return eval(car(expressions), frame);       //true expression
        } else {
            return eval(car(cdr(expressions)), frame);      //false expression
        }
    } else {
        printf("evaluation error: condition is not a boolean\n");
        texit(1);
    }
    
}

Value *evalLet(Value *args, Frame *frame) {
    Frame *subFrame = makeFrame(frame);
    Value *bindings = car(args);
    Value *tempBinding;

    if(bindings->type != CONS_TYPE){
        printf("evaluation error: list of bindings is not nested\n");
        texit(1);
    }

    if((cdr(args))->type == NULL_TYPE && cdr(args)->type == CONS_TYPE){
        printf("evaluation error: no body expression\n");
        texit(1);
        return NULL;
    }
    else{    
        Value *expressions = car(cdr(args));
        Value *symbol;
        Value *bindingsExp;

        //printf("type %u\n",(car(car(bindings)))->type );
        if(bindings->type != CONS_TYPE){
            printf("evaluation error: improper binding\n");
            texit(1);
        }
        while(bindings->type != NULL_TYPE){ 
            if(car(bindings)->type == CONS_TYPE && cdr(car(bindings))->type == CONS_TYPE && cdr(cdr(car(bindings)))->type == NULL_TYPE){
                symbol = car(car(bindings)); //takes symbol of binding
                bindingsExp = car(cdr(car(bindings)));
        
                if(symbol->type == SYMBOL_TYPE){
                    tempBinding = cons(symbol, eval(bindingsExp, subFrame));  //create binding  
                 }else{
                    printf("evaluation error: variable is not a symbol\n");
                    texit(1);
                }
            }else{
                printf("evaluation error: invalid list of bindings\n");
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
    if(tree->type == SYMBOL_TYPE){
    
    }
    while(currFrame != NULL){ 
        currBinding = frame->bindings; //iterates through frame

        while(nextBinding->type != NULL_TYPE){ //checks in current frame for binding match
            currBinding = car(nextBinding);
            if(!strcmp(tree->s, car(currBinding)->s)){
                return cdr(currBinding);
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
            if (strcmp(first->s,"if") == 0) {
                result = evalIf(args,frame);
            }
            else if (!strcmp(first->s,"let")) {
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

        
void printHelper(Value *list) {
    
    switch ((*list).type) {
        case INT_TYPE:
            printf("%i: Integer \n ", (*list).i);
            break;
        case DOUBLE_TYPE:
            printf("%f: Double \n", (*list).d);
            break;
        case STR_TYPE:
            printf("%s: String \n", (*list).s);
            break;
        case NULL_TYPE:
            printf("NULL \n");
            break;
        case CONS_TYPE:
            printHelper((*list).c.car);
            printHelper((*list).c.cdr); 
            break;
        case PTR_TYPE:
            printf("%p: pointer \n ", (*list).p);
            break;
        case OPEN_TYPE:
            printf("%c: open \n", (*list).s);
            break;
        case CLOSE_TYPE:
            printf("%c: Close \n", (*list).s);
            break;
        case BOOL_TYPE:
            printf("%i:Bool \n", (*list).i);
            break;
        case SYMBOL_TYPE:
               printf("%s: Symbol \n", (*list).s);
            break;
            
    }
}
       

void interpret(Value *tree) {

    Frame *globalFrame = makeFrame(NULL);
    //tree = car(tree);
    Value *results;
    while(tree->type != NULL_TYPE){
        results = eval(car(tree), globalFrame);
        printHelper(results);
        tree = cdr(tree);
    }
    

}