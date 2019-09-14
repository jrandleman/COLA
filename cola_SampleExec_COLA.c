// AUTHOR: JORDAN RANDLEMAN - cola_SampleExec.c - Sample Implementation of
// C Overloaded Length Arguments (COLA) Fcn/Macro use for 'cola.c'
// >> NOTE: "Overloading" means to have 2+ fcns/macros share the same name

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

/*****************************************************************************
 ||^\\ //=\\ //=\ /| |\ /\\  //\ /|==\ /\\ ||==== //^\\ ==== ==== //=\\ /\\ ||
 ||  ))|| || ||   || || ||\\//|| ||=   ||\\|| || |/===\| ||   ||  || || ||\\||
 ||_// \\=// \\=/ \\=// || \/ || \|==/ || \// || ||   || ||  ==== \\=// || \//
 *****************************************************************************
 *                         -:- COLA.C 7 CAVEATS -:-                         *
 *   (0) "COLA_" PREFIX IS RESERVED                                         *
 *   (1) NO OVERLOADED VARIADIC FCNS/MACROS                                 *
 *   (2) NO OVERLOADS W/IN CONDITIONAL PREPROCESSOR DIRECTIVES directives   *
 *       (*) IE NOT W/IN: #if, #ifdef, #ifndef, #elif, #else, & #endif      *
 *   (3) NO FCN PTRS POINTING TO OVERLOADED FCNS                            *
 *       (*) can't determine overloaded arg # from only overloaded fcn name *
 *   (4) NO REDEFINING OVERLOADED NAME AS A DIFFERENT VAR NAME IN ANY SCOPE *
 *   (5) NO OVERLOADED MACROS CAN EVER BE "#undef"'d                        *
 *   (6) ONLY GLOBALLY DEFINED FCNS/MACROS CAN BE OVERLOADED                *
 *****************************************************************************/

typedef struct profile {
  char name[100];
  int age;
  float gpa;
} PROFILE;


// Overload Macros w/ Differing Arg Lengths
#define COLA__2_show(PERSON, SCHOOL) (printf("name, school: %s, %s\n", PERSON.name, SCHOOL))
#define COLA__3_show(PERSON, FIRSTFAVCLASS, SECNDFAVCLASS)\
  (printf("name, 1st & 2nd favorite classes: %s, %s & %s\n", PERSON.name, FIRSTFAVCLASS, SECNDFAVCLASS))


// Overload Functions w/ Differing Arg Lengths
void COLA__3_assign(PROFILE *person, char NAME[], int AGE) {
  person-> age = AGE;
  strcpy(person->name, NAME);
}
void COLA__2_assign(PROFILE *person, float GPA) {
  person->gpa = GPA;
}


// Overload Both Macros & Functions With the Same Name
void COLA__1_show(PROFILE person) { // same name as the 2 macros above
  printf("name, age, gpa: %s, %d, %.1f\n", person.name, person.age, person.gpa);
}


int main() {
  PROFILE author;

  // Using Overloaded "assign" Function:
  printf("Using an Overloaded \"assign\" Function to Give a Name & Age to a \"struct profile\" Object Named \"author\":\n");
  COLA__3_assign(&author, "Jordan Randleman", 20);
  printf("author name, age: %s, %d\n", author.name, author.age);
  printf("\nUsing another Overloaded \"assign\" Function to Give \"author\" a GPA:\n");
  COLA__2_assign(&author, 3.9);
  printf("author gpa: %.1f\n\n", author.gpa);

  // Using 3 Overloaded "show" Actions, 2 Macros & 1 Function:
  printf("Using 3 Overloaded \"show\" Actions, 2 Macros & 1 Function:\nMACRO OF 2 ARGS:  \t");
  COLA__2_show(author, "Santa Clara University");
  printf("MACRO OF 3 ARGS:  \t");
  COLA__3_show(author, "CompSci", "Mathematics");
  printf("FUNCTION OF 1 ARG:\t");
  COLA__1_show(author);

  printf("\nBye!\n\n");

  return 0;
}
