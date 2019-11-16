/*
created by Dillon Lanier, Tony Ngo, Matt Stritzel 11-5-2019 for programming languages with dave musicant
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
Value *makeVoidValue() {
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
            if(tree->i == 1){
                printf("#t");
            } else{
                printf("#f");
            }
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
            printf("\"%s\"", (*tree).s);
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
    Value *tempParam =  (*value).cl.paramNames;
    while(tempParam->type != NULL_TYPE){
        if(car(tempParam)->type != SYMBOL_TYPE){
            printf("error: param is not a symbol\n");
            texit(1);
        }
        else{
            tempParam = cdr(tempParam);
        }
    }
    if((*value).cl.paramNames->type != SYMBOL_TYPE)

    (*value).cl.functionCode = car(cdr(args));
    (*value).cl.frame = frame;
    return value;
}

//creates a new binding of var to the value of expr
//modifies current environment frame
//need to have a top or global frame, contains binding of variables created using define

Value *evalDefine(Value *args, Frame *frame) {
    //using topFrame which is global frame
    //name of the defined thing
    Value *variable = car(args);
    if(variable->type != SYMBOL_TYPE){
        printf("error: variable is not a symbol\n");
            texit(1);
    }

    Value *voidValue = makeVoidValue();
    //check to make sure num params = num of variables used
    if(cdr(args)->type == NULL_TYPE){
        printf("evaluation error: reference without definition\n");
        texit(1);
    }
    if(cdr(cdr(args))->type != NULL_TYPE){
        printf("evaluation error: length of arguements for define is longer than 2\n");
        texit(1);
    }


    //expression is the thing the name variable points to
    Value *expression = eval(car(cdr(args)), topFrame);
    Value *tempBinding = cons(variable, expression); //creates binding of variable and expression
    topFrame->bindings = cons(tempBinding, topFrame->bindings); //add binding to global frame

    return voidValue;
}   



//create closure
Value *evalLambda(Value *args, Frame *frame) {
    Value *closure = makeClosure(args, frame);
    return closure;
}


//lambda creates a frame
//parent is frame pointed to by environment
//create local variables (bindings to match the parameters)
// it executes (evals) the cdr in the body of the closure
Value *apply(Value *function, Value *args) {
    
    Frame *frame = makeFrame(function->cl.frame);
    Value *params = function->cl.paramNames;
    Value *body = (*function).cl.functionCode;

    // Go through all the actual argument value
    while(args->type != NULL_TYPE){
        frame->bindings = cons(cons(car(params), car(args)), frame->bindings);

        args = cdr(args);
        params = cdr(params);
        if(args->type != NULL_TYPE && params->type == NULL_TYPE){
            printf("error: more arguements passed than number of params\n");
            texit(1);
        }
        if(args->type == NULL_TYPE && params->type != NULL_TYPE){
            printf("error: number of arguements passed is less than number of parameters needed\n");
            texit(1);
        }
    }
    //printInterpreter(frame->bindings);
    //frame->bindings = new_bindings;    
    return eval(body, frame);
}


Value *evalEach(Value *args, Frame *frame) {
    
    Value *returnList = makeNull();
    if(args->type == NULL_TYPE){
        return args;
    } else if (cdr(args)->type == NULL_TYPE) {
        return cons(eval(car(args), frame), returnList);
    } else {
        while (args->type != NULL_TYPE){
            returnList =  cons(eval(car(args), frame), returnList);
            args = cdr(args);
        }
        //printInterpreter(reverse(returnList));
        return reverse(returnList);
    }
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
    else {
        Value *expressions = cdr(args);
        Value *symbol;
        Value *bindingsExp;

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
        while (cdr(expressions)->type != NULL_TYPE) {
            eval(car(expressions), subFrame);
            expressions = cdr(expressions);
        }
        return eval(car(expressions),subFrame);
    }      

}


Value *lookUpSymbol(Value *tree, Frame *frame) {
    
    
    Frame *currFrame = frame;
    Value *currBinding;
    Value *nextBinding = frame->bindings ;
    
    while(currFrame != NULL){ 
        nextBinding = currFrame->bindings; //iterates through frame
        while(nextBinding->type != NULL_TYPE){ //checks in current frame for binding match
            
            currBinding = car(nextBinding);
            
            printf("%s \n", car(currBinding)->s );
            printf("%s \n", tree->s );


            if(!strcmp(tree->s, car(currBinding)->s)){
                
                //printInterpreter(cdr(currBinding));
                return cdr(currBinding);
            }
            
            nextBinding = cdr(nextBinding);
        }
        currFrame = currFrame->parent;
    }
    

    printf("ERROR: variable is not bound to anything\n");
    //printf("%s\n", tree->s);
    //printInterpreter(frame->bindings);
    texit(1);
    return NULL;
}


Value *eval(Value *tree, Frame *frame) {

    Value *result = makeNull();
    switch (tree->type) {
        case INT_TYPE:
            
            printf("Getting to 3 and 5 \n");
            return tree;
            break;
        case BOOL_TYPE: 
           return tree;
           break;
         case SYMBOL_TYPE:
            printf("IN SYMBOL CASE LOOKING FOR +\n");
            return lookUpSymbol(tree, frame);
            break;
         case STR_TYPE:
            return tree;
            break;
        case CONS_TYPE: {
            Value *first = car(tree);
            Value *args = cdr(tree);
            //Sanity and error checking on first...
            //printf("HERE WE ARE\n");
            if (strcmp(first->s,"if") == 0) {
                result = evalIf(args,frame);
            } else if (!strcmp(first->s,"let")) {
                result = evalLet(args,frame);
            } else if (!strcmp(first->s, "quote")) {
                result = evalQuote(args);
            } else if (!strcmp(first->s, "define")){
                result = evalDefine(args, frame);
            } else if (!strcmp(first->s, "lambda")){
                result = evalLambda(args, frame);
            } else {
                
                Value *evaledOperator = eval(first, frame);
                
                printf("%s \n", evaledOperator->s);
                
                // Checks if CLOSURE_TYPE; Else --> PRIM_TYPE
                if (evaledOperator->type == CLOSURE_TYPE) {
                    Value *evaledArgs = evalEach(args, frame);
                    return apply(evaledOperator,evaledArgs);
//                } else if (evaledOperator->type == PRIMITIVE_TYPE) {
                } else {
                    
                    printf("Hitting the eval > primitive case \n");
    
                    Value *evaledArgs = evalEach(args, frame);
                    result = evaledOperator->pf(evaledArgs);
                    printf("Not getting to here \n");
                }
            }
            break;
        }
        default:
            return tree;
    }
    return result;
}


Value *primitiveCar(Value *value){
    // Error Checking
    if (value->type != CONS_TYPE){
        printf("Syntax Error: invalid input.\n");
        texit(1);
    }
    if (cdr(value)->type!= NULL_TYPE){
        printf("Syntax Error: Expect only one argument.\n");
        texit(1);
    }
    
    Value *result = car(car(value));
    return result;
}

Value *primitiveCdr(Value *value) {
    
    if (value->type != CONS_TYPE){
        printf("Syntax Error: Invalid input -- needs to be cons.\n");
        texit(1);
    }
    if (cdr(value)->type!= NULL_TYPE){
        printf("Syntax Error: Expect only one argument.\n");
        texit(1);
    }
    Value *result = cdr(car(value));
    return result;
}



Value *primitiveCons(Value *value) {
    
    // Error Checking
    if (value->type != CONS_TYPE){
        printf("Syntax Error: \"cons\" statement expect a CONS_TYPE argument.\n");
        texit(1);
    }
    if (cdr(value)->type == NULL_TYPE || cdr(cdr(value))->type!= NULL_TYPE){
        printf("Syntax Error: Expect only two arguments \n");
        texit(1);
    }
    
    Value *firstCons = car(value);
    Value *secondCons = cdr(value);
    if (secondCons->type == CONS_TYPE){
        secondCons = car(secondCons);
    }
    return cons(firstCons,secondCons);
}


Value *primitiveAdd(Value *args) {
    
    printInterpreter(args);
    printf("args for primitiveAdd\n----------------\n");
    int result = 0;
    //result->type = INT_TYPE;
    while (args->type != NULL_TYPE) {
    
        Value *number = car(args);
        
        if (number->type != INT_TYPE &&
            number->type != DOUBLE_TYPE) {
            printf("Syntax Error: Add expect a INT_TYPE or DOUBLE_TYPE!\n");
            texit(1);
        }
        
        if (number->type == INT_TYPE){
            result += number->i;
        } else {
            result += number->d;
        }

        args = cdr(args);
    }

    Value *total = talloc(sizeof(Value));
    total->type = DOUBLE_TYPE;
    total->d = result;
    
    return total;
}

void bind(char *name, Value *(*function)(struct Value *), Frame *frame) {
    printf("Binding \n");
    
    Value *nameHolder = talloc(sizeof(Value));
    nameHolder->type = SYMBOL_TYPE;
    nameHolder->s = name;
    
    
    // Add primitive functions to top-level bindings list
    Value *value = talloc(sizeof(Value));
    value->type = PRIMITIVE_TYPE;
    value->pf = function;
    Value *binding = makeNull();
    binding = cons(value, binding);
    binding = cons(nameHolder, binding);
    frame->bindings = cons(binding, frame->bindings);
}

void bindPrimitives(Frame *frame){
    bind("+", primitiveAdd, frame);
   // bind("null?", primitiveNull, frame);
    bind("car", primitiveCar, frame);
    bind("cdr", primitiveCdr, frame);
    bind("cons", primitiveCons, frame);
}



void interpret(Value *tree) {

    topFrame = makeFrame(NULL);
    bindPrimitives(topFrame);
    
    Value *results;
    while(tree->type != NULL_TYPE){
        results = eval(car(tree), topFrame);
        printInterpreter(results);
        if(results->type != VOID_TYPE){
            printf("\n");
        }   
        tree = cdr(tree);
    }
}


