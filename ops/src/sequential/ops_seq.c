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

/** @brief ops sequential backend implementation
  * @author Gihan Mudalige
  * @details Implements the OPS API calls for the sequential backend
  */

#include <ops_lib_cpp.h>

int xdim0;
int xdim1;
int xdim2;
int xdim3;
int xdim4;
int xdim5;
int xdim6;
int xdim7;
int xdim8;
int xdim9;
int xdim10;
int xdim11;
int xdim12;
int xdim13;
int xdim14;
int xdim15;
int xdim16;
int xdim17;


ops_dat ops_decl_dat_char (ops_block block, int size, int *block_size,
                           int* offset,  char* data, int type_size,
                           char const * type, char const * name )
{
  ops_dat dat;

  if(data != NULL) {
    dat = ops_decl_dat_core(block, size, block_size, offset, data, type_size, type, name);
  }
  else {
    dat = ops_decl_dat_temp_core (block, size, block_size, offset,
                                           data, type_size, type, name );
    int bytes = size*type_size;
    for (int i=0; i<block->dims; i++) bytes = bytes*block_size[i];
    dat->data = (char*) calloc(bytes, 1); //initialize data bits to 0
    dat->user_managed = 0;
  }

  return dat;
}

ops_arg ops_arg_dat( ops_dat dat, ops_stencil stencil, char const * type, ops_access acc )
{
  return ops_arg_dat_core( dat, stencil, acc );
}

ops_arg ops_arg_gbl_char( char * data, int dim, int size, ops_access acc )
{
  return ops_arg_gbl_core( data, dim, size, acc );
}

void ops_print_dat_to_txtfile(ops_dat dat, const char *file_name)
{
  ops_print_dat_to_txtfile_core(dat, file_name);
}

void ops_timers(double * cpu, double * et)
{
    ops_timers_core(cpu,et);
}
