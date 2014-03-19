/* File : example.i */
%module example
%{
#include "interface.h"
%}

/* Let's just grab the original header file here */
%include "interface.h"

