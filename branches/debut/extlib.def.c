#include "extlib.def.h"

void throwExc(char *module, char *msg) {
    printf("/!\\ [%s] %s", module, msg);
    exit(1);
}



int _elComparePointer(Ptr v1, Ptr v2) {
    return (int)(*(Ptr*)v1-*(Ptr*)v2);
}
int _elCompareChar(Ptr v1, Ptr v2) {
    return (int)(*(char*)v1-*(char*)v2);
}
int _elCompareUchar(Ptr v1, Ptr v2) {
    return (int)(*(unsigned char*)v1-*(unsigned char*)v2);
}
int _elCompareShort(Ptr v1, Ptr v2) {
    return (int)(*(short*)v1-*(short*)v2);
}
int _elCompareUshort(Ptr v1, Ptr v2) {
    return (int)(*(unsigned short*)v1-*(unsigned short*)v2);
}
int _elCompareInt(Ptr v1, Ptr v2) {
    return (int)(*(int*)v1-*(int*)v2);
}
int _elCompareUint(Ptr v1, Ptr v2) {
    return (int)(*(unsigned int*)v1-*(unsigned int*)v2);
}
int _elCompareLong(Ptr v1, Ptr v2) {
    return (int)(*(long*)v1-*(long*)v2);
}
int _elCompareUlong(Ptr v1, Ptr v2) {
    return (int)(*(unsigned long*)v1-*(unsigned long*)v2);
}
int _elCompareLonglong(Ptr v1, Ptr v2) {
    return (int)(*(long long*)v1-*(long long*)v2);
}
int _elCompareUlonglong(Ptr v1, Ptr v2) {
    return (int)(*(unsigned long long*)v1-*(unsigned long long*)v2);
}
int _elCompareFloat(Ptr v1, Ptr v2) {
    if(*(float*)v1<*(float*)v2) return -1;
    if(*(float*)v1>*(float*)v2) return 1;
    return 0;
}
int _elCompareDouble(Ptr v1, Ptr v2) {
    if(*(double*)v1<*(double*)v2) return -1;
    if(*(double*)v1>*(double*)v2) return 1;
    return 0;
}
int _elCompareLongdouble(Ptr v1, Ptr v2) {
    if(*(long double*)v1<*(long double*)v2) return -1;
    if(*(long double*)v1>*(long double*)v2) return 1;
    return 0;
}
int _elCompareBool(Ptr v1, Ptr v2) {
    return (int)(*(bool*)v1-*(bool*)v2);
}



ElCmpFct _elCompareFct(int type) {
    switch (type) {
        case 0  : return _elComparePointer;
        case -1 : return _elCompareChar;
        case -2 : return _elCompareUchar;
        case -3 : return _elCompareShort;
        case -4 : return _elCompareUshort;
        case -5 : return _elCompareInt;
        case -6 : return _elCompareUint;
        case -7 : return _elCompareLong;
        case -8 : return _elCompareUlong;
        case -9 : return _elCompareLonglong;
        case -10: return _elCompareUlonglong;
        case -11: return _elCompareFloat;
        case -12: return _elCompareDouble;
        case -13: return _elCompareLongdouble;
        case -14: return _elCompareBool;
        default : return NULL;
    }
}

int _elSizeFct(int type) {
    switch (type) {
        case 0  : return sizeof(Ptr);
        case -1 : return sizeof(char);
        case -2 : return sizeof(unsigned char);
        case -3 : return sizeof(short);
        case -4 : return sizeof(unsigned short);
        case -5 : return sizeof(int);
        case -6 : return sizeof(unsigned int);
        case -7 : return sizeof(long);
        case -8 : return sizeof(unsigned long);
        case -9 : return sizeof(long long);
        case -10: return sizeof(unsigned long long);
        case -11: return sizeof(float);
        case -12: return sizeof(double);
        case -13: return sizeof(long double);
        case -14: return sizeof(bool);
        default : return type;
    }
}
