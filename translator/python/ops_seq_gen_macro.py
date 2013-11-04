#!/usr/bin/env python
#######################################################################
#                                                                     #
#       This Python routine generates the header file ops_seq.h        #
#                                                                     #
#######################################################################


#
# this sets the max number of arguments in ops_par_loop
#
maxargs = 16

#open/create file
f = open('./ops_seq_macro.h','w')

#
#first the top bit
#

top =  """
/*
* Open source copyright declaration based on BSD open source template:
* http://www.opensource.org/licenses/bsd-license.php
*
* This file is part of the OPS distribution.
*
* Copyright (c) 2013, Mike Giles and others. Please see the AUTHORS file in
* the main source directory for a full list of copyright holders.
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
* * Redistributions of source code must retain the above copyright
* notice, this list of conditions and the following disclaimer.
* * Redistributions in binary form must reproduce the above copyright
* notice, this list of conditions and the following disclaimer in the
* documentation and/or other materials provided with the distribution.
* * The name of Mike Giles may not be used to endorse or promote products
* derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY Mike Giles ''AS IS'' AND ANY
* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL Mike Giles BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/** @brief header file declaring the functions for the ops sequential backend
  * @author Gihan Mudalige, Istvan Reguly
  * @details Declares the OPS API calls for the sequential backend
  */

#include "ops_lib_cpp.h"


"""

f.write(top)

#
# now define the macros and extern vars
#

f.write('#ifndef OPS_ACC_MACROS\n')
for nargs in range (0,maxargs):
  f.write('#define OPS_ACC'+str(nargs)+'(x,y) (x+xdim'+str(nargs)+'*(y))\n')
f.write('#endif\n\n')

for nargs in range (0,maxargs):
  f.write('extern int xdim'+str(nargs)+';\n')

#
# now for ops_par_loop defns
#

#
# now for ops_par_loop defns
#

for nargs in range (1,maxargs+1):
    f.write('\n\n//\n')
    f.write('//ops_par_loop routine for '+str(nargs)+' arguments\n')
    f.write('//\n')

    n_per_line = 4

    f.write('template <')
    for n in range (0, nargs):
        f.write('class T'+str(n))
        if nargs <> 1 and n != nargs-1:
          f.write(',')
        else:
          f.write('>\n')
        if n%n_per_line == 3 and n <> nargs-1:
          f.write('\n')

    f.write('void ops_par_loop_macro(void (*kernel)(')
    for n in range (0, nargs):
        f.write('T'+str(n)+'*')
        if nargs <> 1 and n != nargs-1:
          f.write(',')
        else:
          f.write('),\n')
        if n%n_per_line == 3 and n <> nargs-1:
          f.write('\n                           ')
        else:
          f.write(' ')


    f.write('    char const * name, int dim, int *range,\n    ')
    for n in range (0, nargs):
        f.write(' ops_arg arg'+str(n))
        if nargs <> 1 and n != nargs-1:
          f.write(',')
        else:
          f.write(') {\n')
        if n%n_per_line == 3 and n <> nargs-1:
         f.write('\n    ')

    f.write('\n  char *p_a['+str(nargs)+'];')
    f.write('\n  int  offs['+str(nargs)+'][2];\n')
    f.write('\n  int  count[dim];\n')

    f.write('  ops_arg args['+str(nargs)+'] = {')
    for n in range (0, nargs):
        f.write(' arg'+str(n))
        if nargs <> 1 and n != nargs-1:
          f.write(',')
        else:
          f.write('};\n\n')
        if n%n_per_line == 3 and n <> nargs-1:
          f.write('\n                    ')


    f.write('  for (int i = 0; i<'+str(nargs)+';i++) {\n')
    f.write('    if(args[i].stencil!=NULL) {\n')
    f.write('      offs[i][0] = 1;  //unit step in x dimension\n')
    f.write('      offs[i][1] = ops_offs_set(range[0],range[2]+1, args[i]) - ops_offs_set(range[1],range[2], args[i]) +1;\n')

    f.write('      if (args[i].stencil->stride[0] == 0) { //stride in y as x stride is 0\n')
    f.write('        offs[i][0] = 0;\n')
    f.write('        offs[i][1] = args[i].dat->block_size[0];\n')
    f.write('      }\n')
    f.write('      else if (args[i].stencil->stride[1] == 0) {//stride in x as y stride is 0\n')
    f.write('        offs[i][0] = 1;\n')
    f.write('        offs[i][1] = -( range[1] - range[0] ) +1;\n')
    f.write('      }\n')
    f.write('    }\n')
    f.write('  }\n\n')

    f.write('  //store index of non_gbl args\n')
    f.write('  int non_gbl['+str(nargs)+'] = {')
    for n in range (0, nargs):
        f.write('0')
        if nargs <> 1 and n != nargs-1:
          f.write(', ')
        else:
          f.write('};\n')
        if n%n_per_line == 5 and n <> nargs-1:
          f.write('\n                    ')
    f.write('  int g = 0;\n\n')

    f.write('  //set up initial pointers\n')
    f.write('  for (int i = 0; i < '+str(nargs)+'; i++) {\n')
    f.write('    if (args[i].argtype == OPS_ARG_DAT) {\n')
    f.write('      p_a[i] = (char *)args[i].data //base of 2D array\n')
    f.write('      +\n')
    f.write('      //y dimension -- get to the correct y line\n')
    f.write('      args[i].dat->size * args[i].dat->block_size[0] * ( range[2] * args[i].stencil->stride[1] - args[i].dat->offset[1] )\n')
    f.write('      +\n')
    f.write('      //x dimension - get to the correct x point on the y line\n')
    f.write('      args[i].dat->size * ( range[0] * args[i].stencil->stride[0] - args[i].dat->offset[0] );\n')
    f.write('      non_gbl[g++] = i;\n')
    f.write('    }\n')
    f.write('    else if (args[i].argtype == OPS_ARG_GBL)\n')
    f.write('      p_a[i] = (char *)args[i].data;\n')
    f.write('  }\n\n')

    f.write('  int total_range = 1;\n')
    f.write('  for (int m=0; m<dim; m++) {\n')
    f.write('    count[m] = range[2*m+1]-range[2*m];  // number in each dimension\n')
    f.write('    total_range *= count[m];\n')
    f.write('  }\n')
    f.write('  count[dim-1]++;     // extra in last to ensure correct termination\n\n')


    for n in range (0, nargs):
      f.write('  xdim'+str(n)+' = args['+str(n)+'].dat->block_size[0];\n')
    f.write('\n')

    f.write('  for (int nt=0; nt<total_range; nt++) {\n')

    f.write('    // call kernel function, passing in pointers to data\n')
    f.write('\n    kernel( ')
    for n in range (0, nargs):
        f.write(' (T'+str(n)+' *)p_a['+str(n)+']')
        if nargs <> 1 and n != nargs-1:
          f.write(',')
        else:
          f.write(' );\n\n')
        if n%n_per_line == 3 and n <> nargs-1:
          f.write('\n          ')

    f.write('    count[0]--;   // decrement counter\n')
    f.write('    int m = 0;    // max dimension with changed index\n')

    f.write('    while (count[m]==0) {\n')
    f.write('      count[m] = range[2*m+1]-range[2*m]; // reset counter\n')
    f.write('      m++;                                // next dimension\n')
    f.write('      count[m]--;                         // decrement counter\n')
    f.write('    }\n\n')

    f.write('    int a = 0;\n')
    f.write('    // shift pointers to data\n')
    f.write('    for (int i=0; i<'+str(nargs)+'; i++) {\n')
    f.write('        p_a[i] = p_a[i] + (args[i].dat->size * offs[i][m]);\n')
    f.write('    }\n')
    f.write('  }\n')


    f.write('}')
