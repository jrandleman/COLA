// AUTHOR: JORDAN RANDLEMAN - cola_SampleExec.c - Sample Implementation of
// C Overloaded Length Arguments (COLA) Fcn/Macro use for 'cola.c'
// >> NOTE: "Overloading" means to have 2+ fcns/macros share the same name
// >> NOTE: Assign Function default arg values when defining them (more below) 

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

/*****************************************************************************
 ||^\\ //=\\ //=\ /| |\ /\\  //\ /|==\ /\\ ||==== //^\\ ==== ==== //=\\ /\\ ||
 ||  ))|| || ||   || || ||\\//|| ||=   ||\\|| || |/===\| ||   ||  || || ||\\||
 ||_// \\=// \\=/ \\=// || \/ || \|==/ || \// || ||   || ||  ==== \\=// || \//
 *****************************************************************************
 *                         -:- COLA.C X CAVEATS -:-                         *
 *   => "COLA INSTANCE" = "fcn/macro overload OR fcn w/ default arg values" *
 *   (0) "COLA_" PREFIX IS RESERVED                                         *
 *   (1) NO VARIADIC COLA INSTANCES                                         *
 *   (2) NO COLA INSTANCES W/IN CONDITIONAL PREPROCESSOR DIRECTIVES         *
 *       (*) IE NOT W/IN: #if, #ifdef, #ifndef, #elif, #else, & #endif      *
 *   (3) NO FCN PTRS POINTING TO COLA INSTANCES                             *
 *       (*) can't determine overloaded arg # from only overloaded fcn name *
 *   (4) NO REDEFINING COLA INSTANCE NAME TO OTHER VARS REGARDLESS OF SCOPE *
 *   (5) NO OVERLOADED MACROS CAN EVER BE "#undef"'d                        *
 *   (6) ONLY COLA INSTANCES DEFINED/PROTOTYPED GLOBALLY WILL BE RECOGNIZED *
 *   (7) ONLY FUNCTIONS MAY BE ASSIGNED DEFAULT VALUES - NEVER MACROS!      *
 *   (8) NO ARG W/ A DEFAULT VALUE MAY PRECEDE AN ARG W/O A DEFAULT VALUE   *
 *       (*) args w/ default values must always by last in a fcn's arg list *
 *   (9) FCN PROTOTYPES TAKE PRECEDENT OVER DEFINITIONS WRT DEFAULT VALS    *
 *       (*) if a fcn proto has default vals but its defn doesn't (or vise  *
 *           versa) fcn will be treated as if has default vals              *
 *       (*) if a fcn proto has DIFFERENT default vals from its defn, the   *
 *           fcn's proto default vals are treated as the only default vals  *
 *****************************************************************************
 *  -:- ODV GUIDELINE TO COMBINE (O)VERLOADS W/ (D)EFAULT ARG (V)ALUES -:-  *
 * >> overload definitions must satisfy 1 of the following:                 *
 *    (1) an overload's # of non-dflt args must exceed the # of cumulative  *
 *        args (both dflt & not) of all other overloaded instances          *
 *    (2) an overload's # of cumulative args (both dflt & not) must be less *
 *        than the # of non-dflt args of all other overloaded instances     *
*****************************************************************************/

typedef struct profile {
  char name[100];
  int age;
  float gpa;
} PROFILE;


// Overload Macros w/ Differing Arg Lengths
#define show(PERSON, SCHOOL) (printf("name, school: %s, %s\n", PERSON.name, SCHOOL))
#define show(PERSON, FIRSTFAVCLASS, SECNDFAVCLASS)\
  (printf("name, 1st & 2nd favorite classes: %s, %s & %s\n", PERSON.name, FIRSTFAVCLASS, SECNDFAVCLASS))


// Overload Functions w/ Differing Arg Lengths
void assign(PROFILE *person, char NAME[], int AGE) {
  person-> age = AGE;
  strcpy(person->name, NAME);
}
void assign(PROFILE *person, float GPA) {
  person->gpa = GPA;
}


// Overload Both Macros & Functions With the Same Name
void show(PROFILE person) { // same name as the 2 macros above
  printf("name, age, gpa: %s, %d, %.1f\n", person.name, person.age, person.gpa);
}


// Assigning Default Values to a Function's Arguments
// NOTE => args w/ default values must ALWAYS be passed last to a function
void demo_default_vals(int x, char *str = "default value used!", float num = 9.999) { 
  printf("Function \"demo_default_vals\" Invoked With 'x' = %d, 'str' = \"%s\", & 'num' = %.3f\n", x, str, num);
}


// Assigning a Default Value to an Overloaded Function
// NOTE => Do this w/ care to avoid ambiguous fcn invocation signatures. 
//         Users are less likely to make mistakes if they keep overloads & default values
//         apart, but the ability to do so is there if the ODV GUIDLINE under CAVEATS is adhered.
void overload_defaults(int x, int y) {
  printf("Function \"overload_defaults\" Without Default Values: x = %d & y = %d\n", x, y);
}
void overload_defaults(int x = 10) {
  printf("Function \"overload_defaults\" With a Default Value: x = %d\n", x);
}

/* 
 * // throws an error: given 1 arg could also be overload 2 & given 2 args could also be overload 1
 * void overload_defaults(int x, int y = 10) { 
 *   printf("Function \"overload_defaults\" Without Default Values: x = %d & y = %d\n", x, y);
 * } 
 *
 * // also throws an error: could be confused with the 2nd overload given that both can be called w/o an arg
 * void overload_defaults() { 
 *   printf("Function \"overload_defaults\" Without Any Values Called!\n");
 * } 
 */


int main() {
  PROFILE author;

  // Using Overloaded "assign" Function:
  printf("Using an Overloaded \"assign\" Function to Give a Name & Age to a \"struct profile\" Object Named \"author\":\n");
  assign(&author, "Jordan Randleman", 20);
  printf("author name, age: %s, %d\n", author.name, author.age);
  printf("\nUsing another Overloaded \"assign\" Function to Give \"author\" a GPA:\n");
  assign(&author, 3.9);
  printf("author gpa: %.1f\n\n", author.gpa);

  // Using 3 Overloaded "show" Actions, 2 Macros & 1 Function:
  printf("Using 3 Overloaded \"show\" Actions, 2 Macros & 1 Function:\nMACRO OF 2 ARGS:  \t");
  show(author, "Santa Clara University");
  printf("MACRO OF 3 ARGS:  \t");
  show(author, "CompSci", "Mathematics");
  printf("FUNCTION OF 1 ARG:\t");
  show(author);

  // Using a Function With Default Arguments
  printf("\nUsing the Function \"demo_default_vals\" Without its Default Values:\n\t");
  demo_default_vals(1, "Hola!", 1.111); // No Defaults Are Used (All Passed Explicitly to the Fcn)
  printf("Using the Function \"demo_default_vals\" With its Default Float Value of \"9.999\":\n\t");
  demo_default_vals(1, "Hola!"); // Default Value of '9.999' Are Used
  printf("Using the Function \"demo_default_vals\" With its Default Float & String Values:\n\t");
  demo_default_vals(1); // Default Values of '"default value used!"' && '9.999' Are Used

  // Using a Function That's Been Simultaneously Overloaded & Given a Default Argument Value
  // NOTE => Observe the ODV GUIDLINE above under CAVEATS to ensure ambiguous calls are avoided!
  printf("\nUsing the Overloaded \"overload_defaults\" Function Variant w/o Any Defined Default Values:\n\t");
  overload_defaults(112, 358);
  printf("Using the Overloaded \"overload_defaults\" Function Variant w/ That Does Have a Default Value:\n\t");
  overload_defaults();
  printf("Using the Same Overloaded Variant as Above but Overriding its Default Value:\n\t");
  overload_defaults(987654321);

  printf("\nBye!\n\n");

  return 0;
}
