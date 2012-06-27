#include "linker_set.h"

static int a = 10;
static int b = 15;
static int c = 2;
LINKER_SET_ADD_DATA(tset, a);
LINKER_SET_ADD_DATA(tset, b);
LINKER_SET_ADD_DATA(tset, c);
