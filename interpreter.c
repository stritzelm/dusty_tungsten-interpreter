/*
 created by Dillon Lanier, Tony Ngo, Matt Stritzel 11-24-2019 for programming languages with dave musicant.
 Interprets the programming language sceme using C. (takes scheme input and runs it with C)
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
#include <math.h>

//Global Frame
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
        case PRIMITIVE_TYPE:
            //printf("ITS A PRIMITIVE_TYPE\n");
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
            if(cdr(tree)->type != CONS_TYPE && cdr(tree)->type != NULL_TYPE){
                printf(" .");
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

/*
 * Creates a VOID_TYPE value and returns a pointer to it
 */
Value *makeVoidValue() {
    Value *voidValue = makeNull();
    voidValue->type = VOID_TYPE;
    return voidValue;
}

/*
 * Make an empty frame with given frame as its parent
 */
Frame *makeFrame(Frame *parent) {
    Frame *frame = talloc(sizeof(Frame));
    (*frame).bindings = makeNull();
    (*frame).parent = parent;
    return frame;
}

/*
 * Make a closure (param names, parent frame, function code )
 * returns an object w Value type equal to CLOSURE_TYPE
 */
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

/*
 * Lambda creates a closure when seen, calls makeClosure()
 */
Value *evalLambda(Value *args, Frame *frame) {
    Value *closure = makeClosure(args, frame);
    return closure;
}

/*
 * Creates a new binding of var to the value of expr
 * Modifies current environment frame
 * Need to have a top or global frame, contains binding of variables created using define
 */
Value *evalDefine(Value *args, Frame *frame) {
    //using topFrame which is global frame
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

/* Apply creates a frame,
 * parent is frame pointed to by environment
 * create local variables (bindings to match the parameters)
 * it executes (evals) the body of the closure
 */
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
    
    return eval(body, frame);
}

/*
 * If it's a primitive function, it has type PRIMITIVE_TYPE, and is "applied"
 * differently than a CLOSURE_TYPE Value. Simply run corresponding C code for primitive
 * passing the args
 */
Value *applyPrimitive(Value *(*function)(struct Value *), Value *args) {
    return (*function)(args);
}

/*
 * Used to evaluate a Linked List easily, want to eval args before applying.
 */
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

/*
 * Evaluate an if expression, going to return based on whether first arg is true or false.
 */
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

/*
 * Just want to return same stuff without the quote in front.
 */
Value *evalQuote(Value *args) {
    return args;
}

/*
 * Evaluate a Let expression, create a new frame, and add to its bindings
 * Then once done with creating bindings, eval the rest of the code in that frame
 */
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


/* Evaluates the predicate expressions of successive clauses in order, until one of the predicates evaluates to a true value.
 *
 * When a predicate evaluates to a true value, cond evaluates the expressions in the associated clause in left to right order,
 * and returns the result of evaluating the last expression in the clause as the result of the entire cond expression.
 *
 * If the selected clause contains only the predicate and no expressions, cond returns the value of the predicate as the result.
 * If all predicates evaluate to false values, and there is no else clause, the result of the conditional expression is
 * unspecified; if there is an else clause, cond evaluates its expressions (left to right) and returns the value of the last one.
 
*/
Value *evalCond(Value *args, Frame *frame) {

    Value *current = args;
    while (current->type != NULL_TYPE) {
        
        Value *currExpression = car(current);
       
        Value *predicate = car(currExpression);
        Value *expr = cdr(currExpression);
        
        if (predicate->type == SYMBOL_TYPE &&
            !strcmp(predicate->s, "else")) {
            return eval(car(expr), frame);
        }
        
        // Evaluate the condition if this is not the else case.
        predicate = eval(predicate, frame);
        
        if (predicate->type != BOOL_TYPE) {
            printf("Error: Predicate should evaluate to a BOOL_TYPE \n");
            texit(1);
        }
        if (predicate->i == 1) {
            return eval(car(expr), frame);
        }
        current = cdr(current);
    }
}

/* The begin special form is used to evaluate expressions in a particular order.
 *  The begin special form is used to evaluate expressions in a particular order.
 * This expression type is used to sequence side effects such as input and output.
*/

Value *evalBegin(Value *args, Frame *frame) {

   Value *currexpressions = args;

    while (cdr(currexpressions)->type != NULL_TYPE) {
            eval(car(currexpressions), frame);
            currexpressions = cdr(currexpressions);
        }
    return eval(car(currexpressions),frame);
}



/* The expressions are evaluated from left to right, and the value of the first expression that evaluates to a false value is returned.
 * Any remaining expressions are not evaluated. If all the expressions evaluate to true values, the value of the last expression is returned
 * If there are no expressions then #t is returned.
*/

Value *evalAnd(Value *args, Frame *frame) {
    
    Value *current = args;
    
    while (current->type != NULL_TYPE) {
        
        Value *cur = eval(car(current), frame);
        // Returns false if cur is false
        if (cur->type == BOOL_TYPE) {
            if (cur->i == 0) {
                return cur;
            }
        } else { // Should only be BOOL_TYPE
            
            printf("Error: Expected boolean \n");
        }
        
        current = cdr(current);
    }
    
    Value *result = makeNull();
    result->type = BOOL_TYPE;
    result->i = 1;
    return result;
}

/* The expressions are evaluated from left to right, and the value of the first expression that evaluates to a true value is returned.
 * Any remaining expressions are not evaluated.
 * If all expressions evaluate to false values, the value of the last expression is returned
 * If there are no expressions then #f is returned.
*/

Value *evalOr(Value *args, Frame *frame) {
    
    Value *current = args;
    
    while (current->type != NULL_TYPE) {
        
        Value *cur = eval(car(current), frame);
        
        if (cur->type == BOOL_TYPE) {
            if (cur->i == 1) {
                return cur;
            }
        } else { // Should only be BOOL_TYPE
            
            printf("Error: Expected boolean \n");
        }
        
        current = cdr(current);
    }
    
    Value *result = makeNull();
    result->type = BOOL_TYPE;
    result->i = 0;
    return result;
        
}

/*
*
*/
void evalSetBang(Value *args, Frame *frame) {
    if (cdr(args)->type == NULL_TYPE || args->type == NULL_TYPE) {
        printf("Evaluation ERROR!\n");
        texit(1);
    }
    if (cdr(cdr(args))->type != NULL_TYPE) {
        printf("Evaluation ERROR! This implementation only supports two arguments\n");
        texit(1);
    }

    Value *variable = car(args);
    Value *expression = eval(car(cdr(args)), frame);


    Frame *currFrame = frame;
    Value *currBinding = makeNull();
    Value *nextBinding = frame->bindings;
    
    while(currFrame != NULL){ 
        nextBinding = currFrame->bindings; //iterates through frame
        while(nextBinding->type != NULL_TYPE){ //checks in current frame for binding match
            currBinding = car(nextBinding);
            if(!strcmp(variable->s, car(currBinding)->s)){
                cdr(currBinding)->i = expression->i;
            }
            nextBinding = cdr(nextBinding);
        }
        currFrame = currFrame->parent;
    }
}


/*
 * Take a SYMBOL_TYPE Value and look it up in the stack of frames. Start with one
 * you're in, then go up the chain of parents
 */
Value *lookUpSymbol(Value *tree, Frame *frame) {
    Frame *currFrame = frame;
    Value *currBinding = makeNull();
    Value *nextBinding = frame->bindings;
    
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
    printf("ERROR: variable is not bound to anything\n");
    texit(1);
    return NULL;
}

/*
 * Main function for interpreter. Take a tree of parse'd scheme code (R5RS) and evaluate it using C.
 * Implements a Recursive Descent Parser (right??).
 *
 */
Value *eval(Value *tree, Frame *frame) {
    Value *result = makeNull();
    switch (tree->type) {
        case INT_TYPE:
            return tree;
            break;
        case BOOL_TYPE:
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
            Value *args = cdr(tree);
            
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
            } else if (!strcmp(first->s, "cond")) {
                 result = evalCond(args, frame);
            } else if (!strcmp(first->s, "and")) {
                result = evalAnd(args, frame);
            } else if (!strcmp(first->s, "or")) {
                result = evalOr(args, frame);
            } else if (!strcmp(first->s, "set!")) {
                evalSetBang(args, frame);
            } else if (!strcmp(first->s, "begin")) {
                result = evalBegin(args, frame);
            } else {  //It's a Primitive or closure type!
                Value *evaledOperator = eval(first, frame);
                Value *evaledArgs = evalEach(args, frame);
                if (evaledArgs->type == NULL_TYPE) {
                    return first;
                }
                if (evaledOperator->type == PRIMITIVE_TYPE) {
                    return applyPrimitive(evaledOperator->pf, evaledArgs);
                } else if (evaledOperator->type == CLOSURE_TYPE){
                    return apply(evaledOperator,evaledArgs);
                } else {
                    printf("ERROR, method not defined\n");
                }
            }
            break;
        }
        default:
            return tree;
    }
    return result;
}

/*
* Primitive module, returns the answer to a x % y
* EXAMPLE: (module 10 3) = 1
* EXAMPLE: (module 12 7) = 5
*/
Value *primitiveModule(Value *value) {
    double first,second;
    if (length(value) != 2){
        printf("Error: Only two arguments expected!\n");
        texit(1);
    }
    Value *firstArg = car(value);
    Value *secondArg = car(cdr(value));
    
    if ((firstArg->type != INT_TYPE && firstArg->type != DOUBLE_TYPE) ||
        (secondArg->type != INT_TYPE && secondArg->type != DOUBLE_TYPE)){
        printf("Error: Expected an INT_TYPE or DOUBLE_TYPE!\n");
        texit(1);
    }
    
    if (firstArg->type == INT_TYPE){
        first = (double)firstArg->i;
    }
    else {
        first = firstArg->d;
    }
    if (secondArg->type == INT_TYPE){
        second = (double)secondArg->i;
    }
    else {
        second = secondArg->d;
    }

    while ((first - second) >= 0) {
        first = first - second;
    } 
    int intResult = (int) first;
    //If "Int-ing" the result gives a value equal to result, then final result is an int
    //so we shouldn't return a double.
    if (intResult == first) { //case where result we want to return is a int
        Value *returnResult = makeNull();
        returnResult->i = (int) first;
        returnResult->type = INT_TYPE;
        return returnResult;
    } else { //case where result we want to return is a double
        Value *returnResult = makeNull();
        returnResult->d = first;
        returnResult->type = DOUBLE_TYPE;
        return returnResult;
    }
}

/*
 * returns the first thing from a list
 * EXAMPLE:  (car (quote (a b c))) = a
 */
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
    Value *result = car(car(car(value)));
    return result;
}

/*
 * returns everything bu the first thing from a list
 * EXAMPLE:  (cdr (quote (a b c))) = (b c)
 */
Value *primitiveCdr(Value *value){
    Value *result;
    Value *temp = makeNull();
    //check if value is cons type first
    if (value->type != CONS_TYPE){
        printf("Syntax Error: invalid input, expected CONS_TYPE.\n");
        texit(1);
    }
    //make sure only 1 arg
    if (cdr(value)->type!= NULL_TYPE){
        printf("Syntax Error: Expect only one argument.\n");
        texit(1);
    }
    if (car(value)->type == NULL_TYPE){
        printf("Syntax Error: expected pair.\n");
        texit(1);
    }
    result = cdr(car(car(value)));
    return cons(result, temp);
}

/*
 * Implements the scheme function "Cons"
 * EXAMPLE: (cons 6 3) = (6 . 3)
 * EXAMPLE: (cons 5 (quote(3 4 6)) ) = (5 3 4 6)
 */
Value *primitiveCons(Value *value) {
    // Error Checking
    if (value->type != CONS_TYPE){ // Problem if neither thing is a cons
        printf("Syntax Error: \"cons\" statement expect a CONS_TYPE argument.\n");
        texit(1);
    }
    //Make sure there are only 2 arguements
    if (cdr(value)->type == NULL_TYPE || cdr(cdr(value))->type != NULL_TYPE){
        printf("Syntax Error: Expect only two arguments \n");
        texit(1);
    }
    Value *firstCons = car(value);
    Value *secondCons = cdr(value);
    Value *end = makeNull();
    Value *firstResult;
    Value *secondResult;
    //check of first arg is cons
    if (firstCons->type == CONS_TYPE){
        firstCons = car(firstCons);
    }
    //check if second arg is cons
    if (secondCons->type == CONS_TYPE){ // Case where you are "cons-ing" two lists
        if(car(secondCons)->type == CONS_TYPE){
            secondResult = car(car(secondCons));
            while(cdr(car(secondCons))->type != NULL_TYPE){
                secondResult = cons(car(cdr(car(secondCons))), secondResult);
                secondCons = cdr(secondCons);
            }
            return cons(cons((firstCons), secondResult) , end);
        }
        else{ // Case where first thing is cons, second isn't
            return cons(cons(firstCons, car(secondCons)), end);
        }
    }
    printf("errpr: cons error\n");
    texit(1);
    return NULL;
}

/*
 * returns the sum of all arguments if they are ints and doubles
 * EXAMPLE:  (+ 2.2 9 0) = 11.2
 */
Value *primitiveAdd(Value *args) {
    double result = 0;
    //At end of while loop have the result in a double form
    while (args->type != NULL_TYPE) {
        Value *number = car(args);
        if (number->type != INT_TYPE &&
            number->type != DOUBLE_TYPE) {
            printf("Syntax Error: Add expect a INT_TYPE or DOUBLE_TYPE!\n");
            texit(1);
        }
        if (number->type == INT_TYPE){
            result += (float)number->i;
            
        } else {
            result += number->d;
        }
        args = cdr(args);
    }
    
    int intResult = (int) result;
    //If "Int-ing" the result gives a value equal to result, then final result is an int
    //so we shouldn't return a double.
    if (intResult == result) { //case where result we want to return is a int
        Value *returnResult = makeNull();
        returnResult->i = (int) result;
        returnResult->type = INT_TYPE;
        return returnResult;
    } else { //case where result we want to return is a double
        Value *returnResult = makeNull();
        returnResult->d = result;
        returnResult->type = DOUBLE_TYPE;
        return returnResult;
    }
}

/* * should be able to take any number of numeric arguments >= 2 */
Value *primitiveMult(Value *args) {
    
    double result = 1.0; // Multiplies args by 1
    
    //loop through all arguments, add them
    while(args->type != NULL_TYPE){
        Value *number = car(args);
        //check to make sure args are ints or doubles
        if (number->type != INT_TYPE &&
            number->type != DOUBLE_TYPE) {
            printf("Syntax Error: Mult expect a INT_TYPE or DOUBLE_TYPE!\n");
            texit(1);
        }
        if(number->type == INT_TYPE){
            result = result * number->i;
        }
        else {
            result = result * number->d;
        }
        
        args = cdr(args);
    }
    
    int intResult = (int) result;
    
    if (intResult == result) {
        
        Value *returnResult = makeNull();
        returnResult->i = (int) result;
        returnResult->type = INT_TYPE;
        return returnResult;
    } else {
        
        Value *returnResult = makeNull();
        returnResult->d = result;
        returnResult->type = DOUBLE_TYPE;
        return returnResult;
    }
}


Value *primitiveDiv(Value *args){
    //Checking the length of sublist.
    double first,second;
    if (length(args) != 2){
        printf("Error: Expected only two arguments!\n");
        texit(1);
    }
    Value *firstArg = car(args);
    Value *secondArg = car(cdr(args));
    //Checking the type of sublist, only allow INT_TYPE and DOUBLE_TYPE.
    if ((firstArg->type != INT_TYPE && firstArg->type != DOUBLE_TYPE) ||
        (secondArg->type != INT_TYPE && secondArg->type != DOUBLE_TYPE)){
        printf("Error: Expected an INT_TYPE or DOUBLE_TYPE!\n");
        texit(1);
    }
    if (firstArg->type == INT_TYPE){
        first = (double)firstArg->i;
    }
    else {
        first = firstArg->d;
    }
    if (secondArg->type == INT_TYPE){
        second = (double)secondArg->i;
    }
    else {
        second = secondArg->d;
    }
    
    double result = first / second;
    
    int intResult = (int) result;
    
    if (intResult == result) {
        
        Value *returnResult = makeNull();
        returnResult->i = (int) result;
        returnResult->type = INT_TYPE;
        return returnResult;
    } else {
        
        Value *returnResult = makeNull();
        returnResult->d = result;
        returnResult->type = DOUBLE_TYPE;
        return returnResult;
    }
}


Value *primitiveSub(Value *args){
    double first,second;
    if (length(args) != 2){
        printf("Error: Only two arguments expected!\n");
        texit(1);
    }
    Value *firstArg = car(args);
    Value *secondArg = car(cdr(args));
    
    if ((firstArg->type != INT_TYPE && firstArg->type != DOUBLE_TYPE) ||
        (secondArg->type != INT_TYPE && secondArg->type != DOUBLE_TYPE)){
        printf("Error: Expected an INT_TYPE or DOUBLE_TYPE!\n");
        texit(1);
    }
    
    if (firstArg->type == INT_TYPE){
        first = (double)firstArg->i;
    }
    else {
        first = firstArg->d;
    }
    if (secondArg->type == INT_TYPE){
        second = (double)secondArg->i;
    }
    else {
        second = secondArg->d;
    }
    
    double result = first - second;
    
    int intResult = (int) result;
    
    if (intResult == result) {
        Value *returnResult = makeNull();
        returnResult->i = (int) result;
        returnResult->type = INT_TYPE;
        //printf("got to the end of primitive add\n");
        return returnResult;
    } else {
        Value *returnResult = makeNull();
        returnResult->d = result;
        returnResult->type = DOUBLE_TYPE;
        //printf("got to the end of primitive add\n");
        return returnResult;
    }
}

Value *primitiveLess(Value *args){
    double first,second;
    
    if (length(args) != 2){
        printf("Error: Expected only two arguments!\n");
        texit(1);
    }
    Value *firstArg = car(args);
    Value *secondArg = car(cdr(args));
    
    
    if ((firstArg->type != INT_TYPE && firstArg->type != DOUBLE_TYPE)||
        (secondArg->type != INT_TYPE && secondArg->type != DOUBLE_TYPE)){
        printf("Error: Expected an INT_TYPE or DOUBLE_TYPE!\n");
        texit(1);
    }
    
    if (firstArg->type == INT_TYPE){
        first = (double)firstArg->i;
    }
    else {
        first = firstArg->d;
    }
    if (secondArg->type == INT_TYPE){
        second = (double)secondArg->i;
    }
    else {
        second = secondArg->d;
    }
    
    Value *returnResult = talloc(sizeof(Value));
    returnResult->type = BOOL_TYPE;
    if (first >= second){
        returnResult->i = 0;
    }else{
        returnResult->i = 1;
    }
    
    return returnResult;
}

Value *primitiveGreater(Value *args){
    double first,second;
    
    if (length(args) != 2){
        printf("Error: Expected only two arguments!\n");
        texit(1);
    }
    Value *firstArg = car(args);
    Value *secondArg = car(cdr(args));
    
    if ((firstArg->type != INT_TYPE && firstArg->type != DOUBLE_TYPE)||
        (secondArg->type != INT_TYPE && secondArg->type != DOUBLE_TYPE)){
        printf("Error: Expected an INT_TYPE or DOUBLE_TYPE!\n");
        texit(1);
    }
    
    if (firstArg->type == INT_TYPE){
        first = (double)firstArg->i;
    }
    else {
        first = firstArg->d;
    }
    if (secondArg->type == INT_TYPE){
        second = (double)secondArg->i;
    }
    else {
        second = secondArg->d;
    }
    
    Value *returnResult = talloc(sizeof(Value));
    returnResult->type = BOOL_TYPE;
    if (first >= second){
        returnResult->i = 1;
    }else{
        returnResult->i = 0;
    }
    
    return returnResult;
}

Value *primitiveEq(Value *args){
    //Checking the length of sublist.
    double first,second;
    if (length(args) != 2){
        printf("Error: Expected only two arguments!\n");
        texit(1);
    }
    Value *firstArg = car(args);
    Value *secondArg = car(cdr(args));
    //Checking the type of sublist, only allow INT_TYPE and DOUBLE_TYPE.
    if ((firstArg->type != INT_TYPE && firstArg->type != DOUBLE_TYPE)||
        (secondArg->type != INT_TYPE && secondArg->type != DOUBLE_TYPE) ){
        printf("Error: Expected an INT_TYPE or DOUBLE_TYPE!\n");
        texit(1);
    }
    
    if (firstArg->type == INT_TYPE){
        first = (double)firstArg->i;
    }
    else {
        first = firstArg->d;
    }
    if (secondArg->type == INT_TYPE){
        second = (double)secondArg->i;
    }
    else {
        second = secondArg->d;
    }
    
    Value *returnResult = talloc(sizeof(Value));
    returnResult->type = BOOL_TYPE;
    if (first == second){
        returnResult->i = 1;
    }else{
        returnResult->s = 0;
    }
    return returnResult;
}



/*
 *
 */
Value *primitiveNull(Value *value){
    Value *result = makeNull();
    result->type = BOOL_TYPE;
    if(value->type == CONS_TYPE && cdr(value)->type!= NULL_TYPE){
        //error because more than 1 arguement
        printf("Syntax Error: Expect only one argument.\n");
        texit(1);
    }
    if (car(car(value))->type == NULL_TYPE){
        result->i = 1;
        return result;
    }
    else{
        result->i = 0;
        return result;
    }
}

/*
 * Takes a primitive function name, corresponding C function, and frame
 * Creates a Global Frame binding of the name to the C function
 */
void bind(char *name, Value *(*function)(struct Value *), Frame *frame) {
    Value *value = talloc(sizeof(Value));
    value->type = PRIMITIVE_TYPE;
    value->pf = function;
    Value *valueName = makeNull();
    valueName->type = SYMBOL_TYPE;
    valueName->s = name;
    
    Value *tempBinding = cons(valueName, value);
    topFrame->bindings = cons(tempBinding, topFrame->bindings);
}

/*
 * Helper function to make interpret() look better
 * Binds all the primitives at start of program in the Global Frame
 */
void bindPrimitives(Frame *frame){
    bind("+", primitiveAdd, frame);
    bind("null?", primitiveNull, frame);
    bind("car", primitiveCar, frame);
    bind("cdr", primitiveCdr, frame);
    bind("cons", primitiveCons, frame);
    bind("*", primitiveMult, frame);
    bind("-", primitiveSub, frame);
    bind("<", primitiveLess, frame);
    bind(">", primitiveGreater, frame);
    bind("/", primitiveDiv, frame);
    bind("=", primitiveEq, frame);
    bind("module", primitiveModule, frame);
}

/*
 * Kick everything off by taking in the parse tree,
 * Creating a global frame and putting primitive bindings in it
 * Going through the tree recursively calling eval,
 * and finally printing the output of the parse tree code
 */
void interpret(Value *tree) {
    topFrame = makeFrame(NULL);
    bindPrimitives(topFrame);
    Value *results;
    while(tree->type != NULL_TYPE) {
        results = eval(car(tree), topFrame);
        printInterpreter(results);
        if(results->type != VOID_TYPE){
            printf("\n");
        }
        tree = cdr(tree);
    }
}
