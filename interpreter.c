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



/*
Given an expression tree and a frame in which to evaluate that expression, 
eval returns the value of the expression.
*/
/* SAMPLE CODE FROM DAVE

switch (tree->type)  {
     case INT_TYPE: {
        ...
        break;
     }
     case ......: {
        ...
        break;
     }  
     case SYMBOL_TYPE: {
        return lookUpSymbol(tree, frame);
        break;
     }  
     case CONS_TYPE: {
        Value *first = car(tree);
        Value *args = cdr(tree);

        // Sanity and error checking on first...

        if (!strcmp(first->s,"if")) {
            result = evalIf(args,frame);
        }

        // .. other special forms here...

        else {
           // not a recognized special form
           evalationError();
        }
        break;
     }

      ....
    }    
    ....
}

*/
/*
(if #f 7 12)
should return 12
*/
Frame *makeFrame(Frame *parent) {
	Frame *frame = talloc(sizeof(Frame));
	(*frame).bindings = makeNull();
	(*frame).parent = parent;
	return frame;
}

Value *evalIf(Value *args, Frame *frame) {
	Value *test = car(args);
	Value *expressions = cdr(args);
	int testresult = eval(test, frame);   //evaluate the test, get back a int bool.

	if (testresult == 1) {
		printf("IN THE TRUE CASE OF EVAL\n");
		return eval(car(expressions), frame);		//true expression
	} else {
	 	printf("IN THE FALSE CASE OF EVAL IF\n");
		return eval(car(cdr(expressions)), frame);		//false expression
	}
}


// Value *evalLet(args, Frame *Frame) {

// }

// Value *lookUpSymbol(Value *tree, Frame *frame) {

// }

Value *eval(Value *tree, Frame *frame) {

	Value *result = makeNull();
	//printf("HERE:%\n", tree);
	switch (tree->type) {
	    case INT_TYPE:
	    	printf("INT CASE:%i\n", tree->i);
	    	return tree->i;
	    	break;
	    case BOOL_TYPE: 
	       return tree->b;
	       break;
	    // case SYMBOL_TYPE:
	    //    //return lookUpSymbol(tree, frame);
	    //    break;
	    // case STR_TYPE:
	    // 	return tree;
	    // 	break
	    case CONS_TYPE: {
	        Value *first = car(tree);
	        //printf("THIS:%s\n", (*first).symbol);
	        // if ((*first).type == NULL_TYPE) {
	        // 	return first;
	        // }
	        Value *args = cdr(tree);


	        //eval(car(tree), frame);
	        //eval(cdr(tree), frame);
	        //Sanity and error checking on first...
	        //printf("HERE WE ARE\n");
	        if (strcmp(first->symbol,"if") == 0) {
	        	printf("IN THE IF \n");
	            result = evalIf(args,frame);
	        }
	        // else if (!strcmp(first->symbol,"let")) {

	        //     //result = evalLet(args,frame);
	        // }
	        // .. other special forms here...
	        else {
	           // not a recognized special form
	           //evalationError();
	        	printf("ERROR: invald expression to evaluate\n");
	        }
	        break;
	    }
	    default:
	     	printf("ERROR: type not accounted for by switch case!\n");
	}
	return result;
}

void interpret(Value *tree) {

	Frame *globalFrame = makeFrame(NULL);
	tree = car(tree);
	Value *results = eval(tree, globalFrame);

}
