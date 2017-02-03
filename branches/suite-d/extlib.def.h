/**
 * \file extlib.def.h
 * \brief Functions and types needed by ExtLib
 * \author Jason Pindat
 * \version 1.0
 * \date 05/21/2014
 *
 */

#ifndef EXTLIB_DEF_H
#define EXTLIB_DEF_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define EL_ASC        1
#define EL_DESC       -1

#define EL_POINTER    0
#define EL_CHAR       -1
#define EL_UCHAR      -2
#define EL_SHORT      -3
#define EL_USHORT     -4
#define EL_INT        -5
#define EL_UINT       -6
#define EL_LONG       -7
#define EL_ULONG      -8
#define EL_LONGLONG   -9
#define EL_ULONGLONG  -10
#define EL_FLOAT      -11
#define EL_DOUBLE     -12
#define EL_LONGDOUBLE -13
#define EL_BOOL       -14

/** Ptr : type for a generic pointer. */
typedef void *Ptr;

typedef int(*ElCmpFct)(Ptr, Ptr);


/** \brief Throws an error.
 *
 * \param module : Name of the module and function that throws the error.
 * \param msg : Error message.
 * \return void
 *
 */
void throwExc(char *module, char *msg);

ElCmpFct _elCompareFct(int type);
int _elSizeFct(int type);

#endif
