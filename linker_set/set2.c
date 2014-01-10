#include "linker_set.h"

static int a = 8;
static int b = 13;
static int c = 2;
LINKER_SET_ADD_DATA(tset, a);
LINKER_SET_ADD_DATA(tset, b);
LINKER_SET_ADD_DATA(tset, c);
