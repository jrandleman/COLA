# COLA
## _C Overloaded Length Arguments for Functions &amp; Macros!_

----------------------
## Using `cola.c`:

### Overview:
* ***Enables Function/Macro Overloading Based on # of Arguments!***
  * "_Overloading" refers to having 2+ function/macros share the same name for programming convenience_
* ***Suppose `filename.c`. If Overloads Found, Writes a Modified Copy in Valid C to `filename_COLA.c`***
* ***See [cola_SampleExec.c](https://github.com/jrandleman/COLA/blob/master/cola_SampleExec.c) for a Demo Implementation!*** 
  * [cola_SampleExec_COLA.c](https://github.com/jrandleman/COLA/blob/master/cola_SampleExec_COLA.c) _shows the result of running_ `cola_SampleExec.c` _through the_ `cola.c` _parser_

### Compile [cola.c](https://github.com/jrandleman/COLA/blob/master/cola.c) (_add `-l` for [parsing info](#using-the--l-info-flag)!_):
```c
$ gcc -o cola cola.c
$ ./cola yourFile.c // $ ./cola -l yourFile.c 
```

### COLA's 7 Caveats, Straight From `cola.c`:
```c
/*****************************************************************************
 *                         -:- COLA.C 7 CAVEATS -:-                         *
 *   (0) "COLA_" PREFIX IS RESERVED                                         *
 *   (1) NO OVERLOADED VARIADIC FCNS/MACROS                                 *
 *   (2) NO OVERLOADS W/IN CONDITIONAL PREPROCESSOR DIRECTIVES              *
 *       (*) IE NOT W/IN: #if, #ifdef, #ifndef, #elif, #else, & #endif      *
 *   (3) NO FCN PTRS POINTING TO OVERLOADED FCNS                            *
 *       (*) can't determine overloaded arg # from only overloaded fcn name *
 *   (4) NO REDEFINING OVERLOADED NAME AS A DIFFERENT VAR NAME IN ANY SCOPE *
 *   (5) NO OVERLOADED MACROS CAN EVER BE "#undef"'d                        *
 *   (6) ONLY GLOBALLY DEFINED FCNS/MACROS CAN BE OVERLOADED                *
 *****************************************************************************/
```

### Using the `-l` Info Flag:
* ***Helpful to Debug Programs/Get General Info of the `cola.c` Parser***
#### `-l` Ouput After Running [`cola_SampleExec.c`](https://github.com/jrandleman/COLA/blob/master/cola_SampleExec.c) Through [`cola.c`](https://github.com/jrandleman/COLA/blob/master/cola.c):
```
>> OVERLOADS:
	01) NAME "show" OVERLOADS 2 3 1
	02) NAME "assign" OVERLOADS 3 2

>> ALL REGISTERED GLOBAL FCNS/MACROS:
	01) NAME "show", ARGS LENGTH 2, IS A PROTOTYPE 0
	02) NAME "show", ARGS LENGTH 3, IS A PROTOTYPE 0
	03) NAME "assign", ARGS LENGTH 3, IS A PROTOTYPE 0
	04) NAME "assign", ARGS LENGTH 2, IS A PROTOTYPE 0
	05) NAME "show", ARGS LENGTH 1, IS A PROTOTYPE 0
	06) NAME "main", ARGS LENGTH 0, IS A PROTOTYPE 0
```
