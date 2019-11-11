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

Frame *topFrame;

/*
helper method that prints a space for output formatting
*/
void printSpace(Value *prev) {
    if ((*prev).type != NULL_TYPE && (*prev).type != OPEN_TYPE) {
        printf(" ");
    }
}

/*
* Creates a VOID_TYPE value and returns a pointer to it
*/
Value *makeVoidValue(){
    Value *voidValue = makeNull();
    voidValue->type = VOID_TYPE;
    return voidValue;
}

/*
Helper function for displaying a parse tree to the screen.
*/
void printInterpreterHelper(Value *tree, Value *prev, int empty) {

    switch((*tree).type) {
        case BOOL_TYPE:
            printSpace(prev);
            printf("%i", (*tree).i);
            (*prev).type = BOOL_TYPE;
            break;
        case SYMBOL_TYPE:
            printSpace(prev);
            printf("%s", (*tree).s);
            (*prev).type = SYMBOL_TYPE;
            break;
        case INT_TYPE:
            
            printSpace(prev);
            printf("%i", (*tree).i);
            (*prev).type = INT_TYPE;
            break;
        case DOUBLE_TYPE:
            printSpace(prev);
            printf("%f", (*tree).d);
            (*prev).type = DOUBLE_TYPE;
            break;
        case STR_TYPE:
            printSpace(prev);
            printf("%s", (*tree).s);
            (*prev).type = STR_TYPE;
            break;
        case PTR_TYPE:
            printSpace(prev);
            printf("%p", (*tree).p);
            (*prev).type = PTR_TYPE;
            break;
        case CONS_TYPE:
            if (car(tree)->type == CONS_TYPE) {
                printSpace(prev);
                printf("(");
                (*prev).type = OPEN_TYPE;
            }
            printInterpreterHelper(car(tree), prev, 1);
            if (car(tree)->type == CONS_TYPE) {
                printf(")");
            }
            printInterpreterHelper(cdr(tree), prev, 0);  
            break;
        case NULL_TYPE:
            if (empty == 1) {
                printf("()");
            }
            (*prev).type = CLOSE_TYPE;
            break;
        case VOID_TYPE:
            break;
        default:
            printf("ERROR: Unaccounted for type\n");
    }   
}


/*
This function displays a parse tree to the screen.
feeds the tree, the previously printed thing, and a bool to a helper method
 */
void printInterpreter(Value *tree) {

    Value *prev = makeNull();
    printInterpreterHelper(tree, prev, 1);

}


Frame *makeFrame(Frame *parent) {
    Frame *frame = talloc(sizeof(Frame));
    (*frame).bindings = makeNull();
    (*frame).parent = parent;
    return frame;
}

Value *makeClosure(Value *args, Frame *frame){
    Value *value = talloc(sizeof(Value));
    value->type = CLOSURE_TYPE;
    (*value).cl.paramNames = car(args);
    (*value).cl.functionCode = car(cdr(args));
    (*value).cl.frame = frame;
    return value;
}

//creates a new binding of var to the value of expr
//modifies current environment frame
//need to have a top or global frame, contains binding of variables created using define

Value *evalDefine(Value *args, Frame *frame) {
    
    //iterate to parent frame
    Value *variable;
    Value *expression;
    Frame *currFrame = topFrame;
    Value *tempBinding;
    Value *voidValue = makeVoidValue();

    currFrame = topFrame; //assignment currFrame to top frame for bindings
    variable = car(args);
    
    expression = eval(car(cdr(args)), currFrame);
    
    tempBinding = cons(variable, expression); //creates binding of variable and expression

    currFrame->bindings = cons(tempBinding, currFrame->bindings); //add binding to global frame

    return voidValue;
}   



//create closure
Value *evalLambda(Value *args, Frame *frame){
    Value *closure = makeClosure(args, frame);
    //Frame *lambdaFrame = makeFrame((*closure).cl.frame);
    //Value *bindings = car
    
    return closure;
    
}




//lambda creates a frame
//parent is frame pointed to by environment
//create local variables (bindings to match the parameters)
// it executes (evals) the cdr in the body of the closure
Value *apply(Value *function, Value *args) {
    

    
    Frame *frame = makeFrame(function->cl.frame);
    Value *params = function->cl.paramNames;
    Value *current = args;
    Value *argName = function->cl.paramNames;
    
    // Go through all the actual argument value
    while(current->type != NULL_TYPE){

        Value *vcurrent = car(current);
        Value *vargName = car(argName);
        // Bind the paraname with the actual value
        Value *result = eval(vcurrent, frame->parent);
        
        Value *bind = makeNull();
        
        bind = cons(result,bind);
        bind = cons(vargName,bind);
        frame->bindings = cons(bind, frame->bindings);
        
        current = cdr(current);
        argName = cdr(argName);
    }
    

    Value *last = car(reverse(function->cl.functionCode));
    
    return eval(last, frame);
    
}

Value *evalEach(args, frame) {
    
    Value *returnList = makeNull();
    while (cdr(args)->type != NULL_TYPE){
        returnList =  cons(eval(car(args), frame), returnList);
        args = cdr(args);
    }
    return returnList;
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
        return NULL;
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
                    
                    tempBinding = cons(symbol, eval(bindingsExp, frame));  //create binding  
                 }else{
                    printf("evaluation error: variable is not a symbol\n");
                    texit(1);
                }
            } else{
                printf("evaluation error: invalid list of bindings\n");
                texit(1);
            }
            bindings = cdr(bindings); 
            subFrame->bindings = cons(tempBinding, subFrame->bindings); //adds binding to subframe
        }

           
        
        //subFrame->parent = frame;
        while (cdr(expressions)->type != NULL_TYPE) {
            eval(car(expressions), subFrame);
            expressions = cdr(expressions);
        }
        // printf("ABOUT TO PRINT BINDINGS\n");
        //printf("EXPRESSIONS: %s\n", expressions->s);
        // printInterpreterHelper(subFrame->bindings);
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
        nextBinding = currFrame->bindings; //iterates through frame

        while(nextBinding->type != NULL_TYPE){ //checks in current frame for binding match
            
            
            currBinding = car(nextBinding);
            if(!strcmp(tree->s, car(currBinding)->s)){
                
                return cdr(currBinding);
            }
            nextBinding = cdr(nextBinding);
        }
        currFrame = currFrame->parent;
    }
    printf("%s\n", tree->s);  // PRINTING OUT ADD-ONE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                            // THE BUG: its not being found in the frame!
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
            } else if (!strcmp(first->s,"let")) {
                result = evalLet(args,frame);
            } else if (!strcmp(first->s, "quote")) {
                //printf("IN THE QUOTE CASE\n");
                result = evalQuote(args);
            } else if (!strcmp(first->s, "define")){

                result = evalDefine(args, frame);

            } else if (!strcmp(first->s, "lambda")){
                result = evalLambda(args, frame);
                
                //printInterpreter((*result).cl.functionCode);
                // printf(car((*result).cl.paramNames));

            } else {
                
              //  if (evaledOperator->type == CLOSURE_TYPE) {
                    Value *evaledOperator = eval(first, frame);
                    Value *evaledArgs = evalEach(args, frame);
                  //  return apply(evaledOperator,evaledArgs);
             //   }
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

void interpret(Value *tree) {

    topFrame = makeFrame(NULL);
    //tree = car(tree);
    Value *results;
    while(tree->type != NULL_TYPE){
        results = eval(car(tree), topFrame);
        printInterpreter(results);
        printf("\n");
        tree = cdr(tree);
    }

}


