// AUTHOR: JORDAN RANDLEMAN -- cola.c -- C OVERLOADED LENGTH ARGUMENTS:
// PRE-PREPROCESS .C FILES OVERLOADING FCNS & MACROS BY THEIR # OF ARGS
// Email jrandleman@scu.edu or see https://github.com/jrandleman for support
/**
 * compile: $ gcc -o cola cola.c
 *          $ ./cola yourFile.c 
 *     (OR) $ ./cola -l yourFile.c // optional '-l' shows macro/function parsing details
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>

// scrape file's contents & put contents into file
#define FSCRAPE(BUFF,FNAME)\
  ({BUFF[0]='\0';FILE*fptr;if((fptr=fopen(FNAME,"r"))==NULL){exit(EXIT_FAILURE);}while(fgets(&BUFF[strlen(BUFF)],500,fptr)!=NULL);fclose(fptr);})
#define FPUT(BUFF,FNAME) ({FILE*fptr;if((fptr=fopen(FNAME,"w"))==NULL){exit(EXIT_FAILURE);}fprintf(fptr,"%s",BUFF);fclose(fptr);})
// initialize array with zero's (wipes garbage memory)
#define FLOOD_ZEROS(arr, len) ({int arr_i = 0;for(; arr_i < len; ++arr_i) arr[arr_i] = 0;})
// checks if char is whitespace, or alphanumeric/underscore
#define IS_WHITESPACE(c) (c == ' ' || c == '\t' || c == '\n')
#define VARCHAR(CH) (((CH)>='A' && (CH)<='Z') || ((CH)>='a' && (CH)<='z') || (((CH)>='0' && (CH)<='9')) || (CH) == '_')
// max filesize, max # fcns/macros per program, max # of unique overloads per fcn/macro
// self imposed for memory's sake, increment as needed
#define MAX_FILESIZE 1000001                                       // 1 gigabyte + '\0'
#define MAX_TOTAL_NUMBER_OF_FUNCTIONS_AND_MACROS_IN_PROGRAM 5000   // max # of macros & fcns parsed file can have
#define MAX_UNIQUE_OVERLOADS_PER_OVERLOADED_FCN_MACRO_INSTANCE 100 // max # of COLA's per overloaded fcn/macro name

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

/* GLOBAL FILE FCN/MACRO & OVERLOADING TRACKING STRUCTURES */
// holds all global fcn & macro def's found in file
struct function_macro_instance {
  char name[75];       // defined fcn/macro name
  int args;            // number of args
  bool overloaded;     // whether its overloaded
  bool is_a_prototype; // whether is a function prototype
} fmacs[MAX_TOTAL_NUMBER_OF_FUNCTIONS_AND_MACROS_IN_PROGRAM];
int fmacs_size = 0;
// holds function & macro overload instances, derived from "fmacs"
struct function_macro_overload_instance {
  char name[75];        // overoaded fcn/macro name
  int arg_sizes[MAX_UNIQUE_OVERLOADS_PER_OVERLOADED_FCN_MACRO_INSTANCE]; // different arg lengths per overload
  int arg_sizes_length; // # of other overoaded fcn/macros w/ same name
} overload_fmacs[MAX_TOTAL_NUMBER_OF_FUNCTIONS_AND_MACROS_IN_PROGRAM];
int overload_fmacs_size = 0;

/* STRING, CHAR, AND GLOBAL SCOPES STATUS UPDATING && STRING HELPER FUNCTIONS */
void handle_string_char_brace_scopes(bool*, bool*, int*, char*);
bool is_at_substring(char*, char*);
/* COMMENT, CONDITIONAL DIRECTIVE, & MACRO-BODY SKIP/CPY FUNCTIONS */
char *cola_skip_comments(char*, char*, bool);
char *cola_skip_conditional_directives(char*, char*, bool);
char *skip_macro_body(char*);
/* "fmacs" (FUNCTION MACRO INSTANCES) STRUCT HELPER FUNCTIONS */
int non_prototype_duplicate_instance_in_fmacs(char*, int);
bool overloaded_name_already_in_fmacs(char*);
void update_all_name_overloaded_status(char*);
void get_fmac_name(char*, char*);
/* MACRO INSTANCE & FUNCTIONLIKENESS EVALUATION FUNCTIONS */
bool is_at_macro_name(char*);
bool macro_is_functionlike(char*);
/* "overload_fmacs" (FUNCTION MACRO OVERLOAD INSTANCES) STRUCT HELPER FUNCTIONS */
bool name_not_in_overload_fmacs(char*);
int overload_fmacs_instance_idx(int);
/* MESSAGING FUNCTIONS */
void COLA_in_ASCII();
void cola_ERROR_ascii_art();
void cola_missing_Cfile_alert();
void confirm_valid_file(char*);
void terminate_program(char*);
void throw_fatal_error_variadic_overload(char*);
void throw_fatal_error_duplicate_overload(char*, int);
void throw_fatal_error_undefined_arg_length_invocation(char*, int);
void throw_fatal_error_non_functional_invocation_of_overloaded_name(char*);
/* FUNCTION/MACRO ARGUMENT COUNTING FUNCTION */
int count_args(char*, char*);
/* FUNCTION-PROTOTYPE CHECKING FUNCTION */
bool is_function_prototype(char*);
/* MAIN ACCOUNTING FUNCTION FOR FUNCTION/MACRO DEFINITIONS IN FILE */
void register_all_global_function_macro_defs(char*);
/* FILTER OVERLOADED MACROS FROM "FMACS" INTO "OVERLOAD_FMACS" STRUCTURE */
void filter_overloads_from_FMACS_to_OVERLOAD_FMACS();
/* PREFIX ALL OVERLOADED FUNCTION/MACRO INSTANCES */
void prefix_overloaded_instances(char*, char*);

/******************************************************************************
* MAIN EXECUTION OF "C OVERLOADED LENGTH ARGUMENTS" PARSER
******************************************************************************/

int main(int argc, char *argv[]) {
  // confirm passed .c file in cmd line arg to cola
  if(argc<2 || argc>3 || (argv[argc-1][strlen(argv[argc-1])-2] != '.' && argv[argc-1][strlen(argv[argc-1])-1] != 'c')) 
    cola_missing_Cfile_alert();
  // determine if displaying cola info at exit as per '-l' argv[1]
  bool show_cola_info = false;
  if(argc == 3) {
    if(strcmp(argv[1], "-l") != 0) cola_missing_Cfile_alert(); // info flag != '-l'
    show_cola_info = true;
  }

  char read[MAX_FILESIZE], write[MAX_FILESIZE], filename[100];
  FLOOD_ZEROS(read, MAX_FILESIZE); FLOOD_ZEROS(write, MAX_FILESIZE);
  FLOOD_ZEROS(filename, 100);
  strcpy(filename, argv[argc-1]);
  confirm_valid_file(filename);
  FSCRAPE(read, filename); // read file

  register_all_global_function_macro_defs(read);   // parse all global fcns/macros
  filter_overloads_from_FMACS_to_OVERLOAD_FMACS(); // filter fcns/macros, only keeping overloads
  
  if(overload_fmacs_size > 0) {
    prefix_overloaded_instances(read, write);      // prefix all overloads "COLA__<arg#>_"
    COLA_in_ASCII();
    printf("\n================================================================================\n");
    printf(" >> %s ==\033[1mCOLA\033[0m=\033[1mOVERLOAD\033[0m=> ", filename);
    strcpy(&filename[strlen(filename)-2], "_COLA.c");
    printf("%s", filename);
    printf("\n================================================================================\n\n");
    FPUT(write, filename);
  } else // no need to convert file w/o overloads
    printf("\n\033[1m>> cola.c: \033[4mNO\033[0m \033[1m\033[4mOVERLOADS\033[0m \033[1m\033[4mDETECTED\033[0m\033[1m!\033[0m\n\n");

  // show cola.c's overload/registered global fcn/macro data (if "-l" flag active)
  if(show_cola_info) {
    if(overload_fmacs_size > 0) {
      printf("\033[1m>> \033[4mOVERLOADS\033[0m\033[1m:\033[0m\n");
      for(int i = 0; i < overload_fmacs_size; ++i) {
        printf("\t%02d) \033[1m\033[4mNAME\033[0m \"%s\" \033[1m\033[4mOVERLOADS\033[0m", i + 1, overload_fmacs[i].name);      
        for(int j = 0; j < overload_fmacs[i].arg_sizes_length; ++j)
          printf(" %d", overload_fmacs[i].arg_sizes[j]);
        printf("\n");
      }
      printf("\n");
    }

    printf("\033[1m>> \033[4mALL REGISTERED GLOBAL FCNS/MACROS\033[0m\033[1m:\033[0m\n");
    for(int i = 0; i < fmacs_size; ++i)
      printf("\t%02d) \033[1m\033[4mNAME\033[0m \"%s\", \033[1m\033[4mARGS LENGTH\033[0m %d, \033[1m\033[4mIS A PROTOTYPE\033[0m %d\n", 
        i + 1, fmacs[i].name, fmacs[i].args, fmacs[i].is_a_prototype);
    printf("\n");
  }

  return 0;
}

/******************************************************************************
* STRING, CHAR, AND GLOBAL SCOPES STATUS UPDATING && STRING HELPER FUNCTIONS
******************************************************************************/

// handle incrementing/decrementing status of whether "r" currently in a string, char, or in a braced-scope
void handle_string_char_brace_scopes(bool *in_a_string, bool *in_a_char, int *in_global_scope, char *r) {
  if(!(*in_a_char)   && *r == '"'  && (*(r-1) != '\\' || *(r-2) == '\\')) *in_a_string = !(*in_a_string); // confirm whether in a string or not
  if(!(*in_a_string) && *r == '\'' && (*(r-1) != '\\' || *(r-2) == '\\')) *in_a_char   = !(*in_a_char);   // confirm whether in a char or not
  if(!(*in_a_string) && !(*in_a_char) && *r == '{') ++ *in_global_scope; // update braces scope
  if(!(*in_a_string) && !(*in_a_char) && *r == '}') -- *in_global_scope; // update braces scope
}

// returns if 'p' points to the particular substring 'substr'
bool is_at_substring(char *p, char *substr) {
  while(*substr != '\0' && *p != '\0' && *p == *substr) ++p, ++substr;
  return (*substr == '\0');
}

/******************************************************************************
* COMMENT, CONDITIONAL DIRECTIVE, & MACRO-BODY SKIP/CPY FUNCTIONS
******************************************************************************/

// if at a comment instance in "*read" skips over them (& copies to "*write" if "cpy_comments" == true)
char *cola_skip_comments(char *read, char *write, bool cpy_comments) {
  if(*read == '/' && *(read + 1) == '/') {        // single-line comment
    if(cpy_comments) *write++ = *read++, *write++ = *read++, *write++ = *read++; else read += 3; // skip/copy initial "//"
    while(*read != '\0' && (*read != '\n' || *(read-1) == '\\')) { // skip/copy comment w/o parsing
      if(cpy_comments) *write++ = *read++; else ++read; 
    }
  } else if(*read == '/' && *(read + 1) == '*') { // block comment
    if(cpy_comments) *write++ = *read++, *write++ = *read++, *write++ = *read++; else read += 3; // skip/copy initial "/*"
    while(*read != '\0' && (*read != '*' || *(read + 1) != '/')) {
      if(cpy_comments) *write++ = *read++; else ++read;            // skip/copy comment w/o parsing
    }
    if(cpy_comments) *write++ = *read++, *write++ = *read++; else read += 2;                     // skip last '*' & '/'
  }
  return read;
}

// if at a preprocessor conditional directive instance in "*read", skips over 
// (& copies to "*write" if "cpy_dirs" == true)
char *cola_skip_conditional_directives(char *read, char *write, bool cpy_dirs) {
  // conditional directives = #if, #ifdef, #ifndef, #elif, #else, & #endif
  char *scout = read - 1;
  bool in_a_string = false, in_a_char = false;
  int in_global_scope = 0, in_conditional_scope = 1;
  if(is_at_substring(read, "#if")) {
    // global outer (non-nested) conditional directives only defined after '\n' & optional whitespace sequence
    while(*scout == ' ' || *scout == '\t') --scout;
    if(*scout == '\n') { // at a conditional directive
      if(cpy_dirs) *write++ = *read++, *write++ = *read++, *write++ = *read++; else read += 3; // skip/cpy initial "#if"
      while(in_conditional_scope > 0) {
        handle_string_char_brace_scopes(&in_a_string, &in_a_char, &in_global_scope, read);
        if(!in_a_string && !in_a_char && is_at_substring(read, "#if"))         ++in_conditional_scope;
        else if(!in_a_string && !in_a_char && is_at_substring(read, "#endif")) --in_conditional_scope;
        if(in_conditional_scope <= 0) break;
        if(cpy_dirs) *write++ = *read++; else ++read;
      }
    }
  }
  return read;
}

// invoked at a macro, skips macro body's contents
char *skip_macro_body(char *read) {
  bool in_a_string = false, in_a_char = false;
  int in_global_scope = 0, in_arg_scope = 1;
  ++read; // skip initial '('
  // skip macro arguments to get to body
  while(*read != ')') {
    handle_string_char_brace_scopes(&in_a_string, &in_a_char, &in_global_scope, read);
    if(!in_a_string && !in_a_char && *read == '(')      ++in_arg_scope;
    else if(!in_a_string && !in_a_char && *read == ')') --in_arg_scope;
    if(in_arg_scope <= 0) break;
    ++read;
  }
  // skip macro body
  while(*read != '\0' && (*read != '\n' || *(read-1) == '\\')) ++read;
  return read;
}

/******************************************************************************
* "fmacs" (FUNCTION MACRO INSTANCES) STRUCT HELPER FUNCTIONS
******************************************************************************/

// checks whether exact same fcn/macro already in fmacs (w/ same arg # thus NOT an overload)
int non_prototype_duplicate_instance_in_fmacs(char *name, int total_args) {
  for(int i = 0; i < fmacs_size; ++i) 
    if(strcmp(fmacs[i].name, name) == 0 && fmacs[i].args == total_args) 
      return i;
  return -1;
}

// checks whether same fcn/macro name already in fmacs
bool overloaded_name_already_in_fmacs(char *name) {
  for(int i = 0; i < fmacs_size; ++i) 
    if(strcmp(fmacs[i].name, name) == 0) return true;
  return false;
}

// given a name of a confirmed overloaded fcn/macro instance, updates "overloaded"
// status of all its previously registered overloaded variants
void update_all_name_overloaded_status(char *name) {
  for(int i = 0; i < fmacs_size; ++i) 
    if(strcmp(fmacs[i].name, name) == 0) fmacs[i].overloaded = true;
}

// cpys the macro/fcn name from "read" to "name"
void get_fmac_name(char *read, char *name) {
  while(!VARCHAR(*read)) --read; // move to end of fcn/macro name
  while(VARCHAR(*read))  --read; // move directly prior fcn/macro name
  ++read;                        // move to beginning of fcn/macro name
  while(VARCHAR(*read)) *name++ = *read++; // copy fcn/macro name
  *name = '\0';
}

/******************************************************************************
* MACRO INSTANCE & FUNCTIONLIKENESS EVALUATION FUNCTIONS
******************************************************************************/

// given a ptr to initial '(', returns whether or not "read" name is that of a macro
bool is_at_macro_name(char *read) {
  while(!VARCHAR(*read))      --read; // skip past '('
  while(VARCHAR(*read))       --read; // skip past possible macro name
  while(IS_WHITESPACE(*read)) --read; // skip past whitespace btwn "#define" (OR fcn type) & name
  return (*read == 'e' && *(read-1) == 'n' && *(read-2) == 'i' && *(read-3) == 'f' 
    && *(read-4) == 'e' && *(read-5) == 'd' && *(read-6) == '#');
}

// confirm macro is "functionlike", ie has both a list of args btwn "()" 
// as well as a non-whitespace & non-comment "macro body" (else can't overload)
bool macro_is_functionlike(char *read) { 
  ++read; // skip initial '('
  bool in_a_string = false, in_a_char = false;
  int in_global_scope = 0, in_arg_scope = 1;
  char *ignore_arg = read;  // unused in this context
  while(in_arg_scope > 0) { // skip past possible macro args (macro body f not function-like)
    handle_string_char_brace_scopes(&in_a_string, &in_a_char, &in_global_scope, read);
    if(!in_a_string && !in_a_char && *read == '(')      ++in_arg_scope;
    else if(!in_a_string && !in_a_char && *read == ')') --in_arg_scope;
    if(in_arg_scope <= 0) break;
    ++read;
  }
  ++read; // move past last closing ')' for possible args (if functionlike, else this closes macro body)
  // confirm macro has non-whitespace & non-comment body (makes it function-like in conjunction w/ having args)
  while(*read != '\0' && (*read != '\n' || *(read-1) == '\\')) { 
    read = cola_skip_comments(read, ignore_arg, false);
    if(!IS_WHITESPACE(*read)) return true;
    if(*read != '\0' && (*read != '\n' || *(read-1) == '\\')) ++read;
  }
  return false;
}

/******************************************************************************
* "overload_fmacs" (FUNCTION MACRO OVERLOAD INSTANCES) STRUCT HELPER FUNCTIONS
******************************************************************************/

// checks whether same fcn/macro name already in overload_fmacs
bool name_not_in_overload_fmacs(char *name) {
  for(int i = 0; i < overload_fmacs_size; ++i) 
    if(strcmp(overload_fmacs[i].name, name) == 0) return false;
  return true;
}

// returns idx of fcn/macro arg-length overload instance if instance doesn't 
// already have "fmacs[fmacs_idx].args" in "arg_sizes[]", & returns -1 if 
// already found (ie if already stored its name & arg # instance from a prototype)
int overload_fmacs_instance_idx(int fmacs_idx) {
  for(int j = 0, k; j < overload_fmacs_size; ++j) 
    if(strcmp(overload_fmacs[j].name, fmacs[fmacs_idx].name) == 0) {
      // search instance's overloaded arg lengths for "fmacs[fmacs_idx].args"
      for(k = 0; k < overload_fmacs[j].arg_sizes_length; ++k)
        if(fmacs[fmacs_idx].args == overload_fmacs[j].arg_sizes[k]) break;
      // if "fmacs[fmacs_idx].args" not found, return instance idx to 
      // add # of args as a new overload to "overload_fmacs" instance's "arg_sizes[]"
      return (k == overload_fmacs[j].arg_sizes_length) ? j : -1;
    }
  // the following "return -1;" is never triggered -- would indicate "fmacs[fmacs_idx].name" 
  // isn't in the "overload_fmacs" struct, BUT this function is only triggered if the name IS in 
  // "overload_fmacs" as per the "name_not_in_overload_fmacs" function above returning "false"
  return -1; 
}

/******************************************************************************
* MESSAGING FUNCTIONS
******************************************************************************/

// "COLA" in ASCII
void COLA_in_ASCII() {
  // Color Code Source:
  // http://www.lihaoyi.com/post/BuildyourownCommandLinewithANSIescapecodes.html
  const char *NONE = "\033[0m";                      // clear syntax settings
  const char *BOLD = "\033[1m";                      // bold font
  const char *BLUE = "\033[38;5;21m";                // blue font
  const char *RONW = "\033[48;5;231m\033[38;5;196m"; // Red font ON White back
  const char *WONR = "\033[48;5;196m\033[38;5;231m"; // White font ON Red back
  printf("\n                   /|======================================|\\\n");
  printf("                   ||%s%s        %s          %s        %s          %sO %s||\n", BOLD, RONW, WONR, RONW, WONR, BLUE, NONE);
  printf("                   ||%s%s //==\\\\ %s  //==\\\\  %s |\\     %s  //^\\\\  %sOo %s||\n", BOLD, RONW, WONR, RONW, WONR, BLUE, NONE);
  printf("                   ||%s%s ||  |/ %s ||    || %s ||   _ %s |/===\\| %so  %s||\n", BOLD, RONW, WONR, RONW, WONR, BLUE, NONE);
  printf("                   ||%s%s ||     %s  \\\\==//  %s ||  || %s ||   || |\\ %s||\n", BOLD, RONW, WONR, RONW, WONR, NONE);
  printf("                   ||%s%s \\\\     %s//=====\\\\ %s \\===// %s |/   \\| // %s||\n", BOLD, RONW, WONR, RONW, WONR, NONE);
  printf("                   ||%s%s  \\\\===/%s/       \\=%s========%s========//  %s||\n", BOLD, RONW, WONR, RONW, WONR, NONE);
  printf("                   ||%s%s        %s          %s        %s            %s||\n", BOLD, RONW, WONR, RONW, WONR, NONE);
  printf("                   ||%s%s  O  V  %s  E  R  L %s  O  A  %s  D  E  D ! %s||\n", BOLD, RONW, WONR, RONW, WONR, NONE);
  printf("                   ||%s%s        %s          %s        %s            %s||\n", BOLD, RONW, WONR, RONW, WONR, NONE);
  printf("                   \\|======================================|/\n");
}

// "ERROR" in ASCII
void cola_ERROR_ascii_art() {
  printf("\n========================================\n\033[1m\033[31m");
  printf("  /|===\\ ||^\\\\ ||^\\\\ //==\\\\ ||^\\\\   //\n");
  printf("  ||==   ||_// ||_// ||  || ||_//  //\n");
  printf("  \\|===/ || \\\\ || \\\\ \\\\==// || \\\\ <*>");
  printf("\033[0m\n========================================\n");
}

// error & how-to-execute message
void cola_missing_Cfile_alert() {
  cola_ERROR_ascii_art();
  printf("** Missing .c File Cmd Line Argument! **\n");
  printf("Execution:  $ gcc -o cola cola.c    \n            $ ./cola yourFile.c");
  printf("\n========================================");
  printf("\n*** Or Else: Misused '-l' Info Flag! ***");
  printf("\n   Execution:  $ ./cola -l yourFile.c   ");
  printf("\n========================================");
  printf("\n********* Filename Conversion: *********\n"); 
  printf("     yourFile.c ==> yourFile_COLA.c     ");
  printf("\n========================================\n");
  printf(" >> Terminating COLA Processor.");
  printf("\n===============================\n\n");
  exit(EXIT_FAILURE);
}

// confirms file exists, non-empty, & takes less memory than MAX_FILESIZE
void confirm_valid_file(char *filename) {
  struct stat buf;
  if(stat(filename, &buf)) {
    cola_ERROR_ascii_art();
    fprintf(stderr, " >> FILE \"%s\" DOES NOT EXIST!\n", filename);
    fprintf(stderr, " >> Terminating COLA Processor.\n\n");
    exit(EXIT_FAILURE);
  }
  if(buf.st_size > MAX_FILESIZE || buf.st_size == 0) {
    cola_ERROR_ascii_art();
    if(buf.st_size > MAX_FILESIZE) {
      fprintf(stderr, " >> FILE \"%s\" SIZE %lld BYTES EXCEEDS %d BYTE CAP!\n",filename,buf.st_size,MAX_FILESIZE); 
      fprintf(stderr, " >> RAISE 'MAX_FILESIZE' MACRO LIMIT!\n");
    } else fprintf(stderr, " >> COLA CAN'T PROCESS AN EMPTY FILE!\n"); 
    fprintf(stderr, " >> Terminating COLA Processor.\n\n");
    exit(EXIT_FAILURE);
  }
}

// outputs "message" to stderr, notifies client of cola.c termination, & exits program
void terminate_program(char *message) {
  fprintf(stderr, " >> %s\n >> Terminating COLA Processor.\n\n", message);
  exit(EXIT_FAILURE);
}

// invoked by finding an overloaded variadic macro/fcn
void throw_fatal_error_variadic_overload(char *name) {
  fprintf(stderr, "\ncola.c: \033[1m\033[31mERROR\033[0m INVALID OVERLOADED VARIADIC FUNCTION/MACRO DETECTED!\n");
  fprintf(stderr, " >> NAME OF VARIADIC FUNCTION/MACRO: \"%s\"\n", name);
  terminate_program("VARIADIC FUNCTIONS/MACROS \033[1mCANNOT\033[0m BE OVERLOADED!");
}

// invoked by detecting overloaded fcn/macro name instance w/ an already existing number of args
void throw_fatal_error_duplicate_overload(char *name, int arg_total) {
  fprintf(stderr, "\ncola.c: \033[1m\033[31mERROR\033[0m DUPLICATE FUNCTION/MACRO ARGUMENT-LENGTH OVERLOAD DETECTED!\n");
  fprintf(stderr, " >> NAME & ARG-LENGTH DUPLICATE INSTANCE OF AN ALREADY EXISTING ARG-LENGTH OVERLOAD: \"%s\", ARG LENGTH: %d.\n", name, arg_total);
  terminate_program("FUNCTIONS/MACROS W/ THE SAME NUMBER OF ARGUMENTS \033[1mCANNOT\033[0m BE OVERLOADED!");
}

// invoked by detecting fcn/macro name invocation w/ an unaccounted for # of args (global 
// definition of overloaded fcn/macro instance w/ particular arg length not registered by "overload_fmacs")
void throw_fatal_error_undefined_arg_length_invocation(char *name, int arg_total) {
  fprintf(stderr, "\ncola.c: \033[1m\033[31mERROR\033[0m UNDEFINED OVERLOADED ARG LENGTH INVOCATION!\n");
  fprintf(stderr, " >> NO GLOBAL FUNCTION/MACRO DEFINITION OR PROTOTYPE MATCHING INVOKED # OF ARGS FOUND!\n");
  fprintf(stderr, " >> NAME & ARG-LENGTH OF UNDEFINED OVERLOAD INSTANCE: \"%s\", ARG LENGTH: %d.\n", name, arg_total);
  terminate_program("ALL FUNCTION/MACRO ARG-LENGTH OVERLOADS MUST BE INDIVIDUALLY GLOBALLY DEFINED OR PROTOTYPED!");
}

// invoked by detecting fcn/macro name invocation w/o any "args list"
void throw_fatal_error_non_functional_invocation_of_overloaded_name(char *name) {
  fprintf(stderr, "\ncola.c: \033[1m\033[31mERROR\033[0m UNDEFINED NON-FUNCTIONAL INVOCATION OF ARG LENGTH OVERLOAD (NO ANY ARGUMENTS PASSED BTWN \"()\")!\n");
  fprintf(stderr, " >> NAME OF ARGUMENT-LESS OVERLOAD INVOCATION INSTANCE: \"%s\".\n", name);
  terminate_program("OVERLOADED FUNCTION/MACRO NAMES \033[1mCANNOT\033[0m BE REDEFINED TO ANY OTHER VARIABLE IN ANY OTHER SCOPE!");
}

/******************************************************************************
* FUNCTION/MACRO ARGUMENT COUNTING FUNCTION
******************************************************************************/

// given a ptr at the opening '(', returns the # of args in a fcn/macro
int count_args(char *r, char *function_name) {
  bool in_a_string = false, in_a_char = false;
  int in_global_scope = 0; // 0 if in global scope, > 0 if in a fcn or any other braces
  int in_arg_scope = 1, arg_total = 1;
  ++r; // move "r" to 1st arg in fcn & past opening '('
  while(*r != '\0' && in_arg_scope > 0) {
    handle_string_char_brace_scopes(&in_a_string, &in_a_char, &in_global_scope, r);
    if(!in_a_string && !in_a_char && *r == '(')      ++in_arg_scope;
    else if(!in_a_string && !in_a_char && *r == ')') --in_arg_scope;
    if(in_arg_scope <= 0) break;
    if(!in_a_string && !in_a_char && *r == ',') ++arg_total; // at an arg
    if(!in_a_string && !in_a_char && is_at_substring(r, "...") 
      && overloaded_name_already_in_fmacs(function_name))
      throw_fatal_error_variadic_overload(function_name); // variadic overload fatal error
    ++r;
  }
  --r; // check whether an empty arg list
  while(IS_WHITESPACE(*r)) --r;
  return (*r == '(') ? 0 : arg_total;
}

/******************************************************************************
* FUNCTION-PROTOTYPE CHECKING FUNCTION
******************************************************************************/

// given ptr at opening '(' of fcn/macro args list, return whether at a fcn prototype
bool is_function_prototype(char *r) {
  bool in_a_string = false, in_a_char = false, overload;
  int in_global_scope = 0; // 0 if in global scope, > 0 if in a fcn or any other braces
  int in_arg_scope = 1;
  ++r; // move past initial opening '('
  while(*r != '\0' && in_arg_scope > 0) {
    handle_string_char_brace_scopes(&in_a_string, &in_a_char, &in_global_scope, r);
    if(!in_a_string && !in_a_char && *r == '(')      ++in_arg_scope;
    else if(!in_a_string && !in_a_char && *r == ')') --in_arg_scope;
    if(in_arg_scope <= 0) break;
    ++r;
  }
  ++r; // move past closing ")"
  while(IS_WHITESPACE(*r)) ++r; // skip past optional space btwn fcn args list & def
  return (*r == ';'); // prototype
}

/******************************************************************************
* MAIN ACCOUNTING FUNCTION FOR FUNCTION/MACRO DEFINITIONS IN FILE
******************************************************************************/

// register all functions & macros declared globaclly outside of conditional
// preprocessor directives, along w/ their arg number & overloaded status
// (fills "fmacs" which then gets filtered into "overload_fmacs" on return to main)
void register_all_global_function_macro_defs(char *read) {
  char *r = read, *scout, function_name[75];
  bool in_a_string = false, in_a_char = false, overload, prototype, macro;
  int in_global_scope = 0; // 0 if in global scope, > 0 if in a fcn or any other braces
  int arg_total, duplicate_overload;
  // "ignore_arg" needed to give "cola_skip_conditional_directives" appropriate arg types
  char *ignore_arg = read;

  // register all function/macro names in file to detect overloads prior to prefixing invocations
  while(*r != '\0') {
    handle_string_char_brace_scopes(&in_a_string, &in_a_char, &in_global_scope, r);
    if(!in_a_string && !in_a_char) {
      r = cola_skip_comments(r, ignore_arg, false);
      if(*r != '\0' && in_global_scope == 0) 
        r = cola_skip_conditional_directives(r, ignore_arg, false);
    }
    if(!in_a_string && !in_a_char && in_global_scope == 0 && *r == '(') { // potential function/macro
      // check if at a macro, & if so confirm its "functionlike" (can't overload non-functionlike macros)
      macro = is_at_macro_name(r);
      if(macro && !macro_is_functionlike(r)) { 
        r = skip_macro_body(r); 
        continue; 
      } 
      // copy function name
      FLOOD_ZEROS(function_name, 75);
      get_fmac_name(r, function_name);
      // get number of args for fcn/macro
      arg_total = count_args(r, function_name);
      // check whether at a function prototype
      prototype = is_function_prototype(r);
      // disregard if function name already exists w/ exact same arg # (ie already registered its prototype)
      if((duplicate_overload = non_prototype_duplicate_instance_in_fmacs(function_name, arg_total)) != -1) { 
        if(!fmacs[duplicate_overload].is_a_prototype) // throw fatal error if not a prototype
          throw_fatal_error_duplicate_overload(function_name, arg_total);
        ++r; 
        continue;
      }
      // check if function overload exists w/ same name BUT different # arg
      overload = overloaded_name_already_in_fmacs(function_name);
      if(overload) update_all_name_overloaded_status(function_name);
      // assign unique fcn/macro instance properties
      strcpy(fmacs[fmacs_size].name, function_name);
      fmacs[fmacs_size].args = arg_total;
      fmacs[fmacs_size].overloaded = overload;
      fmacs[fmacs_size].is_a_prototype = prototype;
      ++fmacs_size;
      // check if at a macro to skip over its body
      if(macro) r = skip_macro_body(r);
    }
    if(*r != '\0') ++r;
  }
}

/******************************************************************************
* FILTER OVERLOADED MACROS FROM "FMACS" INTO "OVERLOAD_FMACS" STRUCTURE
******************************************************************************/

// convert "fmacs" struct to "overload_fmacs" struct (disregarding non-overloaded fcns)
void filter_overloads_from_FMACS_to_OVERLOAD_FMACS() {
  int i, overload_idx;
  for(i = 0; i < fmacs_size; ++i) {
    if(fmacs[i].overloaded && name_not_in_overload_fmacs(fmacs[i].name)) {
      // new function/macro name overload instance
      strcpy(overload_fmacs[overload_fmacs_size].name, fmacs[i].name);
      overload_fmacs[overload_fmacs_size].arg_sizes[0] = fmacs[i].args;
      overload_fmacs[overload_fmacs_size].arg_sizes_length = 1;
      ++ overload_fmacs_size;
    } else if(fmacs[i].overloaded && (overload_idx = overload_fmacs_instance_idx(i)) != -1) {
      // existing function/macro name overload has a new arg-length overload instance
      overload_fmacs[overload_idx].arg_sizes[overload_fmacs[overload_idx].arg_sizes_length] = fmacs[i].args;
      ++ overload_fmacs[overload_idx].arg_sizes_length;
    }
  }
}

/******************************************************************************
* PREFIX ALL OVERLOADED FUNCTION/MACRO INSTANCES
******************************************************************************/

// prefix all invocation/declaration/definition instances of file's registered
// overloaded fcn/macro names & arg lengths (stored in "overload_fmacs" struct)
void prefix_overloaded_instances(char *read, char *write) {
  char *r = read, *w = write, *scout;
  bool in_a_string = false, in_a_char = false;
  int i, j, arg_total, ignore_arg; // "global scope" irrelevant here thus now denoted as "ignore_arg"

  // prefix every overloaded instance w/ "COLA__<No_of_Args>_"
  while(*r != '\0') {
    // account for scopes
    handle_string_char_brace_scopes(&in_a_string, &in_a_char, &ignore_arg, r);
    if(!in_a_string && !in_a_char) {
      r = cola_skip_comments(r, w, true), w += strlen(w);
      if(!in_a_string && !in_a_char && *r != '\0') 
        r = cola_skip_conditional_directives(r, w, true), w += strlen(w);
    }
    // check for potential fcn/macro overload
    if(!in_a_string && !in_a_char && VARCHAR(*r) && !VARCHAR(*(r-1))) {
      for(i = 0; i < overload_fmacs_size; ++i) {
        // potential overloaded instance
        if(is_at_substring(r, overload_fmacs[i].name) && !VARCHAR(*(r+strlen(overload_fmacs[i].name)))) {
          // get number of args
          scout = r;
          while(VARCHAR(*scout))       ++scout; // skip past name
          while(IS_WHITESPACE(*scout)) ++scout; // skip past optional whitespace btwn name && '('
          // overload name invocation w/o any args passed btwn "()" afterwards -- COLA names
          // can NEVER to redefined/reassigned to ANY other variables in ANY other scope
          if(*scout != '(')
            throw_fatal_error_non_functional_invocation_of_overloaded_name(overload_fmacs[i].name);
          arg_total = count_args(scout, overload_fmacs[i].name);
          // confirm an overload instance with "arg_total" args was detected earlier
          for(j = 0; j < overload_fmacs[i].arg_sizes_length; ++j)
            if(arg_total == overload_fmacs[i].arg_sizes[j])
              break;
          // if overloaded instance invocation has an undefined # of args wrt "overload_fmacs",
          // ie no matching global definition/declaration/prototype found to couple w/ invocation
          if(j == overload_fmacs[i].arg_sizes_length)
            throw_fatal_error_undefined_arg_length_invocation(overload_fmacs[i].name, arg_total);
          // prefix overloaded fcn/macro instance's name w/ reserved header & arg number
          sprintf(w, "COLA__%d_%s", arg_total, overload_fmacs[i].name);
          w += strlen(w);
          r = scout;
          break;
        }
      }
      if(i == overload_fmacs_size && *r != '\0') *w++ = *r++;
    } else
      *w++ = *r++;
  }
  *w = '\0';
}
