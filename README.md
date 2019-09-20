# COLA
## _C Overloaded Length Arguments for Functions &amp; Macros!_
## _Default Argument Values For Functions!_

----------------------
## Using `cola.c`:

### Overview:
* ***Enables Function/Macro Overloading Based on # of Arguments!***
  * _"Overloading" refers to having 2+ function/macros share the same name for programming convenience_
* ***Enables Function Default Argument Values!***
  * _Default argument values must be listed **last** in a function's argument list!_
* ***Suppose `filename.c`. If Overloads Or Defaults Found, Writes a Modified Copy in Valid C to `filename_COLA.c`***
* ***See [cola_SampleExec.c](https://github.com/jrandleman/COLA/blob/master/cola_SampleExec.c) for a Demo Implementation!*** 
  * [cola_SampleExec_COLA.c](https://github.com/jrandleman/COLA/blob/master/cola_SampleExec_COLA.c) _shows the result of running_ `cola_SampleExec.c` _through the_ `cola.c` _parser_

### Compile [cola.c](https://github.com/jrandleman/COLA/blob/master/cola.c) (_add `-l` for [parsing info](#using-the--l-info-flag)!_):
```c
$ gcc -std=c99 -o cola cola.c
$ ./cola yourFile.c // $ ./cola -l yourFile.c 
```
### COLA's 10 Caveats, Straight From `cola.c`:
```c
/*****************************************************************************
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
 *           versa) fcn will be treated as if both had the default vals     *
 *       (*) if a fcn proto has DIFFERENT default vals from its defn, the   *
 *           fcn's proto default vals are treated as the only default vals  *
 ****************************************************************************/
```
### "ODV" Guildline to Combine Overloaded Functions w/ Default Values:
* ***Straight From [`cola.c`](https://github.com/jrandleman/COLA/blob/master/cola.c)!***</br>
* ***See More on Both Overloading & Default Argument Values in [`cola_SampleExec.c`](https://github.com/jrandleman/COLA/blob/master/cola_SampleExec.c)!***</br>
```c
/*****************************************************************************
 *  -:- ODV GUIDELINE TO COMBINE (O)VERLOADS W/ (D)EFAULT ARG (V)ALUES -:-  *
 * >> overload definitions must satisfy 1 of the following:                 *
 *    (1) an overload's # of non-dflt args must exceed the # of cumulative  *
 *        args (both dflt & not) of all other overloaded instances          *
 *    (2) an overload's # of cumulative args (both dflt & not) must be less *
 *        than the # of non-dflt args of all other overloaded instances     *
*****************************************************************************/
```

### Using the `-l` Info Flag:
* ***Helpful to Debug Programs/Get General Info of the `cola.c` Parser***
#### `-l` Ouput After Running [`cola_SampleExec.c`](https://github.com/jrandleman/COLA/blob/master/cola_SampleExec.c) Through [`cola.c`](https://github.com/jrandleman/COLA/blob/master/cola.c):
```
>> OVERLOADS:
   01) NAME "show"              OVERLOAD ARG LENGTHS 2, 3, 1
   02) NAME "assign"            OVERLOAD ARG LENGTHS 3, 2
   03) NAME "overload_defaults" OVERLOAD ARG LENGTHS 2, 1

>> DEFAULTS:
   01) NAME "demo_default_vals" DEFAULT FCN ARG VALUES (Fcn1) "default value used!", 9.999
   02) NAME "overload_defaults" DEFAULT FCN ARG VALUES (Fcn1) 10

>> ALL REGISTERED GLOBAL FCNS/MACROS:
   01) NAME "show",              IS PROTOTYPE 0, TOTAL ARGS 2
   02) NAME "show",              IS PROTOTYPE 0, TOTAL ARGS 3
   03) NAME "assign",            IS PROTOTYPE 0, TOTAL ARGS 3
   04) NAME "assign",            IS PROTOTYPE 0, TOTAL ARGS 2
   05) NAME "show",              IS PROTOTYPE 0, TOTAL ARGS 1
   06) NAME "demo_default_vals", IS PROTOTYPE 0, TOTAL ARGS 3
   07) NAME "overload_defaults", IS PROTOTYPE 0, TOTAL ARGS 2
   08) NAME "overload_defaults", IS PROTOTYPE 0, TOTAL ARGS 1
   09) NAME "main",              IS PROTOTYPE 0, TOTAL ARGS 0
```
