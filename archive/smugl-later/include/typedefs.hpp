#pragma once
/*
 *  Definitions for 'native types' so that things get typecast
 *  properly
 */

typedef const char  string;     /* Static string definition */
typedef signed long vocid_t;    /* Vocab ID */
typedef signed long msgno_t;    /* A 'umsg' number */
typedef signed long counter_t;  /* Any value used for counting */
typedef signed short arg_t;     /* Compiler lang-table argument */
typedef signed long basic_obj;  /* Basic object number */
typedef signed int container_t; /* Position within container array */
typedef unsigned long flag_t;   /* Flag value */

