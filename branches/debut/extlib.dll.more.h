/**
 * \file extlib.dll.more.h
 * \brief Advanced functions for doubly-linked lists
 * \author Jason Pindat
 * \version 1.0
 * \date 04/21/2014
 *
 * Some advanced functions to manage doubly-linked lists which have a non-constant complexity. Including this file includes the basic functions too.
 *
 */

#ifndef EXTLIB_DLL_MORE_H
#define EXTLIB_DLL_MORE_H

#include "extlib.def.h"
#include "extlib.dll.h"



/** \brief Removes the whole content of a list (Not primitive).
 *
 * \param list : List to clear.
 * \return The list.
 *
 */
Dll dllClear(Dll list);



/** \brief Returns the size of the list (Not primitive).
 *
 * \param list : List to count elements.
 * \return Number of elements.
 *
 */
int dllCount(Dll list);



/** \brief Returns the node of a list at a given position (Not primitive).
 *
 * \param list : List to seek in.
 * \param pos : Position of the node, 0 is start.
 * \return Node at the position in the list, NIL if end of list or empty list.
 *
 */
DllNode dllGetAt(Dll list, int pos);



/** \brief Modifies the data of the node at a given position in a list (Not primitive).
 *
 * \param list : List in which is the node.
 * \param pos : Position of the node, 0 is start.
 * \param data : Pointer to the new data.
 * \return The modified node.
 *
 */
DllNode dllSetAt(Dll list, int pos, Ptr data);



/** \brief Adds a new node at a given position to a list (Not primitive).
 *
 * \param list : List to add in.
 * \param pos : Position of the new node, 0 is start.
 * \param data : Pointer to the data to add.
 * \return The added node.
 *
 */
DllNode dllPushAt(Dll list, int pos, Ptr data);



/** \brief Removes the node at a given position in a list (Not primitive).
 *
 * \param list : List to remove in.
 * \param pos : Position of the node, 0 is start.
 * \return The same list.
 *
 */
Dll dllPopAt(Dll list, int pos);



/** \brief Sets the function to compare 2 elements of this list, note that if you declared the list with EL_*, the comparison function of the specified type is automatically linked.
 *
 * \param list : Doubly-linked list in which you set the fonction.
 * \param fct : pointer to the function, the function must take 2 pointers to the data and return an int which is <0 if 1st value is lower tha 2nd, >0 for the opposite and =0 if 1st value equals 2nd value.
 * \return The same list.
 *
 */
Dll dllComparable(Dll list, ElCmpFct fct);

/** \brief Sorts the list with a merge sort, dllComparable must have been called or the list must have been created with a EL_* constant.
 *
 * \param list : List to sort.
 * \param method : must be EL_ASC or EL_DESC.
 * \return The same list.
 *
 */
Dll dllSort(Dll list, int method);

#endif
