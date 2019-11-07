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

int quote = 0; // could pass this into eval each time, if it's 1 means
//there is a quote for the cons type bc problem is cons is (a b c). So if quote 
//is true we can just return tree and print that?


/*
helper method that prints a space for output formatting
*/
void printSpace1(Value *prev) {
    if ((*prev).type != NULL_TYPE && (*prev).type != OPEN_TYPE) {
        printf(" ");
    }
}

/*
Helper function for displaying a parse tree to the screen.
*/
void printTreeHelper1(Value *tree, Value *prev, int empty) {

    switch((*tree).type) {
        case BOOL_TYPE:
            printSpace1(prev);
            printf("%i", (*tree).i);
            (*prev).type = BOOL_TYPE;
            break;
        case SYMBOL_TYPE:
            printSpace1(prev);
            printf("%s", (*tree).s);
            (*prev).type = SYMBOL_TYPE;
            break;
        case INT_TYPE:
            printSpace1(prev);
            printf("%i", (*tree).i);
            (*prev).type = INT_TYPE;
            break;
        case DOUBLE_TYPE:
            printSpace1(prev);
            printf("%f", (*tree).d);
            (*prev).type = DOUBLE_TYPE;
            break;
        case STR_TYPE:
            printSpace1(prev);
            printf("%s", (*tree).s);
            (*prev).type = STR_TYPE;
            break;
        case PTR_TYPE:
            printSpace1(prev);
            printf("%p", (*tree).p);
            (*prev).type = PTR_TYPE;
            break;
        case CONS_TYPE:
            if (car(tree)->type == CONS_TYPE) {
                printSpace1(prev);
                printf("(");
                (*prev).type = OPEN_TYPE;
            }
            printTreeHelper1(car(tree), prev, 1);
            //printSpace1(prev);
            if (car(tree)->type == CONS_TYPE) {
                printf(")");
            }
            printTreeHelper1(cdr(tree), prev, 0);  
            break;
        case NULL_TYPE:
            if (empty == 1) {
                printf("()");
            }
            (*prev).type = CLOSE_TYPE;
            break;
        default:
            printf("ERROR: Unaccounted for type\n");
    }   
}


/*
This function displays a parse tree to the screen.
feeds the tree, the previously printed thing, and a bool to a helper method
 */
void printTree1(Value *tree) {

    Value *prev = makeNull();
    printTreeHelper1(tree, prev, 1);

}


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

Value *evalQuote(Value *args) {
    //just want to return same stuff without the quote in front.
    return args;
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
        Value *expressions = cdr(args);
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
        while (cdr(expressions)->type != NULL_TYPE) {
            eval(car(expressions), subFrame);
            expressions = cdr(expressions);
        }
        // printf("ABOUT TO PRINT BINDINGS\n");
        printf("EXPRESSIONS: %s\n", expressions->s);
        // printTree1(subFrame->bindings);
        // printf("\n");
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
    //printf("eval called:\n", tree);
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
         
            return lookUpSymbol(tree, frame);
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
            else if (!strcmp(first->s, "quote")) {
                //printf("IN THE QUOTE CASE\n");
                result = evalQuote(args);
            } else {

                result = eval(first, frame);
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

/*     
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
*/

void interpret(Value *tree) {

    Frame *globalFrame = makeFrame(NULL);
    //tree = car(tree);
    Value *results;
    while(tree->type != NULL_TYPE){
        results = eval(car(tree), globalFrame);
        printTree1(results);
        tree = cdr(tree);
    }
    

}


