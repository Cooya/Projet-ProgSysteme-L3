#include "extlib.def.h"
#include "extlib.dll.h"
#include "extlib.dll.more.h"

struct _Dll {
    ElCmpFct cmpFct;
    DllNode first;
    DllNode last;
    int elemSize;
};

struct _DllNode {
    Ptr data;
    DllNode next;
    DllNode prev;
};



Dll dllNew(int elemSize) {
    Dll list = malloc(sizeof(struct _Dll));

    if(elemSize<=0) {
        list->elemSize=_elSizeFct(elemSize);
        list->cmpFct=_elCompareFct(elemSize);
    }
    else {
        list->elemSize=elemSize;
        list->cmpFct=NULL;
    }

    list->first=NULL;
    list->last=NULL;
    return list;
}

void dllDel(Dll list) {
    dllClear(list);
    free(list);
}



bool dllIsEmpty(Dll list) {
    return !list->first;
}

Dll dllClear(Dll list) {
    DllNode node = list->first;
    DllNode noden;

    while(node) {
        noden = node;
        node=node->next;
        free(noden->data);
        free(noden);
    }
    list->first=NULL;
    list->last=NULL;
    return list;
}

int dllCount(Dll list) {
    int size=0;
    DllNode node = list->first;
    while(node) {
        size++;
        node = node->next;
    }
    return size;
}

Ptr dllGetData(DllNode node) {
    return node->data;
}



DllNode dllGetFront(Dll list) {
    return list->first;
}

DllNode dllGetNext(DllNode node) {
    return node->next;
}

DllNode dllGetPrev(DllNode node) {
    return node->prev;
}

DllNode dllGetBack(Dll list) {
    return list->last;
}

DllNode dllGetAt(Dll list, int pos) {
    DllNode node = list->first;

    if(!node)
        return NULL;

    for(int i=0; i<pos; i++) {
        node=node->next;
        if(!node)
            return NULL;
    }

    return node;
}



DllNode dllSetFront(Dll list, Ptr data) {
    DllNode node = list->first;

    memcpy(node->data, data, list->elemSize);

    return node;
}

DllNode dllSetNode(Dll list, DllNode node, Ptr data) {
    memcpy(node->data, data, list->elemSize);

    return node;
}

DllNode dllSetBack(Dll list, Ptr data) {
    DllNode node = list->last;

    memcpy(node->data, data, list->elemSize);

    return node;
}

DllNode dllSetAt(Dll list, int pos, Ptr data) {
    DllNode node = dllGetAt(list, pos);

    dllSetNode(list, node, data);

    return node;
}



DllNode dllPushFront(Dll list, Ptr data) {
    DllNode newnode = malloc(sizeof(struct _DllNode));

    newnode->next=list->first;
    list->first=newnode;

    newnode->prev=NULL;
    if(newnode->next)
        newnode->next->prev=newnode;
    else
        list->last=newnode;

    newnode->data=malloc(list->elemSize);
    memcpy(newnode->data, data, list->elemSize);

    //newnode->data=data;

    return newnode;
}

DllNode dllPushAfter(Dll list, DllNode node, Ptr data) {
    DllNode newnode = malloc(sizeof(struct _DllNode));

    newnode->next=node->next;
    node->next=newnode;

    newnode->prev=node;
    if(newnode->next)
        newnode->next->prev=newnode;
    else
        list->last=newnode;

    newnode->data=malloc(list->elemSize);
    memcpy(newnode->data, data, list->elemSize);

    //newnode->data=data;

    return newnode;
}

DllNode dllPushBefore(Dll list, DllNode node, Ptr data) {
    DllNode newnode = malloc(sizeof(struct _DllNode));

    newnode->prev=node->prev;
    node->prev=newnode;

    newnode->next=node;
    if(newnode->prev)
        newnode->prev->next=newnode;
    else
        list->first=newnode;

    newnode->data=malloc(list->elemSize);
    memcpy(newnode->data, data, list->elemSize);

    //newnode->data=data;

    return newnode;
}

DllNode dllPushBack(Dll list, Ptr data) {
    DllNode newnode = malloc(sizeof(struct _DllNode));

    newnode->prev=list->last;
    list->last=newnode;

    newnode->next=NULL;
    if(newnode->prev)
        newnode->prev->next=newnode;
    else
        list->first=newnode;

    newnode->data=malloc(list->elemSize);
    memcpy(newnode->data, data, list->elemSize);

    //newnode->data=data;

    return newnode;
}

DllNode dllPushAt(Dll list, int pos, Ptr data) {
    if(pos==0)
        return dllPushFront(list, data);

    //DllNode node = dllGetAt(list, pos-1);

    /*if(!node)
        throwExc("ExtLib.Dll.dllAddAt", "Given position is out of bounds.");*/

    return dllPushAfter(list, dllGetAt(list, pos-1), data);
}



Dll dllPopFront(Dll list) {
    DllNode oldnode = list->first;

    list->first=oldnode->next;

    if(oldnode->next)
        oldnode->next->prev=NULL;
    else
        list->last=NULL;

    free(oldnode->data);
    free(oldnode);

    return list;
}

Dll dllPopAfter(Dll list, DllNode node) {
    DllNode oldnode = node->next;

    if(oldnode->next) {
        oldnode->next->prev=node;
        node->next=oldnode->next;
    }
    else {
        list->last=node;
        node->next=NULL;
    }

    free(oldnode->data);
    free(oldnode);

    return list;
}

Dll dllPopBefore(Dll list, DllNode node) {
    DllNode oldnode = node->prev;

    if(oldnode->prev) {
        oldnode->prev->next=node;
        node->prev=oldnode->prev;
    }
    else {
        list->first=node;
        node->prev=NULL;
    }

    free(oldnode->data);
    free(oldnode);

    return list;
}

Dll dllPopNode(Dll list, DllNode node) {
    if(node->next)
        node->next->prev=node->prev;
    else
        list->last=node->prev;

    if(node->prev)
        node->prev->next=node->next;
    else
        list->first=node->next;

    free(node->data);
    free(node);

    return list;
}

Dll dllPopBack(Dll list) {
    DllNode oldnode = list->last;

    list->last=oldnode->prev;

    if(oldnode->prev)
        oldnode->prev->next=NULL;
    else
        list->first=NULL;

    free(oldnode->data);
    free(oldnode);

    return list;
}

Dll dllPopAt(Dll list, int pos) {
    if(pos==0)
        return dllPopFront(list);

    dllPopAfter(list, dllGetAt(list, pos-1));

    return list;
}



Dll dllComparable(Dll list, ElCmpFct fct) {
    list->cmpFct=fct;
    return list;
}

Dll dllSort(Dll list, int method) {
    DllNode first=list->first;
    ElCmpFct fct=list->cmpFct;
    int listSize=1, numMerges, leftSize, rightSize;
    DllNode tail, left, right, next;
    if (!first || !first->next) return list;  // Trivial case

    do { // For each power of two<=list length
        numMerges=0,left=first;
        tail=first=0; // Start at the start

        while (left) { // Do this list_len/listSize times:
            numMerges++,right=left,leftSize=0,rightSize=listSize;
            // Cut list into two halves (but don't overrun)
            while (right && leftSize<listSize) leftSize++,right=right->next;
            // Run through the lists appending onto what we have so far.
            while (leftSize>0 || (rightSize>0 && right)) {
                // Left empty, take right OR Right empty, take left, OR compare.
                if (!leftSize)                  {next=right;right=right->next;rightSize--;}
                else if (!rightSize || !right)  {next=left;left=left->next;leftSize--;}
                else if (method*fct(left->data,right->data)<0)     {next=left;left=left->next;leftSize--;}
                else                            {next=right;right=right->next;rightSize--;}
                // Update pointers to keep track of where we are:
                if (tail) tail->next=next;  else first=next;
                // Sort prev pointer
                next->prev=tail; // Optional.
                tail=next;
            }
            // Right is now AFTER the list we just sorted, so start the next sort there.
            left=right;
        }
        // Terminate the list, double the list-sort size.
        tail->next=0,listSize<<=1;
    } while (numMerges>1); // If we only did one merge, then we just sorted the whole list.
    list->first=first;
    list->last=tail;
    return list;
}



void dllHeap(Dll list) {
    int elts=dllCount(list);
    int effcost=elts*list->elemSize;
    int opcost=sizeof(struct _Dll)+elts*sizeof(struct _DllNode);

    printf("Doubly-linked list at %p\n", list);
    printf("\t%d elements, each using %d bytes\n", elts, list->elemSize);
    printf("\t%d bytes used for elemets\n", effcost);
    printf("\t%d bytes used as operating cost\n", opcost);
    printf("\t%d bytes total used\n", opcost+effcost);
}
