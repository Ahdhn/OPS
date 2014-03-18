#!/usr/bin/env python

# Open source copyright declaration based on BSD open source template:
# http://www.opensource.org/licenses/bsd-license.php
#
# This file is part of the OPS distribution.
#
# Copyright (c) 2013, Mike Giles and others. Please see the AUTHORS file in
# the main source directory for a full list of copyright holders.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
# Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.
# Redistributions in binary form must reproduce the above copyright
# notice, this list of conditions and the following disclaimer in the
# documentation and/or other materials provided with the distribution.
# The name of Mike Giles may not be used to endorse or promote products
# derived from this software without specific prior written permission.
# THIS SOFTWARE IS PROVIDED BY Mike Giles ''AS IS'' AND ANY
# EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL Mike Giles BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#######################################################################
#                                                                     #
#       This Python routine generates the header file ops_seq.h       #
#                                                                     #
#######################################################################


#
# this sets the max number of arguments in ops_par_loop
#
maxargs = 18

#open/create file
f = open('./ops_mpi_seq.h','w')

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
#include "ops_mpi_core.h"


"""

f.write(top)

#
# now define the macros and extern vars
#

f.write('#ifndef OPS_ACC_MACROS\n')
f.write('#ifndef OPS_DEBUG\n')
for nargs in range (0,maxargs):
  f.write('#define OPS_ACC'+str(nargs)+'(x,y) (x+xdim'+str(nargs)+'*(y))\n')
f.write('#else\n\n')
for nargs in range (0,maxargs):
  f.write('#define OPS_ACC'+str(nargs)+'(x,y) (ops_stencil_check_2d('+str(nargs)+', x, y, xdim'+str(nargs)+', -1))\n')
f.write('#endif\n')
f.write('#endif\n\n')

for nargs in range (0,maxargs):
  f.write('extern int xdim'+str(nargs)+';\n')

functions =  """
inline int mult(int* size, int dim)
{
  int result = 1;
  if(dim > 0) {
    for(int i = 0; i<dim;i++) result *= size[i];
  }
  return result;
}

inline int add(int* coords, int* size, int dim)
{
  int result = coords[0];
  for(int i = 1; i<=dim;i++) result += coords[i]*mult(size,i);
  return result;
}


inline int off(int ndim, int dim , int* start, int* end, int* size, int* stride)
{

  int i = 0;
  int c1[ndim];
  int c2[ndim];

  for(i=0; i<ndim; i++) c1[i] = start[i];
  c1[dim] = start[dim] + 1*stride[dim];

  for(i = 0; i<dim; i++) stride[i]!=0 ? c2[i] = end[i]:c2[i] = start[i]+1;
  for(i=dim; i<ndim; i++) c2[i] = start[i];

  int off =  add(c1, size, dim) - add(c2, size, dim) + 1; //plus 1 to get the next element

  return off;
}

inline int address(int ndim, int dat_size, int* start, int* size, int* stride, int* off)
{
  int base = 0;
  for(int i=0; i<ndim; i++) {
    base = base + dat_size * mult(size, i) * (start[i] * stride[i] - off[i]);
  }
  return base;
}
"""

f.write(functions)

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

    f.write('void ops_par_loop(void (*kernel)(')
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


    f.write('    char const * name, ops_block block, int dim, int *range,\n    ')
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

    f.write('  sub_block_list sb = OPS_sub_block_list[block->index];\n')


    f.write('\n\n  //compute localy allocated range for the sub-block \n' +
    '  int ndim = sb->ndim;\n' )
    #'  int start[ndim*'+str(nargs)+'];\n' +
    #'  int end[ndim*'+str(nargs)+'];\n\n')
    f.write('  int start[ndim];\n');
    f.write('  int end[ndim];\n\n')

    f.write('  for (int n=0; n<ndim; n++) {\n')
    f.write('    start[n] = sb->istart[n];end[n] = sb->iend[n]+1;\n')
    f.write('    if (start[n] >= range[2*n]) start[n] = 0;\n')
    f.write('    else start[n] = range[2*n] - start[n];\n')
    f.write('    if (end[n] >= range[2*n+1]) end[n] = range[2*n+1] - sb->istart[n];\n')
    f.write('    else end[n] = sb->sizes[n];\n')
    f.write('  }\n')

    #f.write('  for(int i = 0; i<'+str(nargs)+'; i++) {\n' +
    #  '    for(int n=0; n<ndim; n++) {\n' +
    #  '      start[i*ndim+n] = s[n];\n' +
    #  '      end[i*ndim+n]   = e[n];\n' +
    #  '    }\n' +
    #  '  }\n\n')

    f.write('  #ifdef OPS_DEBUG\n')
    f.write('  ops_register_args(args, name);\n');
    f.write('  #endif\n\n')

    f.write('  for (int i = 0; i<'+str(nargs)+';i++) {\n')
    f.write('    if(args[i].stencil!=NULL) {\n')
    f.write('      offs[i][0] = args[i].stencil->stride[0]*1;  //unit step in x dimension\n')
    f.write('      for(int n=1; n<ndim; n++) {\n')
    f.write('        offs[i][n] = off(ndim, n, &start[0], &end[0],\n'+
            '                         args[i].dat->block_size, args[i].stencil->stride);\n')
    f.write('      }\n')
    f.write('    }\n')
    f.write('  }\n\n')

    f.write('  //set up initial pointers\n')
    f.write('  for (int i = 0; i < '+str(nargs)+'; i++) {\n')
    f.write('    if (args[i].argtype == OPS_ARG_DAT) {\n')
    f.write('      p_a[i] = (char *)args[i].data //base of 2D array\n')
    f.write('      + address(ndim, args[i].dat->size, &start[0], \n'+
            '        args[i].dat->block_size, args[i].stencil->stride, args[i].dat->offset);\n')
    f.write('    }\n')
    f.write('    else if (args[i].argtype == OPS_ARG_GBL)\n')
    f.write('      p_a[i] = (char *)args[i].data;\n')
    f.write('  }\n\n')


    f.write('  int total_range = 1;\n')
    f.write('  for (int n=0; n<ndim; n++) {\n')
    f.write('    count[n] = end[n]-start[n];  // number in each dimension\n')
    f.write('    total_range *= count[n];\n')
    f.write('  }\n')
    f.write('  count[dim-1]++;     // extra in last to ensure correct termination\n\n')


    for n in range (0, nargs):
      f.write('  if (args['+str(n)+'].argtype == OPS_ARG_DAT)')
      f.write('  xdim'+str(n)+' = args['+str(n)+'].dat->block_size[0];\n')
    f.write('\n')

    #f.write('  //calculate max halodepth for each dat\n')
    #f.write('  for (int i = 0; i<'+str(nargs)+';i++) {\n')
    #f.write('    int max_depth[ndim];\n')
    #f.write('    max_depth[0] = 0;\n')
    #f.write('    max_depth[1] = 0;\n')
    #f.write('    if(args[i].stencil!=NULL) {\n')
    #f.write('      for(int d = 0; d<ndim; d++) {\n')
    #f.write('        for (int p = 0; p<args[i].stencil->points; p++) {\n')
    #f.write('          max_depth[d] = MAX(max_depth[d], abs(args[i].stencil->stencil[p*ndim+d]));\n')
    #f.write('          if(max_depth[d]>2) printf("larger halo %d\\n",max_depth[d]);\n')
    #f.write('        }\n')
    #f.write('      }\n')
    #f.write('      if(args[i].argtype == OPS_ARG_DAT)\n')
    #f.write('        ops_exchange_halo(&args[i],max_depth);\n')
    #f.write('    }\n')
    #f.write('  }\n\n')

    f.write('  for (int i = 0; i < '+str(nargs)+'; i++) {\n')
    f.write('    if(args[i].argtype == OPS_ARG_DAT)\n')
    f.write('      ops_exchange_halo(&args[i],2);\n')
    f.write('  }\n\n')


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
    f.write('      count[m] =  end[m]-start[m];// reset counter\n')
    f.write('      m++;                        // next dimension\n')
    f.write('      count[m]--;                 // decrement counter\n')
    f.write('    }\n\n')

    f.write('    // shift pointers to data\n')
    f.write('    for (int i=0; i<'+str(nargs)+'; i++) {\n')
    f.write('      if (args[i].argtype == OPS_ARG_DAT)\n')
    f.write('        p_a[i] = p_a[i] + (args[i].dat->size * offs[i][m]);\n')
    f.write('    }\n')
    f.write('  }\n\n')

    for n in range (0, nargs):
      f.write('  if (args['+str(n)+'].argtype == OPS_ARG_GBL && args['+str(n)+'].acc != OPS_READ)')
      f.write('  ops_mpi_reduce(&arg'+str(n)+',(T'+str(n)+' *)p_a['+str(n)+']);\n')
    f.write('\n')

    for n in range (0, nargs):
      f.write('  if (args['+str(n)+'].argtype == OPS_ARG_DAT)')
      f.write('  ops_set_halo_dirtybit(&args['+str(n)+']);\n')


    f.write('}')
