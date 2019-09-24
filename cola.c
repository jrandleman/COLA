// AUTHOR: JORDAN RANDLEMAN -- cola.c -- C OVERLOADED LENGTH ARGUMENTS:
// PRE-PREPROCESS .C FILES OVERLOADING FCNS & MACROS BY THEIR # OF ARGS
// & .C FILES ASSIGNING THEIR FCNS DEFAULT VALUES
// Email jrandleman@scu.edu or see https://github.com/jrandleman for support
/**
 * compile: $ gcc -std=c99 -o cola cola.c
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
#define MAX_FILESIZE 1000001                                       // 1 megabyte + '\0'
#define MAX_TOTAL_NUMBER_OF_FUNCTIONS_AND_MACROS_IN_PROGRAM 5000   // max # of macros & fcns parsed file can have
#define MAX_UNIQUE_OVERLOADS_PER_OVERLOADED_FCN_MACRO_INSTANCE 100 // max # of COLA's per overloaded fcn/macro name
#define MAX_LENGTH_PER_ARG 1001    // max # of characters per fcn arg list parsed file can have
#define MAX_ARGS_PER_FCN 251       // max # args per fcn arg list parsed file can have
#define MAX_TOKEN_NAME_LENGTH 150  // max length any single label can have in the parsed file (name of fcn, variable, arg, etc)
#define BAD_CODE_BUFFER_LENGTH 251 // portion of code shown to user at error

/*****************************************************************************
 ||^\\ //=\\ //=\ /| |\ /\\  //\ /|==\ /\\ ||==== //^\\ ==== ==== //=\\ /\\ ||
 ||  ))|| || ||   || || ||\\//|| ||=   ||\\|| || |/===\| ||   ||  || || ||\\||
 ||_// \\=// \\=/ \\=// || \/ || \|==/ || \// || ||   || ||  ==== \\=// || \//
 *****************************************************************************
 *                         -:- COLA.C 9 CAVEATS -:-                         *
 *   => "COLA INSTANCE" = "fcn/macro overload OR fcn w/ default arg values" *
 *   (0) "COLA_" PREFIX IS RESERVED                                         *
 *   (1) NO VARIADIC COLA INSTANCES                                         *
 *   (2) NO FCN PTRS POINTING TO COLA INSTANCES                             *
 *       (*) can't determine overloaded arg # from only overloaded fcn name *
 *   (3) NO REDEFINING COLA INSTANCE NAME TO OTHER VARS REGARDLESS OF SCOPE *
 *   (4) NO OVERLOADED MACROS CAN EVER BE "#undef"'d                        *
 *   (5) ONLY COLA INSTANCES DEFINED/PROTOTYPED GLOBALLY WILL BE RECOGNIZED *
 *   (6) ONLY FUNCTIONS MAY BE ASSIGNED DEFAULT VALUES - NEVER MACROS!      *
 *   (7) NO ARG W/ A DEFAULT VALUE MAY PRECEDE AN ARG W/O A DEFAULT VALUE   *
 *       (*) args w/ default values must always by last in a fcn's arg list *
 *   (8) FCN PROTOTYPES TAKE PRECEDENT OVER DEFINITIONS WRT DEFAULT VALS    *
 *       (*) if a fcn proto has default vals but its defn doesn't (or vise  *
 *           versa) fcn will be treated as if both had the default vals     *
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

/* GLOBAL FILE FCN/MACRO & OVERLOADING TRACKING STRUCTURES */
// holds all global fcn & macro def's found in file
struct function_macro_instance {
  char name[MAX_TOKEN_NAME_LENGTH]; // defined fcn/macro name
  int args;            // number of args
  bool overloaded;     // whether its overloaded
  bool is_a_prototype; // whether is a function prototype
  bool is_a_macro;     // whether is a functionlike macro
} fmacs[MAX_TOTAL_NUMBER_OF_FUNCTIONS_AND_MACROS_IN_PROGRAM];
int fmacs_size = 0;
// holds function & macro overload instances, derived from "fmacs"
struct function_macro_overload_instance {
  char name[MAX_TOKEN_NAME_LENGTH]; // overoaded fcn/macro name
  int arg_sizes[MAX_UNIQUE_OVERLOADS_PER_OVERLOADED_FCN_MACRO_INSTANCE];   // different arg lengths per overload
  int arg_sizes_length; // # of other overoaded fcn/macros w/ same name
  bool is_a_macro[MAX_UNIQUE_OVERLOADS_PER_OVERLOADED_FCN_MACRO_INSTANCE]; // whether each overload is a functionlike macro
} overload_fmacs[MAX_TOTAL_NUMBER_OF_FUNCTIONS_AND_MACROS_IN_PROGRAM];
int overload_fmacs_size = 0;

/* GLOBAL FILE FCN-WITH-DEFAULT-VALUE-ARGS TRACKING STRUCTURES */
// holds all global fcn def's found in file with default arg values
struct function_with_default_value_instance {
  char fcn_name[MAX_TOKEN_NAME_LENGTH]; // fcn instance name
  int total_args;    // fcn's total args (both dflt & not)
  int total_dflts;   // fcn's total dflt args
  int dflt_idxs[MAX_ARGS_PER_FCN]; // fcn's dflt arg idxs wrt its arg list
  char dflt_vals[MAX_ARGS_PER_FCN][MAX_LENGTH_PER_ARG];      // fcn's dflt values
} all_dflt_fcns[MAX_TOTAL_NUMBER_OF_FUNCTIONS_AND_MACROS_IN_PROGRAM]; // struct to hold all dflt-valued fcn instances
int all_dflt_fcns_size = 0; // total functions registered
// holds defaulted-arg function instances, derived from "all_dflt_fcns"
struct unique_function_default_values {
  char name[MAX_TOKEN_NAME_LENGTH]; // defaulted-arg (& possibly overoaded) fcn name
  int arg_sizes_length; // # of other overoaded fcns w/ same name
  int arg_sizes[MAX_UNIQUE_OVERLOADS_PER_OVERLOADED_FCN_MACRO_INSTANCE];   // arg length(s) (> 1 if overloaded)
  int total_dflts[MAX_UNIQUE_OVERLOADS_PER_OVERLOADED_FCN_MACRO_INSTANCE]; // total arg dflt(s) (> 1 if overloaded)
  int dflt_idxs[MAX_UNIQUE_OVERLOADS_PER_OVERLOADED_FCN_MACRO_INSTANCE][MAX_ARGS_PER_FCN]; // arg dflt idx(s) (> 1 if overloaded)
  char dflt_vals[MAX_UNIQUE_OVERLOADS_PER_OVERLOADED_FCN_MACRO_INSTANCE][MAX_ARGS_PER_FCN][MAX_LENGTH_PER_ARG]; // arg dflt val(s) (> 1 if overloaded)
} unique_dflt_fcns[MAX_TOTAL_NUMBER_OF_FUNCTIONS_AND_MACROS_IN_PROGRAM];   // dflt-arg fcn instance
int unique_dflt_fcns_size = 0; // total unique fcn names associated w/ having 1+ dflt args

/* STRING, CHAR, AND GLOBAL SCOPES STATUS UPDATING && STRING HELPER FUNCTIONS */
void handle_string_char_brace_scopes(bool*, bool*, int*, char*);
bool is_at_substring(char*, char*);
/* COMMENT & MACRO-BODY SKIP/CPY FUNCTIONS */
char *cola_skip_comments(char*, char*);
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
/* O/P ALIGNMENT SPACE-PADDING FUNCTIONS */
int max_fmacs_name_length();
int max_overload_fmacs_name_length();
int max_dflts_name_length();
void print_space_padding(int, int);
/* MESSAGING FUNCTIONS */
void COLA_in_ASCII();
void cola_ERROR_ascii_art();
void cola_missing_Cfile_alert();
void confirm_no_reserved_COLA_prefix(char*);
void confirm_valid_file(char*);
void get_invalid_code_snippet(const char*, const char*, char []);
void terminate_program(char*);
void throw_fatal_error_variadic_overload(char*, char*, int);
void throw_fatal_error_duplicate_overload(char*, int, char*, int);
void throw_fatal_error_undefined_arg_length_invocation(char*, int, char*, int);
void throw_fatal_error_non_functional_invocation_of_overload_or_dflt_name(char*, char*, int);
void throw_fatal_error_invalid_default_arg_value(char*, bool, int);
void throw_fatal_error_ambiguous_overload(char*, int, int, int, int, int);
/* FUNCTION/MACRO ARGUMENT COUNTING FUNCTION */
int count_args(char*, char*, bool, int);
/* FUNCTION-PROTOTYPE CHECKING FUNCTION */
bool is_function_prototype(char*);
/* MAIN ACCOUNTING FUNCTION FOR FUNCTION/MACRO DEFINITIONS IN FILE */
void register_all_global_function_macro_defs(char*);
/* FILTER OVERLOADED FCNS/MACROS FROM "FMACS" INTO "OVERLOAD_FMACS" STRUCTURE */
void filter_overloads_from_FMACS_to_OVERLOAD_FMACS();
/* PREFIX FUNCTION/MACRO OVERLOAD INSTANCES & SPLICE IN DEFAULT FUNCTION VALS */
void prefix_overloaded_and_splice_default_value_instances(char*, char*);
/* DEFAULT VALUE "unique_dflt_fcns" (HAS DFLT FCN INSTANCES) STRUCT HELPER FCNS */
bool name_not_in_unique_dflt_fcns(char*);
int unique_dflt_fcns_instance_idx(int);
int total_dflts_for_ARG_SIZE_fcn_instance_in_UNIQUE_DFLT_FCNS(int, int);
/* DEFAULT VALUE FILTER FCN FROM "ALL_DFLT_FCNS" INTO "UNIQUE_DFLT_FCNS" STRUCT */
void filter_defaulted_fcns_from_ALL_DFLT_FCNS_to_UNIQUE_DFLT_FCNS();
/* DEFAULT VALUE FCNS CONFIRMING DFLT != PRIOR NON-DFLT, IN MACRO, OR AMBIGUOUS */
void confirm_no_ambiguous_overloaded_default_values();
bool ambiguous_overload(int, int, int, int);
void confirm_valid_default_values_position(int, char*, bool);
/* DEFAULT VALUE STORING IN "all_dflt_fcns" & WHITING-OUT FROM ARG LIST FUNCTION */
int store_dflt_value(int, char*);

/******************************************************************************
* MAIN EXECUTION OF "C OVERLOADED LENGTH ARGUMENTS" PARSER
******************************************************************************/

int main(int argc, char *argv[]) {
  // confirm passed .c file in cmd line arg to cola
  if(argc<2 || argc>3 || argv[argc-1][strlen(argv[argc-1])-2] != '.' || argv[argc-1][strlen(argv[argc-1])-1] != 'c') 
    cola_missing_Cfile_alert();
  // determine if displaying cola info at exit as per '-l' argv[1]
  bool show_cola_info = false;
  if(argc == 3) {
    if(strcmp(argv[1], "-l") != 0) cola_missing_Cfile_alert(); // info flag != '-l'
    show_cola_info = true;
  }

  char read[MAX_FILESIZE], write[MAX_FILESIZE], filename[100];
  const char *bold_underline = "\033[1m\033[4m";
  FLOOD_ZEROS(read, MAX_FILESIZE); FLOOD_ZEROS(write, MAX_FILESIZE);
  FLOOD_ZEROS(filename, 100);
  strcpy(filename, argv[argc-1]);
  confirm_valid_file(filename);
  FSCRAPE(read, filename); // read file
  confirm_no_reserved_COLA_prefix(read);

  register_all_global_function_macro_defs(read);    // parse all global fcns/macros
  filter_defaulted_fcns_from_ALL_DFLT_FCNS_to_UNIQUE_DFLT_FCNS(); // filter dflt-arg'd fcns, grouping together any overloads
  filter_overloads_from_FMACS_to_OVERLOAD_FMACS();  // filter fcns/macros, only keeping overloads
  confirm_no_ambiguous_overloaded_default_values(); // confirm no dflt'd fcn & possible overload invocation signatures ambiguity
  
  bool found_overloads = (overload_fmacs_size > 0);
  bool found_defaults  = (unique_dflt_fcns_size > 0);

  if(found_overloads || found_defaults) {
    prefix_overloaded_and_splice_default_value_instances(read, write); // prefix all overloads "COLA__<arg#>_"
    COLA_in_ASCII();
    printf("\n================================================================================\n");
    printf(" >> %s ==\033[1mCOLA\033[0m=\033[1mOVERLOAD\033[0m=> ", filename);
    strcpy(&filename[strlen(filename)-2], "_COLA.c");
    printf("%s", filename);
    printf("\n================================================================================\n\n");
    FPUT(write, filename);
  } else // no need to convert file w/o overloads
    printf("\n\033[1m>> cola.c: \033[4mNO\033[0m %sOVERLOADS\033[0m %sOR\033[0m %sDEFAULTS\033[0m %sDETECTED\033[0m\033[1m!\033[0m\n\n", 
      bold_underline, bold_underline, bold_underline, bold_underline);

  if(show_cola_info) {
    int max = 0, i = 0, j = 0, k = 0;
    if(found_overloads) {
      max = max_overload_fmacs_name_length();
      printf("\033[1m>> \033[4mOVERLOADS\033[0m\033[1m:\033[0m\n");
      for(i = 0; i < overload_fmacs_size; ++i) {
        printf("   %02d) \033[1m\033[4mNAME\033[0m \"%s\"", i + 1, overload_fmacs[i].name);
        print_space_padding(strlen(overload_fmacs[i].name), max);
        printf(" \033[1m\033[4mOVERLOAD ARG LENGTHS\033[0m");
        for(j = 0; j < overload_fmacs[i].arg_sizes_length; ++j) {
          printf(" %d", overload_fmacs[i].arg_sizes[j]);
          if(j < overload_fmacs[i].arg_sizes_length - 1) printf(",");
        }
        printf("\n");
      }
    }

    if(found_defaults) {
      max = max_dflts_name_length();
      printf("\n\033[1m>> \033[4mDEFAULTS\033[0m\033[1m:\033[0m\n");
      for(i = 0; i < unique_dflt_fcns_size; ++i) {
        printf("   %02d) \033[1m\033[4mNAME\033[0m \"%s\"", i + 1, unique_dflt_fcns[i].name);
        print_space_padding(strlen(unique_dflt_fcns[i].name), max);
        printf(" \033[1m\033[4mDEFAULT FCN ARG VALUES\033[0m \033[1m(\033[4mFcn1\033[0m\033[1m)\033[0m");
        for(j = 0; j < unique_dflt_fcns[i].arg_sizes_length; ++j) {
          for(k = 0; k < unique_dflt_fcns[i].total_dflts[j]; ++k) {
            printf(" %s", unique_dflt_fcns[i].dflt_vals[j][k]);
            if(k < unique_dflt_fcns[i].total_dflts[j] - 1) printf(",");
          }
          if(j < unique_dflt_fcns[i].arg_sizes_length - 1) printf(", \033[1m(\033[4mFcn2\033[0m\033[1m)\033[0m");
        }
        printf("\n");
      }
    }

    max = max_fmacs_name_length();
    printf("\n\033[1m>> \033[4mALL REGISTERED GLOBAL FCNS/MACROS\033[0m\033[1m:\033[0m\n");
    for(i = 0; i < fmacs_size; ++i) {
      printf("   %02d) \033[1m\033[4mNAME\033[0m \"%s\", ", i + 1, fmacs[i].name);
      print_space_padding(strlen(fmacs[i].name), max);
      printf("\033[1m\033[4mIS PROTOTYPE\033[0m %d, \033[1m\033[4mTOTAL ARGS\033[0m %d\n", 
        fmacs[i].is_a_prototype, fmacs[i].args);
    }
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
* COMMENT & MACRO-BODY SKIP/CPY FUNCTIONS
******************************************************************************/

// if at a comment instance in "*read" skips over them (& copies to "*write" "write" != NULL)
char *cola_skip_comments(char *read, char *write) {
  bool cpy_comments = (write != NULL);            // otherwise only skip over
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
  while(in_arg_scope > 0) { // skip past possible macro args (macro body if not function-like)
    handle_string_char_brace_scopes(&in_a_string, &in_a_char, &in_global_scope, read);
    if(!in_a_string && !in_a_char && *read == '(')      ++in_arg_scope;
    else if(!in_a_string && !in_a_char && *read == ')') --in_arg_scope;
    if(in_arg_scope <= 0) break;
    ++read;
  }
  ++read; // move past last closing ')' for possible args (if functionlike, else this closes macro body)
  // confirm macro has non-whitespace & non-comment body (makes it function-like in conjunction w/ having args)
  while(*read != '\0' && (*read != '\n' || *(read-1) == '\\')) { 
    read = cola_skip_comments(read, NULL);
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
* O/P ALIGNMENT SPACE-PADDING FUNCTIONS
******************************************************************************/

// get the max fcn/macro name length in "fmacs" struct
int max_fmacs_name_length() {
  int max = 0, length;
  for(int i = 0; i < fmacs_size; ++i)
    if((length = strlen(fmacs[i].name)) > max) max = length;
  return max;
}

// get the max fcn/macro name length in "overload_fmacs" struct
int max_overload_fmacs_name_length() {
  int max = 0, length;
  for(int i = 0; i < fmacs_size; ++i)
    if((length = strlen(overload_fmacs[i].name)) > max) max = length;
  return max;
}

// get the max fcn name length in "unique_dflt_fcns" struct
int max_dflts_name_length() {
  int max = 0, length;
  for(int i = 0; i < unique_dflt_fcns_size; ++i)
    if((length = strlen(unique_dflt_fcns[i].name)) > max) max = length;
  return max;
}

// print spaces to pad btwn the 1st column of names & the 2nd data column to align output
void print_space_padding(int length, int max) {
  for(int i = length; i < max; ++i) printf(" ");
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
  printf("Execution: $ gcc -std=c99 -o cola cola.c\n           $ ./cola yourFile.c");
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

// parses file to confirm reserved "COLA_" prefix was not used to name any token
void confirm_no_reserved_COLA_prefix(char *file_buffer) {
  bool in_a_string = false, in_a_char = false, found_reserved_prefix = false;
  char *p = file_buffer, bad_code_buffer[BAD_CODE_BUFFER_LENGTH];
  int ignore_arg = 0; // "global scope" irrelevant here thus denoted as "ignore_arg"
  while(*p != '\0') {
    handle_string_char_brace_scopes(&in_a_string, &in_a_char, &ignore_arg, p);
    if(!in_a_string && !in_a_char) p = cola_skip_comments(p, NULL);
    if(!in_a_string && !in_a_char && (p == file_buffer || !VARCHAR(*(p - 1))) && is_at_substring(p, "COLA_")) {
      found_reserved_prefix = true;
      break;
    }
    ++p;
  }
  if(found_reserved_prefix) {
    fprintf(stderr, "\033[1mcola.c:%03d: \033[31mERROR\033[0m\033[1m DETECTED RESERVED \"COLA_\" PREFIX IN FILE!\033[0m\n >> FOUND HERE:", __LINE__);
    FLOOD_ZEROS(bad_code_buffer, BAD_CODE_BUFFER_LENGTH);
    get_invalid_code_snippet(file_buffer, p, bad_code_buffer);
    fprintf(stderr, "\n\033[1m<CODE_SNIPPET>\n\033[7m%s\033[0m\n\033[1m</CODE_SNIPPET>\033[0m\n\n", bad_code_buffer);
    fprintf(stderr, " >> RMV THE RESERVED PREFIX FROM FILE (AS PER CAVEAT #0 SO WAY TO GO READING THE DOCUMENTATION) TO PARSE FILE W/ COLA.C!\n");
    fprintf(stderr, " >> Terminating COLA Processor.\n");
    exit(EXIT_FAILURE);
  }
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

// fills "bad_code_buffer" with a snippet of the client's invalid code to show in an error message
void get_invalid_code_snippet(const char *absolute_array_start, const char *scout, char bad_code_buffer[]) {
  int j;
  char *bcb_ptr = bad_code_buffer;
  // move back then copy the invalid code snippet to show in error message
  for(j = 0; scout != absolute_array_start && j < 50; ++j) --scout;
  for(j = 0; scout != '\0' && j < BAD_CODE_BUFFER_LENGTH - 51; ++j) // cpy faulty code snippet
    *bcb_ptr++ = *scout++; 
  *bcb_ptr = '\0';
}

// outputs "message" to stderr, notifies client of cola.c termination, & exits program
void terminate_program(char *message) {
  fprintf(stderr, " >> %s\n >> Terminating COLA Processor.\n\n", message);
  exit(EXIT_FAILURE);
}

// invoked by finding an overloaded variadic macro/fcn
void throw_fatal_error_variadic_overload(char *name, char *bad_code_buffer, int line_No) {
  fprintf(stderr, "\033[1m\ncola.c:%03d: \033[31mERROR:\033[0m\033[1m INVALID VARIADIC FCN/MACRO OVERLOAD DETECTED!\n\033[0m", line_No);
  fprintf(stderr, " >> NAME OF VARIADIC FCN/MACRO: \"%s\"\n", name);
  fprintf(stderr, " >> SNIPPET OF THE FILE'S INVALID VARIADIC OVERLOAD:\n");
  fprintf(stderr, "\n\033[1m<CODE_SNIPPET>\n\033[7m%s\033[0m\n\033[1m</CODE_SNIPPET>\033[0m\n\n", bad_code_buffer);
  terminate_program("VARIADIC FCNS/MACROS \033[1mCANNOT\033[0m BE OVERLOADED!");
}

// invoked by detecting overloaded fcn/macro name instance w/ an already existing number of args
void throw_fatal_error_duplicate_overload(char *name, int arg_total, char *bad_code_buffer, int line_No) {
  fprintf(stderr, "\033[1m\ncola.c:%03d: \033[31mERROR:\033[0m\033[1m DUPLICATE FCN/MACRO ARGUMENT-LENGTH OVERLOAD DETECTED!\n\033[0m", line_No);
  fprintf(stderr, " >> NAME & ARG-LENGTH DUPLICATE INSTANCE OF AN ALREADY EXISTING ARG-LENGTH OVERLOAD: \"%s\", ARG LENGTH: %d.\n", name, arg_total);
  fprintf(stderr, " >> SNIPPET OF THE FILE'S DUPLICATE FCN/MACRO ARGUMENT-LENGTH OVERLOAD:\n");
  fprintf(stderr, "\n\033[1m<CODE_SNIPPET>\n\033[7m%s\033[0m\n\033[1m</CODE_SNIPPET>\033[0m\n\n", bad_code_buffer);
  terminate_program("FUNCTIONS/MACROS W/ THE SAME NUMBER OF ARGUMENTS \033[1mCANNOT\033[0m BE OVERLOADED!");
}

// invoked by detecting fcn/macro name invocation w/ an unaccounted for # of args (global 
// definition of overloaded fcn/macro instance w/ particular arg length not registered by "overload_fmacs")
void throw_fatal_error_undefined_arg_length_invocation(char *name, int arg_total, char *bad_code_buffer, int line_No) {
  fprintf(stderr, "\033[1m\ncola.c:%03d: \033[31mERROR:\033[0m\033[1m UNDEFINED OVERLOADED ARG LENGTH INVOCATION!\n\033[0m", line_No);
  fprintf(stderr, " >> NO GLOBAL FCN/MACRO DEFINITION OR PROTOTYPE MATCHED THE # OF ARGS FOUND IN THE INVOCATION!\n");
  fprintf(stderr, " >> NAME & ARG-LENGTH OF UNDEFINED OVERLOAD INSTANCE: \"%s\", ARG LENGTH: %d.\n", name, arg_total);
  fprintf(stderr, " >> SNIPPET OF THE FILE'S UNDEFINED OVERLOADED ARG LENGTH INVOCATION:\n");
  fprintf(stderr, "\n\033[1m<CODE_SNIPPET>\n\033[7m%s\033[0m\n\033[1m</CODE_SNIPPET>\033[0m\n\n", bad_code_buffer);
  terminate_program("ALL FCN/MACRO ARG-LENGTH OVERLOADS MUST BE GLOBALLY DEFINED OR PROTOTYPED!");
}

// invoked by detecting fcn/macro name invocation w/o any "args list"
void throw_fatal_error_non_functional_invocation_of_overload_or_dflt_name(char *name, char *bad_code_buffer, int line_No) {
  fprintf(stderr, "\033[1m\ncola.c:%03d: \033[31mERROR:\033[0m\033[1m REDEFINITION OF OVERLOADED/DEFAULT FCN/MACRO ", line_No);
  fprintf(stderr, "(DETECTED A NON-FUNCTIONAL INVOCATION W/O ARGS & \"()\")!\n\033[0m");
  fprintf(stderr, " >> NAME OF ARG-LESS REDEFINED OVERLOAD/DEFAULT-ARG-VAL INVOCATION INSTANCE: \"%s\".\n", name);
  fprintf(stderr, " >> SNIPPET OF THE FILE'S INVALID INVOCATION INSTANCE:\n");
  fprintf(stderr, "\n\033[1m<CODE_SNIPPET>\n\033[7m%s\033[0m\n\033[1m</CODE_SNIPPET>\033[0m\n\n", bad_code_buffer);
  terminate_program("DEFAULT-ARG-VAL FCNS & OVERLOADED FCN/MACRO NAMES \033[1mCANNOT\033[0m BE REDEFINED TO ANY OTHER VARIABLE/FCN-PTR IN ANY OTHER SCOPE!");
}

// invoked by finding a default value prior to a non-default value in a function's arg list
void throw_fatal_error_invalid_default_arg_value(char *function_name, bool is_a_macro, int line_No) {
  fprintf(stderr, "\033[1m\ncola.c:%03d: \033[31mERROR:\033[0m\033[1m INVALID DEFAULT ARG ASSIGNMENT!\n\033[0m", line_No);
  if(is_a_macro) {
    fprintf(stderr, " >> DEFAULT VALUE FOR MACRO \"%s\" DETECTED!\n", function_name);
    terminate_program("ONLY FCNS CAN TAKE DEFAULT VALUES, MACROS \033[1mCANNOT\033[0m!");
  }
  fprintf(stderr, " >> DETECTED A DEFAULTED ARG PRECEDING A NON-DEFAULTED ARG IN ARG LIST FOR FCN \"%s\"!\n", function_name);
  terminate_program("ONLY THE LAST ARGS IN A FCN'S ARG LIST CAN BE DEFAULTED, DEFAULTED ARGS \033[1mCANNOT\033[0m PRECEDE NON-DEFAULTED ARGS!");
}

// invoked by finding an ambigious invocation signature btwn 2 overloaded fcn instances wrt the implmentation of default values
void throw_fatal_error_ambiguous_overload(char *function_name, int fcn1_dflts, int fcn1_args, int fcn2_args, int fcn2_dflts, int line_No) {
  fprintf(stderr, "\033[1m\ncola.c:%03d: \033[31mERROR:\033[0m\033[1m AMBIGUOUS OVERLOAD WITH DEFAULT ARG ASSIGNMENT!\n\033[0m", line_No);
  fprintf(stderr, " >> FCN \"%s\" OVERLOAD & DFLT VALUES MAKE ITS INVOCATION SIGNATURE AMBIGUOUS WRT ITS OTHER OVERLOADS!\n", function_name);
  fprintf(stderr, " >> DATA FOR THE 2 OVERLOADED FCN INSTANCES W/ CONFLICTING AMBIGUOUS INVOCATIONS:\n    OVERLOAD 1:\n\t    ");
  fprintf(stderr, "TOTAL ARGS = %d, TOTAL NON-DFLT ARGS = %d, TOTAL DFLT ARGS = %d", fcn1_args, fcn1_args - fcn1_dflts, fcn1_dflts);
  fprintf(stderr, "\n    OVERLOAD 2:\n\t    TOTAL ARGS = %d, TOTAL NON-DFLT ARGS = %d, TOTAL DFLT ARGS = %d\n", 
    fcn2_args, fcn2_args - fcn2_dflts, fcn2_dflts);
  fprintf(stderr, " >> ANY OVERLOADED FCN W/ DEFAULT ARG VALUES MUST BE DISCERNABLE BY ADHERING TO EITHER OF THE FOLLOWING:\n");
  fprintf(stderr, "    (1) HAVING ITS \"TOTAL NON-DFLT ARGS\" EXCEED THE \"TOTAL ARGS\" OF ALL OTHER OVERLOADED FCN INSTANCES\n");
  fprintf(stderr, "    (2) HAVING ITS \"TOTAL ARGS\" BE LESS THAN THE \"TOTAL NON-DFLT ARGS\" OF ALL OTHER OVERLOADED FCN INSTANCES\n");
  terminate_program("EITHER ALTER \"OVERLOAD 2\"'S \"TOTAL ARGS\" # OR RMV ITS ARG(S) DEFAULT VALUE TO HAVE A UNIQUE INVOCATION\n    SIGNATURE!");
}

/******************************************************************************
* FUNCTION/MACRO ARGUMENT COUNTING FUNCTION
******************************************************************************/

// given a ptr at the opening '(', returns the # of args in a fcn/macro
// & finds default values to be passed to & stored by the "store_dflt_value" fcn
int count_args(char *r, char *function_name, bool is_a_macro, int overload_fmacs_index) {
  char bad_code_buffer[BAD_CODE_BUFFER_LENGTH]; // used to show snippet of client's code at error
  char *absolute_start = r;
  // default-arg-value handling variables
  all_dflt_fcns[all_dflt_fcns_size].total_dflts = 0; // total defaults found so far is 0 (just started parsing)
  strcpy(all_dflt_fcns[all_dflt_fcns_size].fcn_name, function_name);
  FLOOD_ZEROS(all_dflt_fcns[all_dflt_fcns_size].dflt_idxs, MAX_ARGS_PER_FCN);
  int default_value_length; // length of default arg value to white out
  // arg-counting variables 
  bool in_a_string = false, in_a_char = false;
  int in_global_scope = 0; // 0 if in global scope, > 0 if in a fcn or any other braces
  int in_arg_scope = 1, arg_total = 1;
  ++r; // move "r" to 1st arg in fcn & past opening '('
  while(*r != '\0' && in_arg_scope > 0) {
    handle_string_char_brace_scopes(&in_a_string, &in_a_char, &in_global_scope, r);
    if(!in_a_string && !in_a_char) r = cola_skip_comments(r, NULL);
    if(!in_a_string && !in_a_char && *r == '(')      ++in_arg_scope;
    else if(!in_a_string && !in_a_char && *r == ')') --in_arg_scope;
    if(*r == '\0' || in_arg_scope <= 0) break;
    if(!in_a_string && !in_a_char && *r == ',') ++arg_total; // at an arg
    if(!in_a_string && !in_a_char && is_at_substring(r, "...") 
      && overloaded_name_already_in_fmacs(function_name)) {
      FLOOD_ZEROS(bad_code_buffer, BAD_CODE_BUFFER_LENGTH);
      get_invalid_code_snippet(absolute_start, r, bad_code_buffer);
      throw_fatal_error_variadic_overload(function_name, bad_code_buffer, __LINE__); // variadic overload fatal error
    }
    // at a defaulted arg, store its value & whiteout from 'r'
    if(*r == '=' && overload_fmacs_index == -1) { // in 1st round: parsing
      default_value_length = store_dflt_value(arg_total - 1, r);
      while(*r != '\0' && default_value_length > 0) *r++ = ' ', --default_value_length;
      continue;
    }
    ++r;
  }
  --r; // check whether an empty arg list
  while(IS_WHITESPACE(*r)) --r;

  // the 2nd round of splicing in overloaded prefixes can't tell us if at a macro or fcn,
  // thus must compare the overloaded name w/ the number of registered args in order to 
  // determine whether or not currently at a macro
  if(overload_fmacs_index > -1) // in 2nd round of splicing, not parsing overloads
    for(int i = 0; i < overload_fmacs[overload_fmacs_index].arg_sizes_length; ++i)
      if(overload_fmacs[overload_fmacs_index].arg_sizes[i] == arg_total) {
        is_a_macro = overload_fmacs[overload_fmacs_index].is_a_macro[i];
        break;
      }
  if(overload_fmacs_index == -1) { // in 1st round: parsing
    // throw an error if any defaulted args found prior a non-defaulted arg OR in a macro
    confirm_valid_default_values_position(arg_total, function_name, is_a_macro); 
    // register total # of args & increment fcn counter having completed parsing a fcn arg-list instance
    all_dflt_fcns[all_dflt_fcns_size].total_args = arg_total;
    ++ all_dflt_fcns_size;
  }
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

// register all functions & macros declared globally along w/ their arg # & overloaded
// status (fills "fmacs" which then gets filtered into "overload_fmacs" on return to main)
void register_all_global_function_macro_defs(char *read) {
  char *r = read, *scout, function_name[MAX_TOKEN_NAME_LENGTH], bad_code_buffer[BAD_CODE_BUFFER_LENGTH];
  bool in_a_string = false, in_a_char = false, overload, prototype, macro;
  int in_global_scope = 0; // 0 if in global scope, > 0 if in a fcn or any other braces
  int arg_total, duplicate_overload;

  // register all function/macro names in file to detect overloads prior to prefixing invocations
  while(*r != '\0') {
    handle_string_char_brace_scopes(&in_a_string, &in_a_char, &in_global_scope, r);
    if(!in_a_string && !in_a_char)
      r = cola_skip_comments(r, NULL);
    if(!in_a_string && !in_a_char && in_global_scope == 0 && *r == '(') { // potential function/macro
      // check if at a macro, & if so confirm its "functionlike" (can't overload non-functionlike macros)
      macro = is_at_macro_name(r);
      if(macro && !macro_is_functionlike(r)) { 
        r = skip_macro_body(r); 
        continue; 
      } 
      // copy function name
      FLOOD_ZEROS(function_name, MAX_TOKEN_NAME_LENGTH);
      get_fmac_name(r, function_name);
      // get number of args for fcn/macro
      arg_total = count_args(r, function_name, macro, -1);
      // check whether at a function prototype
      prototype = is_function_prototype(r);
      // disregard if function name already exists w/ exact same arg # (ie already registered its prototype)
      if((duplicate_overload = non_prototype_duplicate_instance_in_fmacs(function_name, arg_total)) != -1) { 
        if(!fmacs[duplicate_overload].is_a_prototype) { // throw fatal error if not a prototype
          FLOOD_ZEROS(bad_code_buffer, BAD_CODE_BUFFER_LENGTH);
          get_invalid_code_snippet(read, r, bad_code_buffer);
          throw_fatal_error_duplicate_overload(function_name, arg_total, bad_code_buffer, __LINE__);
        }
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
      fmacs[fmacs_size].is_a_macro = macro;
      ++fmacs_size;
      // check if at a macro to skip over its body
      if(macro) r = skip_macro_body(r);
    }
    if(*r != '\0') ++r;
  }
}

/******************************************************************************
* FILTER OVERLOADED FCNS/MACROS FROM "FMACS" INTO "OVERLOAD_FMACS" STRUCTURE
******************************************************************************/

// convert "fmacs" struct to "overload_fmacs" struct (disregarding non-overloaded fcns)
void filter_overloads_from_FMACS_to_OVERLOAD_FMACS() {
  int i, overload_idx;
  for(i = 0; i < fmacs_size; ++i) {
    if(fmacs[i].overloaded && name_not_in_overload_fmacs(fmacs[i].name)) {
      // new function/macro name overload instance
      strcpy(overload_fmacs[overload_fmacs_size].name, fmacs[i].name);
      overload_fmacs[overload_fmacs_size].arg_sizes[0] = fmacs[i].args;
      overload_fmacs[overload_fmacs_size].is_a_macro[0] = fmacs[i].is_a_macro;
      overload_fmacs[overload_fmacs_size].arg_sizes_length = 1;
      ++ overload_fmacs_size;
    } else if(fmacs[i].overloaded && (overload_idx = overload_fmacs_instance_idx(i)) != -1) {
      // existing function/macro name overload has a new arg-length overload instance
      overload_fmacs[overload_idx].arg_sizes[overload_fmacs[overload_idx].arg_sizes_length] = fmacs[i].args;
      overload_fmacs[overload_idx].is_a_macro[overload_fmacs[overload_idx].arg_sizes_length] = fmacs[i].is_a_macro;
      ++ overload_fmacs[overload_idx].arg_sizes_length;
    }
  }
}

/******************************************************************************
* PREFIX FUNCTION/MACRO OVERLOAD INSTANCES & SPLICE IN DEFAULT FUNCTION VALS
******************************************************************************/

// prefix all invocation/declaration/definition instances of file's registered
// overloaded fcn/macro names & arg lengths (stored in "overload_fmacs" struct)
// & splice in default values
void prefix_overloaded_and_splice_default_value_instances(char *read, char *write) {
  char *r = read, *w = write, *scout, *next_arg, *arg_end;
  char bad_code_buffer[BAD_CODE_BUFFER_LENGTH];
  bool in_a_string = false, in_a_char = false;
  int i, j, k, arg_total, dflt_args_left, in_arg_scope;
  int ignore_arg = 0; // "global scope" irrelevant here thus now denoted as "ignore_arg"

  // prefix every overloaded instance w/ "COLA__<No_of_Args>_"
  while(*r != '\0') {
    // account for scopes
    handle_string_char_brace_scopes(&in_a_string, &in_a_char, &ignore_arg, r);
    if(!in_a_string && !in_a_char) 
      r = cola_skip_comments(r, w), w += strlen(w);
    // -:- PARSE FOR DEFAULTS -:- 
    // check for potential fcn invocation that has default args values
    if(!in_a_string && !in_a_char && VARCHAR(*r) && !VARCHAR(*(r-1))) {
      for(i = 0; i < unique_dflt_fcns_size; ++i) {
        // potential dflt'd fcn arg val instance
        if(is_at_substring(r, unique_dflt_fcns[i].name) && !VARCHAR(*(r+strlen(unique_dflt_fcns[i].name)))) { 
          // get number of args
          scout = r;
          while(VARCHAR(*scout))       ++scout; // skip past name
          while(IS_WHITESPACE(*scout)) ++scout; // skip past optional whitespace btwn name && '('
          
          // dflt'd name invocation w/o any args passed btwn "()" afterwards -- COLA names
          // can NEVER to redefined/reassigned to ANY other variables in ANY other scope
          if(*scout != '(') {
            FLOOD_ZEROS(bad_code_buffer, BAD_CODE_BUFFER_LENGTH);
            get_invalid_code_snippet(read, scout, bad_code_buffer);
            throw_fatal_error_non_functional_invocation_of_overload_or_dflt_name(unique_dflt_fcns[i].name, bad_code_buffer, __LINE__);
          }
          arg_total = count_args(scout, unique_dflt_fcns[i].name, false, i);

          // confirm at a dflt'd instance w/ less args than "dflt-total + nondflt-total" & as least the same args as non-dflt total
          // if same args as nondflt + dflt, no need to splice in anything
          for(j = 0; j < unique_dflt_fcns[i].arg_sizes_length; ++j)
            if(arg_total < unique_dflt_fcns[i].arg_sizes[j] && arg_total >= unique_dflt_fcns[i].arg_sizes[j] - unique_dflt_fcns[i].total_dflts[j])
              break;
          if(j == unique_dflt_fcns[i].arg_sizes_length) { // not at a defaulted arg that needs splicing in -- continue onto overload splicing
            i = unique_dflt_fcns_size;
            break;
          }

          // prefix dflt'd fcn invocation if its also overloaded
          for(k = 0; k < overload_fmacs_size; ++k)
            if(strcmp(overload_fmacs[k].name, unique_dflt_fcns[i].name) == 0) { // dflt'd fcn is also overloaded
              // prefix overloaded fcn/macro instance's name w/ reserved header & arg number
              sprintf(w, "COLA__%d_%s", unique_dflt_fcns[i].arg_sizes[j], unique_dflt_fcns[i].name);
              w += strlen(w), r = scout;
              break;
            }
          // copy name if it wasn't prefixed as an overload
          if(k == overload_fmacs_size) while(*r != '\0' && r != scout) *w++ = *r++;

          // copy the args & splice in the appropriate dflt values at the end of the args list
          in_arg_scope = 1;
          arg_end = scout++; // move 1 past the opening '('
          while(*scout != '\0' && in_arg_scope > 0) { // ends w/ 'scout' pointing at ')'
            handle_string_char_brace_scopes(&in_a_string, &in_a_char, &ignore_arg, scout);
            if(!in_a_string && !in_a_char && *scout == '(') ++in_arg_scope;
            else if(!in_a_string && !in_a_char && *scout == ')') --in_arg_scope;
            else if(!in_a_string && !in_a_char && *scout == ',') arg_end = scout; // end of an argument
            if(*scout == '\0' || in_arg_scope <= 0) break;
            ++scout;
          }
          // splice in default args
          while(r != scout) *w++ = *r++; // copy args up to right before closing ')'
          dflt_args_left = unique_dflt_fcns[i].total_dflts[j] - unique_dflt_fcns[i].arg_sizes[j] + arg_total;
          for(k = dflt_args_left; k < unique_dflt_fcns[i].total_dflts[j]; ++k) {
            if(arg_total > 0 || k > dflt_args_left) *w++ = ',';
            strcpy(w, unique_dflt_fcns[i].dflt_vals[j][k]);
            w += strlen(w);
          }
          *w++ = *r++; // copy closing ')' of args list
          break;
        }
      }
      // if DID find a valid matching dflt fcns (already prefixed it if overloaded above, thus force the next loop iteration)
      if(i < unique_dflt_fcns_size && *r != '\0') {
        *w++ = *r++;
        continue;
      }
      

      // -:- PARSE FOR OVERLOADS -:- 
      // check for potential fcn/macro overload
      for(i = 0; i < overload_fmacs_size; ++i) {
        // potential overloaded instance
        if(is_at_substring(r, overload_fmacs[i].name) && !VARCHAR(*(r+strlen(overload_fmacs[i].name)))) {
          // get number of args
          scout = r;
          while(VARCHAR(*scout))       ++scout; // skip past name
          while(IS_WHITESPACE(*scout)) ++scout; // skip past optional whitespace btwn name && '('
          
          // overload name invocation w/o any args passed btwn "()" afterwards -- COLA names
          // can NEVER to redefined/reassigned to ANY other variables in ANY other scope
          if(*scout != '(') {
            FLOOD_ZEROS(bad_code_buffer, BAD_CODE_BUFFER_LENGTH);
            get_invalid_code_snippet(read, scout, bad_code_buffer);
            throw_fatal_error_non_functional_invocation_of_overload_or_dflt_name(overload_fmacs[i].name, bad_code_buffer, __LINE__);
          }
          arg_total = count_args(scout, overload_fmacs[i].name, false, i);
          
          // confirm an overload instance with "arg_total" args was detected earlier
          for(j = 0; j < overload_fmacs[i].arg_sizes_length; ++j)
            if(arg_total == overload_fmacs[i].arg_sizes[j])
              break;

          // if overloaded instance invocation has an undefined # of args wrt "overload_fmacs",
          // ie no matching global definition/declaration/prototype found to couple w/ invocation
          if(j == overload_fmacs[i].arg_sizes_length) {
            FLOOD_ZEROS(bad_code_buffer, BAD_CODE_BUFFER_LENGTH);
            get_invalid_code_snippet(read, scout, bad_code_buffer);
            throw_fatal_error_undefined_arg_length_invocation(overload_fmacs[i].name, arg_total, bad_code_buffer, __LINE__);
          }

          // prefix overloaded fcn/macro instance's name w/ reserved header & arg number
          sprintf(w, "COLA__%d_%s", arg_total, overload_fmacs[i].name);
          w += strlen(w), r = scout;
          break;
        }
      }
      if(i == overload_fmacs_size && *r != '\0') *w++ = *r++;
    } else
      *w++ = *r++;
  }
  *w = '\0';
}

/******************************************************************************
* DEFAULT VALUE "unique_dflt_fcns" (HAS DFLT FCN INSTANCES) STRUCT HELPER FCNS
******************************************************************************/

// checks whether same fcn name already in "unique_dflt_fcns"
bool name_not_in_unique_dflt_fcns(char *name) {
  for(int i = 0; i < unique_dflt_fcns_size; ++i) 
    if(strcmp(unique_dflt_fcns[i].name, name) == 0) return false;
  return true;
}

// returns idx of fcn arg-defaulted fcn instance if instance doesn't already have 
// "all_dflt_fcns[all_dflt_fcns_idx].total_args" in unique_dflt_fcns's "arg_sizes[]", & returns 
// -1 if already found (ie if already stored its name & arg # instance from a prototype)
int unique_dflt_fcns_instance_idx(int all_dflt_fcns_idx) {
  for(int j = 0, k; j < unique_dflt_fcns_size; ++j) 
    if(strcmp(unique_dflt_fcns[j].name, all_dflt_fcns[all_dflt_fcns_idx].fcn_name) == 0) {
      // search instance's arg lengths for "all_dflt_fcns[all_dflt_fcns_idx].total_args"
      for(k = 0; k < unique_dflt_fcns[j].arg_sizes_length; ++k)
        if(all_dflt_fcns[all_dflt_fcns_idx].total_args == unique_dflt_fcns[j].arg_sizes[k]) break;
      // if arg count not found, return instance idx to 
      // add # of args as a new overload to "unique_dflt_fcns" instance's "arg_sizes[]"
      return (k == unique_dflt_fcns[j].arg_sizes_length) ? j : -1;
    }
  return -1; 
}

// returns the # of dflt'd args for the total # of args instance in "unique_dflt_fcns[unique_dflt_fcns_idx]"
int total_dflts_for_ARG_SIZE_fcn_instance_in_UNIQUE_DFLT_FCNS(int arg_size, int unique_dflt_fcns_idx) {
  for(int i = 0; i < unique_dflt_fcns[unique_dflt_fcns_idx].arg_sizes_length; ++i) // for each overloaded & dflt'd instance's arg length
    if(unique_dflt_fcns[unique_dflt_fcns_idx].arg_sizes[i] == arg_size) // if same number of args, found the fcn instance
      return unique_dflt_fcns[unique_dflt_fcns_idx].total_dflts[i]; // return how many dflt'd arg values the fcn instance has
  return 0;
}

/******************************************************************************
* DEFAULT VALUE FILTER FCN FROM "ALL_DFLT_FCNS" INTO "UNIQUE_DFLT_FCNS" STRUCT
******************************************************************************/

// convert "all_dflt_fcns" struct to "unique_dflt_fcns" struct (disregarding non-defaulted fcn instances)
void filter_defaulted_fcns_from_ALL_DFLT_FCNS_to_UNIQUE_DFLT_FCNS() {
  int i, j, dflts_idx, per_fcn_idx;
  for(i = 0; i < all_dflt_fcns_size; ++i) {
    if(all_dflt_fcns[i].total_dflts > 0 && name_not_in_unique_dflt_fcns(all_dflt_fcns[i].fcn_name)) {
      // new function name w/ dflt args instance
      strcpy(unique_dflt_fcns[unique_dflt_fcns_size].name, all_dflt_fcns[i].fcn_name);       // copy new fcn name
      unique_dflt_fcns[unique_dflt_fcns_size].arg_sizes[0] = all_dflt_fcns[i].total_args;    // copy # of total args
      unique_dflt_fcns[unique_dflt_fcns_size].arg_sizes_length = 1; // 1st instance of arg list size
      unique_dflt_fcns[unique_dflt_fcns_size].total_dflts[0] = all_dflt_fcns[i].total_dflts; // copy # of defaulted args
      for(j = 0; j < all_dflt_fcns[i].total_dflts; ++j) { // copy default arg values & idxs
        strcpy(unique_dflt_fcns[unique_dflt_fcns_size].dflt_vals[0][j], all_dflt_fcns[i].dflt_vals[j]);
        unique_dflt_fcns[unique_dflt_fcns_size].dflt_idxs[0][j] = all_dflt_fcns[i].dflt_idxs[j];
      }
      ++ unique_dflt_fcns_size;
    } else if(all_dflt_fcns[i].total_dflts && (dflts_idx = unique_dflt_fcns_instance_idx(i)) != -1) {
      // existing function name w/ dflt args has a new arg-length overload instance
      per_fcn_idx = unique_dflt_fcns[dflts_idx].arg_sizes_length;
      unique_dflt_fcns[dflts_idx].arg_sizes[per_fcn_idx] = all_dflt_fcns[i].total_args;    // copy # of total args
      unique_dflt_fcns[dflts_idx].total_dflts[per_fcn_idx] = all_dflt_fcns[i].total_dflts; // copy # of defaulted args
      for(j = 0; j < all_dflt_fcns[i].total_dflts; ++j) { // copy default arg values & idxs
        strcpy(unique_dflt_fcns[dflts_idx].dflt_vals[per_fcn_idx][j], all_dflt_fcns[i].dflt_vals[j]);
        unique_dflt_fcns[dflts_idx].dflt_idxs[per_fcn_idx][j] = all_dflt_fcns[i].dflt_idxs[j];
      }
      ++ unique_dflt_fcns[dflts_idx].arg_sizes_length;
    }
  }
}

/******************************************************************************
* DEFAULT VALUE FCNS CONFIRMING DFLT != PRIOR NON-DFLT, IN MACRO, OR AMBIGUOUS 
******************************************************************************/

// overloaded & dflt'd fcns must not be ambiguous, IE overload definitions must satisfy 1 of the following: 
//   (1) an overload's # of non-default args must exceed the # of cumulative
//       args (both dflt & not) of all other overloaded instances
//   (2) an overload's # of cumulative args (both dflt & not) must be less
//       than the # of non-default args of all other overloaded instances
bool ambiguous_overload(int overload_fmacs_dflts, int overload_fmacs_arg_size, int unique_dflt_fcns_arg_size, int unique_dflt_fcns_dflts) {
  int overload_fmacs_nondflts   = overload_fmacs_arg_size - overload_fmacs_dflts;
  int unique_dflt_fcns_nondflts = unique_dflt_fcns_arg_size - unique_dflt_fcns_dflts;
  bool more_nondflt_args_than_total_args = (overload_fmacs_nondflts > unique_dflt_fcns_arg_size);
  bool less_total_args_than_nondflt_args = (overload_fmacs_arg_size < unique_dflt_fcns_nondflts);
  return !(more_nondflt_args_than_total_args || less_total_args_than_nondflt_args);
}

// confirm no ambigious overloaded definitions wrt assigned default values causing indeterminate fcn invocations
void confirm_no_ambiguous_overloaded_default_values() {
  int overload_fmacs_idx = 0, unique_dflt_fcns_idx, i, j;
  int overload_fmacs_total_dflts;   // # of dflt args for an overload instance
  int unique_dflt_fcns_total_dflts; // # of dflt args for a defaulted instance
  int overload_fmacs_arg_size;      // # of total args for an overload instance
  int unique_dflt_fcns_arg_size;    // # of total args for a defaulted instance
  for(; overload_fmacs_idx < overload_fmacs_size; ++overload_fmacs_idx) // for each overload
    for(unique_dflt_fcns_idx = 0; unique_dflt_fcns_idx < unique_dflt_fcns_size; ++unique_dflt_fcns_idx) { // for each defaulted fcn
      #define current_fmac overload_fmacs[overload_fmacs_idx]
      #define current_dflt unique_dflt_fcns[unique_dflt_fcns_idx]
      // if at least 1 overload instance also has a dflt arg value
      if(strcmp(current_fmac.name, current_dflt.name) == 0)  
        /* at this point exact same arg-length fcn duplicates would have triggered a "duplicate args length" 
         * fatal error, thus all arg lengths -- defaulted or not -- are unique */
        for(i = 0; i < current_fmac.arg_sizes_length; ++i) // for each overload instance (via the differnt unique arg lengths registered)
          for(j = 0; j < current_dflt.arg_sizes_length; ++j) { // for each dflt instance
            // same fcn name & arg length == same fcn instance
            if(current_fmac.arg_sizes[i] == current_dflt.arg_sizes[j]) continue; // don't compare a fcn to itself
            overload_fmacs_total_dflts = total_dflts_for_ARG_SIZE_fcn_instance_in_UNIQUE_DFLT_FCNS(current_fmac.arg_sizes[i], unique_dflt_fcns_idx);
            overload_fmacs_arg_size    = current_fmac.arg_sizes[i];
            unique_dflt_fcns_arg_size     = current_dflt.arg_sizes[j];
            unique_dflt_fcns_total_dflts  = current_dflt.total_dflts[j];
            
            if(ambiguous_overload(overload_fmacs_total_dflts, overload_fmacs_arg_size, unique_dflt_fcns_arg_size, unique_dflt_fcns_total_dflts))
              throw_fatal_error_ambiguous_overload(current_fmac.name, overload_fmacs_total_dflts, 
                overload_fmacs_arg_size, unique_dflt_fcns_arg_size, unique_dflt_fcns_total_dflts, __LINE__);
          }
    }
  #undef current_fmac
  #undef current_dflt
}

// confirms any/all dflt values were defined at the end of the fcn's arg list & not in a macro
void confirm_valid_default_values_position(int arg_total, char *function_name, bool is_a_macro) {
  #define current_dflt all_dflt_fcns[all_dflt_fcns_size] 
  if(current_dflt.total_dflts == 0) return; // no defaulted values found
  const int earliest_dflt_idx = arg_total - current_dflt.total_dflts; 
  for(int i = 0; i < current_dflt.total_dflts; ++i)
    if(current_dflt.dflt_idxs[i] < earliest_dflt_idx || is_a_macro)
      throw_fatal_error_invalid_default_arg_value(function_name, is_a_macro, __LINE__);
  #undef current_dflt
}

/******************************************************************************
* DEFAULT VALUE STORING IN "all_dflt_fcns" & WHITING-OUT FROM ARG LIST FUNCTION
******************************************************************************/

// given a ptr in an function's arg list, reads & stores default value in "all_dflt_fcns" struct,
// returns length of the default value + 1 to whitespace the "= <dfltVal>" in the args list
int store_dflt_value(int current_arg_idx, char *p) {
  int idx = 0, dflt_val_length = 1; // "1" to account for the '='
  char *scout = p + 1;
  // more readble to repn. current "fcn" dflt-tracking instance in "all_dflt_fcns"
  #define current_dflt all_dflt_fcns[all_dflt_fcns_size] 
  FLOOD_ZEROS(current_dflt.dflt_vals[current_arg_idx], MAX_ARGS_PER_FCN);
  // store defaulted arg's idx relative to ithe fcn's entire args list
  current_dflt.dflt_idxs[current_dflt.total_dflts] = current_arg_idx; 
  // skip optional whitespace btwn '=' && dflt value
  while(IS_WHITESPACE(*scout)) ++scout, ++dflt_val_length; 
  // copy default value
  while(*scout != '\0' && *scout != ',' && *scout != ')')
    current_dflt.dflt_vals[current_dflt.total_dflts][idx++] = *scout++, ++dflt_val_length; 
  current_dflt.dflt_vals[current_dflt.total_dflts][idx] = '\0';
  if(idx > 0) { // trim possible whitespaces following the default value
    scout = &current_dflt.dflt_vals[current_dflt.total_dflts][idx-1];
    while(IS_WHITESPACE(*scout)) *scout-- = '\0'; 
  }
  ++ current_dflt.total_dflts;
  #undef current_dflt
  return dflt_val_length;
}
