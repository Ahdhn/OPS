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

/** @brief headder file declaring the functions for the ops sequential backend
  * @author Gihan Mudalige, Istvan Reguly
  * @details Declares the OPS API calls for the sequential backend
  */

#include "ops_lib_cpp.h"

inline void ops_arg_set(int n_x, ops_arg arg, char **p_arg){
  if (arg.stencil!=NULL) {

    for (int i = 0; i < arg.stencil->points; i++){
      p_arg[i] =
         arg.data + //base of 1D array
         arg.dat->size * ( //multiply by the number of bytes per element
         (n_x - arg.dat->offset[0]) * //calculate the offset from index 0
         arg.stencil->stride[0]  + // jump in strides ??
         arg.stencil->stencil[i * arg.stencil->dims + 0] //get the value at the ith stencil point
         );
    }
  } else {
    *p_arg = arg.data;
  }
}

inline void ops_arg_set(int n_x,
                        int n_y, ops_arg arg, char **p_arg){
  if (arg.stencil!=NULL) {
    for (int i = 0; i < arg.stencil->points; i++){
      p_arg[i] =
        arg.data + //base of 2D array
        //y dimension -- get to the correct y line
        arg.dat->size * arg.dat->block_size[0] * ( //multiply by the number of
                                                    //bytes per element and xdim block size

        //(n_y - arg.dat->offset[1]) * // calculate the offset from index 0 for y dim
        //arg.stencil->stride[1] + // jump in strides in y dim ??
        n_y * arg.stencil->stride[1] - arg.dat->offset[1] +
        arg.stencil->stencil[i*arg.stencil->dims + 1]) //get the value at the ith
                                                       //stencil point "+ 1" is the y dim
        +
        //x dimension - get to the correct x point on the y line
        arg.dat->size * ( //multiply by the number of bytes per element

        //(n_x - arg.dat->offset[0]) * //calculate the offset from index 0 for x dim
        //arg.stencil->stride[0] + // jump in strides in x dim ??
        n_x * arg.stencil->stride[0] - arg.dat->offset[0] +
        arg.stencil->stencil[i*arg.stencil->dims + 0] //get the value at the ith
                                                      //stencil point "+ 0" is the x dim
      );
    }
  } else {
    *p_arg = arg.data;
  }
}

inline void ops_arg_set(int n_x,
                        int n_y,
                        int n_z, ops_arg arg, char **p_arg){
  if (arg.stencil!=NULL) {
    for (int i = 0; i < arg.stencil->points; i++)
      p_arg[i] =
      arg.data +
      //z dimension - get to the correct z plane
      arg.dat->size * arg.dat->block_size[1] * arg.dat->block_size[0] * (
      (n_z - arg.dat->offset[2]) *
      arg.stencil->stride[2] +
      arg.stencil->stencil[i*arg.stencil->dims + 2])
      +
      //y dimension -- get to the correct y line on the z plane
      arg.dat->size * arg.dat->block_size[0] * (
      (n_y - arg.dat->offset[1]) *
      arg.stencil->stride[1] +
      arg.stencil->stencil[i*arg.stencil->dims + 1])
      +
      //x dimension - get to the correct x point on the y line
      arg.dat->size * (
      (n_x - arg.dat->offset[0]) *
      arg.stencil->stride[0] +
      arg.stencil->stencil[i*arg.stencil->dims + 0]
      );
  } else {
    *p_arg = arg.data;
  }
}



inline void ops_args_set(int iter_x,
                         int nargs, ops_arg *args, char ***p_a){
  for (int n=0; n<nargs; n++) {
    ops_arg_set(iter_x, args[n], p_a[n]);
  }
}

inline void ops_args_set(int iter_x,
                         int iter_y,
                         int nargs, ops_arg *args, char ***p_a){
  for (int n=0; n<nargs; n++) {
    ops_arg_set(iter_x, iter_y, args[n], p_a[n]);
  }
}

inline void ops_args_set(int iter_x,
                         int iter_y,
                         int iter_z, int nargs, ops_arg *args, char ***p_a){
  for (int n=0; n<nargs; n++) {
    ops_arg_set(iter_x, iter_y, iter_z, args[n], p_a[n]);
  }
}


template < class T0 >
void ops_par_loop(void (*kernel)( T0* ),
  char const * name, int dim, int *range,
  ops_arg arg0 ) {

  char** p_a[1];
  ops_arg args[1] = {arg0};

  // consistency checks
  // ops_args_check(1,args,name);

  for (int i = 0; i < 1; i++) {
    if (args[i].argtype == OPS_ARG_DAT)
      p_a[i] = (char **)malloc(args[i].stencil->points * sizeof(char *));
    else if (args[i].argtype == OPS_ARG_GBL)
      p_a[i] = (char **)malloc(args[i].dim * sizeof(char *));
  }

  // loop over set elements

  if (dim == 1) {
    for (int n_x = range[0]; n_x < range[1]; n_x++) {
      ops_args_set(n_x, 1,args,p_a);
      // call kernel function, passing in pointers to data
      kernel( (T0 *)p_a[0] );
    }
  }
  else if (dim == 2) {
    for (int n_y = range[2]; n_y < range[3]; n_y++) {
      for (int n_x = range[0]; n_x < range[1]; n_x++) {
        ops_args_set(n_x, n_y,1,args,p_a);
        // call kernel function, passing in pointers to data
        kernel( (T0 *)p_a[0] );
      }
    }
  }
  else if (dim == 3) {
    for (int n_z = range[4]; n_z < range[5]; n_z++) {
      for (int n_y = range[2]; n_y < range[3]; n_y++) {
        for (int n_x = range[0]; n_x < range[1]; n_x++) {
          ops_args_set(n_x, n_y, n_z,1,args,p_a);
          // call kernel function, passing in pointers to data
          kernel( (T0 *)p_a[0] );
        }
      }
    }
  }

  for (int i = 0; i < 1; i++) {
    if (args[i].argtype == OPS_ARG_DAT)
      free(p_a[i]);
  }
}


template < class T0, class T1 >
void ops_par_loop(void (*kernel)( T0*, T1* ),
  char const * name, int dim, int *range,
  ops_arg arg0,
  ops_arg arg1) {

  char** p_a[2];
  ops_arg args[2] = {arg0, arg1};

  // consistency checks
  // ops_args_check(2,args,name);

  for (int i = 0; i < 2; i++) {
    if (args[i].argtype == OPS_ARG_DAT)
      p_a[i] = (char **)malloc(args[i].stencil->points * sizeof(char *));
    else if (args[i].argtype == OPS_ARG_GBL)
      p_a[i] = (char **)malloc(args[i].dim * sizeof(char *));
  }

  // loop over set elements

  if (dim == 1) {
    for (int n_x = range[0]; n_x < range[1]; n_x++) {
      ops_args_set(n_x, 2, args,p_a);
      // call kernel function, passing in pointers to data
      kernel( (T0 *)p_a[0], (T1 *)p_a[1]);
    }
  }
  else if (dim == 2) {
    for (int n_y = range[2]; n_y < range[3]; n_y++) {
      for (int n_x = range[0]; n_x < range[1]; n_x++) {
        ops_args_set(n_x, n_y, 2, args,p_a);
        // call kernel function, passing in pointers to data
        kernel( (T0 *)p_a[0], (T1 *)p_a[1] );
      }
    }
  }
  else if (dim == 3) {
    for (int n_z = range[4]; n_z < range[5]; n_z++) {
      for (int n_y = range[2]; n_y < range[3]; n_y++) {
        for (int n_x = range[0]; n_x < range[1]; n_x++) {
          ops_args_set(n_x, n_y, n_z, 2, args,p_a);
          // call kernel function, passing in pointers to data
          kernel( (T0 *)p_a[0], (T1 *)p_a[1] );
        }
      }
    }
  }

  for (int i = 0; i < 2; i++) {
    if (args[i].argtype == OPS_ARG_DAT)
      free(p_a[i]);
  }
}


template < class T0, class T1, class T2 >
void ops_par_loop(void (*kernel)( T0*, T1*, T2* ),
  char const * name, int dim, int *range,
  ops_arg arg0,
  ops_arg arg1,
  ops_arg arg2) {

  char** p_a[3];
  ops_arg args[3] = {arg0, arg1, arg2};

  // consistency checks
  // ops_args_check(3,args,name);

  for (int i = 0; i < 3; i++) {
    if (args[i].argtype == OPS_ARG_DAT)
      p_a[i] = (char **)malloc(args[i].stencil->points * sizeof(char *));
    else if (args[i].argtype == OPS_ARG_GBL)
      p_a[i] = (char **)malloc(args[i].dim * sizeof(char *));
  }

  // loop over set elements

  if (dim == 1) {
    for (int n_x = range[0]; n_x < range[1]; n_x++) {
      ops_args_set(n_x, 3, args,p_a);
      // call kernel function, passing in pointers to data
      kernel( (T0 *)p_a[0], (T1 *)p_a[1], (T2 *)p_a[2]);
    }
  }
  else if (dim == 2) {
    for (int n_y = range[2]; n_y < range[3]; n_y++) {
      for (int n_x = range[0]; n_x < range[1]; n_x++) {
        ops_args_set(n_x, n_y, 3, args,p_a);
        // call kernel function, passing in pointers to data
        kernel( (T0 *)p_a[0], (T1 *)p_a[1], (T2 *)p_a[2] );
      }
    }
  }
  else if (dim == 3) {
    for (int n_z = range[4]; n_z < range[5]; n_z++) {
      for (int n_y = range[2]; n_y < range[3]; n_y++) {
        for (int n_x = range[0]; n_x < range[1]; n_x++) {
          ops_args_set(n_x, n_y, n_z, 3, args,p_a);
          // call kernel function, passing in pointers to data
          kernel( (T0 *)p_a[0], (T1 *)p_a[1], (T2 *)p_a[2] );
        }
      }
    }
  }

  for (int i = 0; i < 3; i++) {
    if (args[i].argtype == OPS_ARG_DAT)
      free(p_a[i]);
  }
}

template < class T0, class T1, class T2, class T3 >
void ops_par_loop(void (*kernel)( T0*, T1*, T2*, T3* ),
  char const * name, int dim, int *range,
  ops_arg arg0,
  ops_arg arg1,
  ops_arg arg2,
  ops_arg arg3) {

  char** p_a[4];
  ops_arg args[4] = {arg0, arg1, arg2, arg3};

  // consistency checks
  // ops_args_check(3,args,name);

  for (int i = 0; i < 4; i++) {
    if (args[i].argtype == OPS_ARG_DAT)
      p_a[i] = (char **)malloc(args[i].stencil->points * sizeof(char *));
    else if (args[i].argtype == OPS_ARG_GBL)
      p_a[i] = (char **)malloc(args[i].dim * sizeof(char *));
  }

  // loop over set elements

  if (dim == 1) {
    for (int n_x = range[0]; n_x < range[1]; n_x++) {
      ops_args_set(n_x, 4, args,p_a);
      // call kernel function, passing in pointers to data
      kernel( (T0 *)p_a[0], (T1 *)p_a[1], (T2 *)p_a[2], (T3 *)p_a[3]);
    }
  }
  else if (dim == 2) {
    for (int n_y = range[2]; n_y < range[3]; n_y++) {
      for (int n_x = range[0]; n_x < range[1]; n_x++) {
        ops_args_set(n_x, n_y, 4, args,p_a);
        // call kernel function, passing in pointers to data
        kernel( (T0 *)p_a[0], (T1 *)p_a[1], (T2 *)p_a[2], (T3 *)p_a[3] );
      }
    }
  }
  else if (dim == 3) {
    for (int n_z = range[4]; n_z < range[5]; n_z++) {
      for (int n_y = range[2]; n_y < range[3]; n_y++) {
        for (int n_x = range[0]; n_x < range[1]; n_x++) {
          ops_args_set(n_x, n_y, n_z, 4, args,p_a);
          // call kernel function, passing in pointers to data
          kernel( (T0 *)p_a[0], (T1 *)p_a[1], (T2 *)p_a[2], (T3 *)p_a[3] );
        }
      }
    }
  }

  for (int i = 0; i < 4; i++) {
    if (args[i].argtype == OPS_ARG_DAT)
      free(p_a[i]);
  }
}

template < class T0, class T1, class T2, class T3, class T4 >
void ops_par_loop(void (*kernel)( T0*, T1*, T2*, T3*, T4* ),
  char const * name, int dim, int *range,
  ops_arg arg0,
  ops_arg arg1,
  ops_arg arg2,
  ops_arg arg3,
  ops_arg arg4) {

  char** p_a[5];
  ops_arg args[5] = {arg0, arg1, arg2, arg3, arg4};

  // consistency checks
  // ops_args_check(3,args,name);

  for (int i = 0; i < 5; i++) {
    if (args[i].argtype == OPS_ARG_DAT)
      p_a[i] = (char **)malloc(args[i].stencil->points * sizeof(char *));
    else if (args[i].argtype == OPS_ARG_GBL)
      p_a[i] = (char **)malloc(args[i].dim * sizeof(char *));
  }

  // loop over set elements

  if (dim == 1) {
    for (int n_x = range[0]; n_x < range[1]; n_x++) {
      ops_args_set(n_x, 5, args,p_a);
      // call kernel function, passing in pointers to data
      kernel( (T0 *)p_a[0], (T1 *)p_a[1], (T2 *)p_a[2], (T3 *)p_a[3],
              (T4 *)p_a[4]);
    }
  }
  else if (dim == 2) {
    for (int n_y = range[2]; n_y < range[3]; n_y++) {
      for (int n_x = range[0]; n_x < range[1]; n_x++) {
        ops_args_set(n_x, n_y, 5, args,p_a);
        // call kernel function, passing in pointers to data
        kernel( (T0 *)p_a[0], (T1 *)p_a[1], (T2 *)p_a[2], (T3 *)p_a[3],
                (T4 *)p_a[4]);
      }
    }
  }
  else if (dim == 3) {
    for (int n_z = range[4]; n_z < range[5]; n_z++) {
      for (int n_y = range[2]; n_y < range[3]; n_y++) {
        for (int n_x = range[0]; n_x < range[1]; n_x++) {
          ops_args_set(n_x, n_y, n_z, 5, args,p_a);
          // call kernel function, passing in pointers to data
          kernel( (T0 *)p_a[0], (T1 *)p_a[1], (T2 *)p_a[2], (T3 *)p_a[3],
                  (T4 *)p_a[4]);
        }
      }
    }
  }

  for (int i = 0; i < 5; i++) {
    if (args[i].argtype == OPS_ARG_DAT)
      free(p_a[i]);
  }
}


template < class T0, class T1, class T2, class T3, class T4, class T5 >
void ops_par_loop(void (*kernel)( T0*, T1*, T2*, T3*, T4*, T5* ),
  char const * name, int dim, int *range,
  ops_arg arg0,
  ops_arg arg1,
  ops_arg arg2,
  ops_arg arg3,
  ops_arg arg4,
  ops_arg arg5) {

  char** p_a[6];
  ops_arg args[6] = {arg0, arg1, arg2, arg3, arg4, arg5};

  // consistency checks
  // ops_args_check(3,args,name);

  for (int i = 0; i < 6; i++) {
    if (args[i].argtype == OPS_ARG_DAT)
      p_a[i] = (char **)malloc(args[i].stencil->points * sizeof(char *));
    else if (args[i].argtype == OPS_ARG_GBL)
      p_a[i] = (char **)malloc(args[i].dim * sizeof(char *));
  }

  // loop over set elements

  if (dim == 1) {
    for (int n_x = range[0]; n_x < range[1]; n_x++) {
      ops_args_set(n_x, 6, args,p_a);
      // call kernel function, passing in pointers to data
      kernel( (T0 *)p_a[0], (T1 *)p_a[1], (T2 *)p_a[2], (T3 *)p_a[3],
              (T4 *)p_a[4], (T5 *)p_a[5]);
    }
  }
  else if (dim == 2) {
    for (int n_y = range[2]; n_y < range[3]; n_y++) {
      for (int n_x = range[0]; n_x < range[1]; n_x++) {
        ops_args_set(n_x, n_y, 6, args,p_a);
        // call kernel function, passing in pointers to data
        kernel( (T0 *)p_a[0], (T1 *)p_a[1], (T2 *)p_a[2], (T3 *)p_a[3],
                (T4 *)p_a[4], (T5 *)p_a[5]);
      }
    }
  }
  else if (dim == 3) {
    for (int n_z = range[4]; n_z < range[5]; n_z++) {
      for (int n_y = range[2]; n_y < range[3]; n_y++) {
        for (int n_x = range[0]; n_x < range[1]; n_x++) {
          ops_args_set(n_x, n_y, n_z, 6, args,p_a);
          // call kernel function, passing in pointers to data
          kernel( (T0 *)p_a[0], (T1 *)p_a[1], (T2 *)p_a[2], (T3 *)p_a[3],
                  (T4 *)p_a[4], (T5 *)p_a[5]);
        }
      }
    }
  }

  for (int i = 0; i < 6; i++) {
    if (args[i].argtype == OPS_ARG_DAT)
      free(p_a[i]);
  }
}


template < class T0, class T1, class T2, class T3, class T4, class T5, class T6  >
void ops_par_loop(void (*kernel)( T0*, T1*, T2*, T3*, T4*, T5*, T6* ),
  char const * name, int dim, int *range,
  ops_arg arg0,
  ops_arg arg1,
  ops_arg arg2,
  ops_arg arg3,
  ops_arg arg4,
  ops_arg arg5,
  ops_arg arg6) {

  char** p_a[7];
  ops_arg args[7] = {arg0, arg1, arg2, arg3, arg4, arg5, arg6};

  // consistency checks
  // ops_args_check(3,args,name);

  for (int i = 0; i < 7; i++) {
    if (args[i].argtype == OPS_ARG_DAT)
      p_a[i] = (char **)malloc(args[i].stencil->points * sizeof(char *));
    else if (args[i].argtype == OPS_ARG_GBL)
      p_a[i] = (char **)malloc(args[i].dim * sizeof(char *));
  }

  // loop over set elements

  if (dim == 1) {
    for (int n_x = range[0]; n_x < range[1]; n_x++) {
      ops_args_set(n_x, 7, args,p_a);
      // call kernel function, passing in pointers to data
      kernel( (T0 *)p_a[0], (T1 *)p_a[1], (T2 *)p_a[2], (T3 *)p_a[3],
              (T4 *)p_a[4], (T5 *)p_a[5], (T6 *)p_a[6]);
    }
  }
  else if (dim == 2) {
    for (int n_y = range[2]; n_y < range[3]; n_y++) {
      for (int n_x = range[0]; n_x < range[1]; n_x++) {
        ops_args_set(n_x, n_y, 7, args,p_a);
        // call kernel function, passing in pointers to data
        kernel( (T0 *)p_a[0], (T1 *)p_a[1], (T2 *)p_a[2], (T3 *)p_a[3],
                (T4 *)p_a[4], (T5 *)p_a[5], (T6 *)p_a[6]);
      }
    }
  }
  else if (dim == 3) {
    for (int n_z = range[4]; n_z < range[5]; n_z++) {
      for (int n_y = range[2]; n_y < range[3]; n_y++) {
        for (int n_x = range[0]; n_x < range[1]; n_x++) {
          ops_args_set(n_x, n_y, n_z, 7, args,p_a);
          // call kernel function, passing in pointers to data
          kernel( (T0 *)p_a[0], (T1 *)p_a[1], (T2 *)p_a[2], (T3 *)p_a[3],
                  (T4 *)p_a[4], (T5 *)p_a[5], (T6 *)p_a[6]);
        }
      }
    }
  }

  for (int i = 0; i < 7; i++) {
    if (args[i].argtype == OPS_ARG_DAT)
      free(p_a[i]);
  }
}


template < class T0, class T1, class T2, class T3, class T4, class T5,
           class T6, class T7  >
void ops_par_loop(void (*kernel)( T0*, T1*, T2*, T3*, T4*, T5*, T6*, T7* ),
  char const * name, int dim, int *range,
  ops_arg arg0,
  ops_arg arg1,
  ops_arg arg2,
  ops_arg arg3,
  ops_arg arg4,
  ops_arg arg5,
  ops_arg arg6,
  ops_arg arg7) {

  char** p_a[8];
  ops_arg args[8] = {arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7};

  // consistency checks
  // ops_args_check(3,args,name);

  for (int i = 0; i < 8; i++) {
    if (args[i].argtype == OPS_ARG_DAT)
      p_a[i] = (char **)malloc(args[i].stencil->points * sizeof(char *));
    else if (args[i].argtype == OPS_ARG_GBL)
      p_a[i] = (char **)malloc(args[i].dim * sizeof(char *));
  }

  // loop over set elements

  if (dim == 1) {
    for (int n_x = range[0]; n_x < range[1]; n_x++) {
      ops_args_set(n_x, 8, args,p_a);
      // call kernel function, passing in pointers to data
      kernel( (T0 *)p_a[0], (T1 *)p_a[1], (T2 *)p_a[2], (T3 *)p_a[3],
              (T4 *)p_a[4], (T5 *)p_a[5], (T6 *)p_a[6], (T7 *)p_a[7]);
    }
  }
  else if (dim == 2) {
    for (int n_y = range[2]; n_y < range[3]; n_y++) {
      for (int n_x = range[0]; n_x < range[1]; n_x++) {
        ops_args_set(n_x, n_y, 8, args,p_a);
        // call kernel function, passing in pointers to data
        kernel( (T0 *)p_a[0], (T1 *)p_a[1], (T2 *)p_a[2], (T3 *)p_a[3],
                (T4 *)p_a[4], (T5 *)p_a[5], (T6 *)p_a[6], (T7 *)p_a[7]);
      }
    }
  }
  else if (dim == 3) {
    for (int n_z = range[4]; n_z < range[5]; n_z++) {
      for (int n_y = range[2]; n_y < range[3]; n_y++) {
        for (int n_x = range[0]; n_x < range[1]; n_x++) {
          ops_args_set(n_x, n_y, n_z, 8, args,p_a);
          // call kernel function, passing in pointers to data
          kernel( (T0 *)p_a[0], (T1 *)p_a[1], (T2 *)p_a[2], (T3 *)p_a[3],
                  (T4 *)p_a[4], (T5 *)p_a[5], (T6 *)p_a[6], (T7 *)p_a[7]);
        }
      }
    }
  }

  for (int i = 0; i < 8; i++) {
    if (args[i].argtype == OPS_ARG_DAT)
      free(p_a[i]);
  }
}


template < class T0, class T1, class T2, class T3, class T4, class T5,
           class T6, class T7, class T8>
void ops_par_loop(void (*kernel)( T0*, T1*, T2*, T3*, T4*, T5*, T6*, T7*,
                                  T8*),
  char const * name, int dim, int *range,
  ops_arg arg0,
  ops_arg arg1,
  ops_arg arg2,
  ops_arg arg3,
  ops_arg arg4,
  ops_arg arg5,
  ops_arg arg6,
  ops_arg arg7,
  ops_arg arg8 ) {

  char** p_a[9];
  ops_arg args[9] = {arg0, arg1, arg2, arg3, arg4,
                      arg5, arg6, arg7, arg8};

  // consistency checks
  // ops_args_check(9,args,name);

  for (int i = 0; i < 9; i++) {
    if (args[i].argtype == OPS_ARG_DAT)
      p_a[i] = (char **)malloc(args[i].stencil->points * sizeof(char *));
    else if (args[i].argtype == OPS_ARG_GBL)
      p_a[i] = (char **)malloc(args[i].dim * sizeof(char *));
  }

  // loop over set elements

  if (dim == 1) {
    for (int n_x = range[0]; n_x < range[1]; n_x++) {
      ops_args_set(n_x, 9, args,p_a);
      // call kernel function, passing in pointers to data
      kernel( (T0 *)p_a[0], (T1 *)p_a[1], (T2 *)p_a[2], (T3 *)p_a[3],
              (T4 *)p_a[4], (T5 *)p_a[5], (T6 *)p_a[6], (T7 *)p_a[7],
              (T8 *)p_a[8]);
    }
  }
  else if (dim == 2) {
    for (int n_y = range[2]; n_y < range[3]; n_y++) {
      for (int n_x = range[0]; n_x < range[1]; n_x++) {
        ops_args_set(n_x, n_y, 9, args,p_a);
        // call kernel function, passing in pointers to data
        kernel( (T0 *)p_a[0], (T1 *)p_a[1], (T2 *)p_a[2], (T3 *)p_a[3],
              (T4 *)p_a[4], (T5 *)p_a[5], (T6 *)p_a[6], (T7 *)p_a[7],
              (T8 *)p_a[8]);
      }
    }
  }
  else if (dim == 3) {
    for (int n_z = range[4]; n_z < range[5]; n_z++) {
      for (int n_y = range[2]; n_y < range[3]; n_y++) {
        for (int n_x = range[0]; n_x < range[1]; n_x++) {
          ops_args_set(n_x, n_y, n_z, 9, args,p_a);
          // call kernel function, passing in pointers to data
          kernel( (T0 *)p_a[0], (T1 *)p_a[1], (T2 *)p_a[2], (T3 *)p_a[3],
              (T4 *)p_a[4], (T5 *)p_a[5], (T6 *)p_a[6], (T7 *)p_a[7],
              (T8 *)p_a[8]);
        }
      }
    }
  }

  for (int i = 0; i < 9; i++) {
    if (args[i].argtype == OPS_ARG_DAT)
      free(p_a[i]);
  }
}


template < class T0, class T1, class T2, class T3, class T4, class T5,
           class T6, class T7, class T8 , class T9>
void ops_par_loop(void (*kernel)( T0*, T1*, T2*, T3*, T4*, T5*, T6*, T7*,
                                  T8*, T9*),
  char const * name, int dim, int *range,
  ops_arg arg0,
  ops_arg arg1,
  ops_arg arg2,
  ops_arg arg3,
  ops_arg arg4,
  ops_arg arg5,
  ops_arg arg6,
  ops_arg arg7,
  ops_arg arg8,
  ops_arg arg9 ) {

  char** p_a[10];
  ops_arg args[10] = {arg0, arg1, arg2, arg3, arg4,
                      arg5, arg6, arg7, arg8, arg9};

  // consistency checks
  // ops_args_check(16,args,name);

  for (int i = 0; i < 10; i++) {
    if (args[i].argtype == OPS_ARG_DAT)
      p_a[i] = (char **)malloc(args[i].stencil->points * sizeof(char *));
    else if (args[i].argtype == OPS_ARG_GBL)
      p_a[i] = (char **)malloc(args[i].dim * sizeof(char *));
  }

  // loop over set elements

  if (dim == 1) {
    for (int n_x = range[0]; n_x < range[1]; n_x++) {
      ops_args_set(n_x, 10, args,p_a);
      // call kernel function, passing in pointers to data
      kernel( (T0 *)p_a[0], (T1 *)p_a[1], (T2 *)p_a[2], (T3 *)p_a[3],
              (T4 *)p_a[4], (T5 *)p_a[5], (T6 *)p_a[6], (T7 *)p_a[7],
              (T8 *)p_a[8], (T9 *)p_a[9]);
    }
  }
  else if (dim == 2) {
    for (int n_y = range[2]; n_y < range[3]; n_y++) {
      for (int n_x = range[0]; n_x < range[1]; n_x++) {
        ops_args_set(n_x, n_y, 10, args,p_a);
        // call kernel function, passing in pointers to data
        kernel( (T0 *)p_a[0], (T1 *)p_a[1], (T2 *)p_a[2], (T3 *)p_a[3],
              (T4 *)p_a[4], (T5 *)p_a[5], (T6 *)p_a[6], (T7 *)p_a[7],
              (T8 *)p_a[8], (T9 *)p_a[9]);
      }
    }
  }
  else if (dim == 3) {
    for (int n_z = range[4]; n_z < range[5]; n_z++) {
      for (int n_y = range[2]; n_y < range[3]; n_y++) {
        for (int n_x = range[0]; n_x < range[1]; n_x++) {
          ops_args_set(n_x, n_y, n_z, 10, args,p_a);
          // call kernel function, passing in pointers to data
          kernel( (T0 *)p_a[0], (T1 *)p_a[1], (T2 *)p_a[2], (T3 *)p_a[3],
              (T4 *)p_a[4], (T5 *)p_a[5], (T6 *)p_a[6], (T7 *)p_a[7],
              (T8 *)p_a[8], (T9 *)p_a[9]);
        }
      }
    }
  }

  for (int i = 0; i < 10; i++) {
    if (args[i].argtype == OPS_ARG_DAT)
      free(p_a[i]);
  }
}

template < class T0, class T1, class T2, class T3, class T4, class T5,
           class T6, class T7, class T8 , class T9 , class T10   >
void ops_par_loop(void (*kernel)( T0*, T1*, T2*, T3*, T4*, T5*, T6*, T7*,
                                  T8*, T9*, T10*),
  char const * name, int dim, int *range,
  ops_arg arg0,
  ops_arg arg1,
  ops_arg arg2,
  ops_arg arg3,
  ops_arg arg4,
  ops_arg arg5,
  ops_arg arg6,
  ops_arg arg7,
  ops_arg arg8,
  ops_arg arg9,
  ops_arg arg10) {

  char** p_a[11];
  ops_arg args[11] = {arg0, arg1, arg2, arg3, arg4,
                      arg5, arg6, arg7, arg8, arg9, arg10};

  // consistency checks
  // ops_args_check(3,args,name);

  for (int i = 0; i < 11; i++) {
    if (args[i].argtype == OPS_ARG_DAT)
      p_a[i] = (char **)malloc(args[i].stencil->points * sizeof(char *));
    else if (args[i].argtype == OPS_ARG_GBL)
      p_a[i] = (char **)malloc(args[i].dim * sizeof(char *));
  }

  // loop over set elements

  if (dim == 1) {
    for (int n_x = range[0]; n_x < range[1]; n_x++) {
      ops_args_set(n_x, 11, args,p_a);
      // call kernel function, passing in pointers to data
      kernel( (T0 *)p_a[0], (T1 *)p_a[1], (T2 *)p_a[2], (T3 *)p_a[3],
              (T4 *)p_a[4], (T5 *)p_a[5], (T6 *)p_a[6], (T7 *)p_a[7],
              (T8 *)p_a[8], (T9 *)p_a[9], (T10 *)p_a[10]);
    }
  }
  else if (dim == 2) {
    for (int n_y = range[2]; n_y < range[3]; n_y++) {
      for (int n_x = range[0]; n_x < range[1]; n_x++) {
        ops_args_set(n_x, n_y, 11, args,p_a);
        // call kernel function, passing in pointers to data
        kernel( (T0 *)p_a[0], (T1 *)p_a[1], (T2 *)p_a[2], (T3 *)p_a[3],
                (T4 *)p_a[4], (T5 *)p_a[5], (T6 *)p_a[6], (T7 *)p_a[7],
                (T8 *)p_a[8], (T9 *)p_a[9], (T10 *)p_a[10]);
      }
    }
  }
  else if (dim == 3) {
    for (int n_z = range[4]; n_z < range[5]; n_z++) {
      for (int n_y = range[2]; n_y < range[3]; n_y++) {
        for (int n_x = range[0]; n_x < range[1]; n_x++) {
          ops_args_set(n_x, n_y, n_z, 11, args,p_a);
          // call kernel function, passing in pointers to data
          kernel( (T0 *)p_a[0], (T1 *)p_a[1], (T2 *)p_a[2], (T3 *)p_a[3],
                  (T4 *)p_a[4], (T5 *)p_a[5], (T6 *)p_a[6], (T7 *)p_a[7],
                  (T8 *)p_a[8], (T9 *)p_a[9], (T10 *)p_a[10]);
        }
      }
    }
  }

  for (int i = 0; i < 11; i++) {
    if (args[i].argtype == OPS_ARG_DAT)
      free(p_a[i]);
  }
}


template < class T0, class T1, class T2, class T3, class T4, class T5,
           class T6, class T7, class T8 , class T9 , class T10, class T11>
void ops_par_loop(void (*kernel)( T0*, T1*, T2*, T3*, T4*, T5*, T6*, T7*,
                                  T8*, T9*, T10*, T11*),
  char const * name, int dim, int *range,
  ops_arg arg0,
  ops_arg arg1,
  ops_arg arg2,
  ops_arg arg3,
  ops_arg arg4,
  ops_arg arg5,
  ops_arg arg6,
  ops_arg arg7,
  ops_arg arg8,
  ops_arg arg9,
  ops_arg arg10,
  ops_arg arg11) {

  char** p_a[12];
  ops_arg args[12] = {arg0, arg1, arg2, arg3, arg4,
                      arg5, arg6, arg7, arg8, arg9,
                      arg10, arg11};

  // consistency checks
  // ops_args_check(16,args,name);

  for (int i = 0; i < 12; i++) {
    if (args[i].argtype == OPS_ARG_DAT)
      p_a[i] = (char **)malloc(args[i].stencil->points * sizeof(char *));
    else if (args[i].argtype == OPS_ARG_GBL)
      p_a[i] = (char **)malloc(args[i].dim * sizeof(char *));
  }

  // loop over set elements

  if (dim == 1) {
    for (int n_x = range[0]; n_x < range[1]; n_x++) {
      ops_args_set(n_x, 12, args,p_a);
      // call kernel function, passing in pointers to data
      kernel( (T0 *)p_a[0], (T1 *)p_a[1], (T2 *)p_a[2], (T3 *)p_a[3],
              (T4 *)p_a[4], (T5 *)p_a[5], (T6 *)p_a[6], (T7 *)p_a[7],
              (T8 *)p_a[8], (T9 *)p_a[9], (T10 *)p_a[10], (T11 *)p_a[11]);
    }
  }
  else if (dim == 2) {
    for (int n_y = range[2]; n_y < range[3]; n_y++) {
      for (int n_x = range[0]; n_x < range[1]; n_x++) {
        ops_args_set(n_x, n_y, 12, args,p_a);
        // call kernel function, passing in pointers to data
        kernel( (T0 *)p_a[0], (T1 *)p_a[1], (T2 *)p_a[2], (T3 *)p_a[3],
              (T4 *)p_a[4], (T5 *)p_a[5], (T6 *)p_a[6], (T7 *)p_a[7],
              (T8 *)p_a[8], (T9 *)p_a[9], (T10 *)p_a[10], (T11 *)p_a[11]);
      }
    }
  }
  else if (dim == 3) {
    for (int n_z = range[4]; n_z < range[5]; n_z++) {
      for (int n_y = range[2]; n_y < range[3]; n_y++) {
        for (int n_x = range[0]; n_x < range[1]; n_x++) {
          ops_args_set(n_x, n_y, n_z, 12, args,p_a);
          // call kernel function, passing in pointers to data
          kernel( (T0 *)p_a[0], (T1 *)p_a[1], (T2 *)p_a[2], (T3 *)p_a[3],
              (T4 *)p_a[4], (T5 *)p_a[5], (T6 *)p_a[6], (T7 *)p_a[7],
              (T8 *)p_a[8], (T9 *)p_a[9], (T10 *)p_a[10], (T11 *)p_a[11]);
        }
      }
    }
  }

  for (int i = 0; i < 12; i++) {
    if (args[i].argtype == OPS_ARG_DAT)
      free(p_a[i]);
  }
}



template < class T0, class T1, class T2, class T3, class T4, class T5,
           class T6, class T7, class T8 , class T9 , class T10, class T11,
           class T12>
void ops_par_loop(void (*kernel)( T0*, T1*, T2*, T3*, T4*, T5*, T6*, T7*,
                                  T8*, T9*, T10*, T11*, T12*),
  char const * name, int dim, int *range,
  ops_arg arg0,
  ops_arg arg1,
  ops_arg arg2,
  ops_arg arg3,
  ops_arg arg4,
  ops_arg arg5,
  ops_arg arg6,
  ops_arg arg7,
  ops_arg arg8,
  ops_arg arg9,
  ops_arg arg10,
  ops_arg arg11,
  ops_arg arg12) {

  char** p_a[13];
  ops_arg args[13] = {arg0, arg1, arg2, arg3, arg4,
                      arg5, arg6, arg7, arg8, arg9,
                      arg10, arg11, arg12};

  // consistency checks
  // ops_args_check(13,args,name);

  for (int i = 0; i < 13; i++) {
    if (args[i].argtype == OPS_ARG_DAT)
      p_a[i] = (char **)malloc(args[i].stencil->points * sizeof(char *));
    else if (args[i].argtype == OPS_ARG_GBL)
      p_a[i] = (char **)malloc(args[i].dim * sizeof(char *));
  }

  // loop over set elements

  if (dim == 1) {
    for (int n_x = range[0]; n_x < range[1]; n_x++) {
      ops_args_set(n_x, 13, args,p_a);
      // call kernel function, passing in pointers to data
      kernel( (T0 *)p_a[0], (T1 *)p_a[1], (T2 *)p_a[2], (T3 *)p_a[3],
              (T4 *)p_a[4], (T5 *)p_a[5], (T6 *)p_a[6], (T7 *)p_a[7],
              (T8 *)p_a[8], (T9 *)p_a[9], (T10 *)p_a[10], (T11 *)p_a[11],
              (T12 *)p_a[12]);
    }
  }
  else if (dim == 2) {
    for (int n_y = range[2]; n_y < range[3]; n_y++) {
      for (int n_x = range[0]; n_x < range[1]; n_x++) {
        ops_args_set(n_x, n_y, 13, args,p_a);
        // call kernel function, passing in pointers to data
        kernel( (T0 *)p_a[0], (T1 *)p_a[1], (T2 *)p_a[2], (T3 *)p_a[3],
              (T4 *)p_a[4], (T5 *)p_a[5], (T6 *)p_a[6], (T7 *)p_a[7],
              (T8 *)p_a[8], (T9 *)p_a[9], (T10 *)p_a[10], (T11 *)p_a[11],
              (T12 *)p_a[12]);
      }
    }
  }
  else if (dim == 3) {
    for (int n_z = range[4]; n_z < range[5]; n_z++) {
      for (int n_y = range[2]; n_y < range[3]; n_y++) {
        for (int n_x = range[0]; n_x < range[1]; n_x++) {
          ops_args_set(n_x, n_y, n_z, 13, args,p_a);
          // call kernel function, passing in pointers to data
          kernel( (T0 *)p_a[0], (T1 *)p_a[1], (T2 *)p_a[2], (T3 *)p_a[3],
              (T4 *)p_a[4], (T5 *)p_a[5], (T6 *)p_a[6], (T7 *)p_a[7],
              (T8 *)p_a[8], (T9 *)p_a[9], (T10 *)p_a[10], (T11 *)p_a[11],
              (T12 *)p_a[12]);
        }
      }
    }
  }

  for (int i = 0; i < 13; i++) {
    if (args[i].argtype == OPS_ARG_DAT)
      free(p_a[i]);
  }
}


template < class T0, class T1, class T2, class T3, class T4, class T5,
           class T6, class T7, class T8 , class T9 , class T10, class T11,
           class T12, class T13>
void ops_par_loop(void (*kernel)( T0*, T1*, T2*, T3*, T4*, T5*, T6*, T7*,
                                  T8*, T9*, T10*, T11*, T12*, T13*),
  char const * name, int dim, int *range,
  ops_arg arg0,
  ops_arg arg1,
  ops_arg arg2,
  ops_arg arg3,
  ops_arg arg4,
  ops_arg arg5,
  ops_arg arg6,
  ops_arg arg7,
  ops_arg arg8,
  ops_arg arg9,
  ops_arg arg10,
  ops_arg arg11,
  ops_arg arg12,
  ops_arg arg13) {

  char** p_a[14];
  ops_arg args[14] = {arg0, arg1, arg2, arg3, arg4,
                      arg5, arg6, arg7, arg8, arg9,
                      arg10, arg11, arg12, arg13};

  // consistency checks
  // ops_args_check(14,args,name);

  for (int i = 0; i < 14; i++) {
    if (args[i].argtype == OPS_ARG_DAT)
      p_a[i] = (char **)malloc(args[i].stencil->points * sizeof(char *));
    else if (args[i].argtype == OPS_ARG_GBL)
      p_a[i] = (char **)malloc(args[i].dim * sizeof(char *));
  }

  // loop over set elements

  if (dim == 1) {
    for (int n_x = range[0]; n_x < range[1]; n_x++) {
      ops_args_set(n_x, 14, args,p_a);
      // call kernel function, passing in pointers to data
      kernel( (T0 *)p_a[0], (T1 *)p_a[1], (T2 *)p_a[2], (T3 *)p_a[3],
              (T4 *)p_a[4], (T5 *)p_a[5], (T6 *)p_a[6], (T7 *)p_a[7],
              (T8 *)p_a[8], (T9 *)p_a[9], (T10 *)p_a[10], (T11 *)p_a[11],
              (T12 *)p_a[12], (T13 *)p_a[13]);
    }
  }
  else if (dim == 2) {
    for (int n_y = range[2]; n_y < range[3]; n_y++) {
      for (int n_x = range[0]; n_x < range[1]; n_x++) {
        ops_args_set(n_x, n_y, 14, args,p_a);
        // call kernel function, passing in pointers to data
        kernel( (T0 *)p_a[0], (T1 *)p_a[1], (T2 *)p_a[2], (T3 *)p_a[3],
              (T4 *)p_a[4], (T5 *)p_a[5], (T6 *)p_a[6], (T7 *)p_a[7],
              (T8 *)p_a[8], (T9 *)p_a[9], (T10 *)p_a[10], (T11 *)p_a[11],
              (T12 *)p_a[12], (T13 *)p_a[13]);
      }
    }
  }
  else if (dim == 3) {
    for (int n_z = range[4]; n_z < range[5]; n_z++) {
      for (int n_y = range[2]; n_y < range[3]; n_y++) {
        for (int n_x = range[0]; n_x < range[1]; n_x++) {
          ops_args_set(n_x, n_y, n_z, 14, args,p_a);
          // call kernel function, passing in pointers to data
          kernel( (T0 *)p_a[0], (T1 *)p_a[1], (T2 *)p_a[2], (T3 *)p_a[3],
              (T4 *)p_a[4], (T5 *)p_a[5], (T6 *)p_a[6], (T7 *)p_a[7],
              (T8 *)p_a[8], (T9 *)p_a[9], (T10 *)p_a[10], (T11 *)p_a[11],
              (T12 *)p_a[12], (T13 *)p_a[13]);
        }
      }
    }
  }

  for (int i = 0; i < 14; i++) {
    if (args[i].argtype == OPS_ARG_DAT)
      free(p_a[i]);
  }
}



template < class T0, class T1, class T2, class T3, class T4, class T5,
           class T6, class T7, class T8 , class T9 , class T10, class T11,
           class T12, class T13 , class T14>
void ops_par_loop(void (*kernel)( T0*, T1*, T2*, T3*, T4*, T5*, T6*, T7*,
                                  T8*, T9*, T10*, T11*, T12*, T13*, T14*),
  char const * name, int dim, int *range,
  ops_arg arg0,
  ops_arg arg1,
  ops_arg arg2,
  ops_arg arg3,
  ops_arg arg4,
  ops_arg arg5,
  ops_arg arg6,
  ops_arg arg7,
  ops_arg arg8,
  ops_arg arg9,
  ops_arg arg10,
  ops_arg arg11,
  ops_arg arg12,
  ops_arg arg13,
  ops_arg arg14) {

  char** p_a[15];
  ops_arg args[15] = {arg0, arg1, arg2, arg3, arg4,
                      arg5, arg6, arg7, arg8, arg9,
                      arg10, arg11, arg12, arg13, arg14};

  // consistency checks
  // ops_args_check(16,args,name);

  for (int i = 0; i < 15; i++) {
    if (args[i].argtype == OPS_ARG_DAT)
      p_a[i] = (char **)malloc(args[i].stencil->points * sizeof(char *));
    else if (args[i].argtype == OPS_ARG_GBL)
      p_a[i] = (char **)malloc(args[i].dim * sizeof(char *));
  }

  // loop over set elements

  if (dim == 1) {
    for (int n_x = range[0]; n_x < range[1]; n_x++) {
      ops_args_set(n_x, 15, args,p_a);
      // call kernel function, passing in pointers to data
      kernel( (T0 *)p_a[0], (T1 *)p_a[1], (T2 *)p_a[2], (T3 *)p_a[3],
              (T4 *)p_a[4], (T5 *)p_a[5], (T6 *)p_a[6], (T7 *)p_a[7],
              (T8 *)p_a[8], (T9 *)p_a[9], (T10 *)p_a[10], (T11 *)p_a[11],
              (T12 *)p_a[12], (T13 *)p_a[13], (T14 *)p_a[14]);
    }
  }
  else if (dim == 2) {
    for (int n_y = range[2]; n_y < range[3]; n_y++) {
      for (int n_x = range[0]; n_x < range[1]; n_x++) {
        ops_args_set(n_x, n_y, 15, args,p_a);
        // call kernel function, passing in pointers to data
        kernel( (T0 *)p_a[0], (T1 *)p_a[1], (T2 *)p_a[2], (T3 *)p_a[3],
              (T4 *)p_a[4], (T5 *)p_a[5], (T6 *)p_a[6], (T7 *)p_a[7],
              (T8 *)p_a[8], (T9 *)p_a[9], (T10 *)p_a[10], (T11 *)p_a[11],
              (T12 *)p_a[12], (T13 *)p_a[13], (T14 *)p_a[14]);
      }
    }
  }
  else if (dim == 3) {
    for (int n_z = range[4]; n_z < range[5]; n_z++) {
      for (int n_y = range[2]; n_y < range[3]; n_y++) {
        for (int n_x = range[0]; n_x < range[1]; n_x++) {
          ops_args_set(n_x, n_y, n_z, 15, args,p_a);
          // call kernel function, passing in pointers to data
          kernel( (T0 *)p_a[0], (T1 *)p_a[1], (T2 *)p_a[2], (T3 *)p_a[3],
              (T4 *)p_a[4], (T5 *)p_a[5], (T6 *)p_a[6], (T7 *)p_a[7],
              (T8 *)p_a[8], (T9 *)p_a[9], (T10 *)p_a[10], (T11 *)p_a[11],
              (T12 *)p_a[12], (T13 *)p_a[13], (T14 *)p_a[14]);
        }
      }
    }
  }

  for (int i = 0; i < 15; i++) {
    if (args[i].argtype == OPS_ARG_DAT)
      free(p_a[i]);
  }
}



template < class T0, class T1, class T2, class T3, class T4, class T5,
           class T6, class T7, class T8 , class T9 , class T10, class T11,
           class T12, class T13 , class T14 , class T15>
void ops_par_loop(void (*kernel)( T0*, T1*, T2*, T3*, T4*, T5*, T6*, T7*,
                                  T8*, T9*, T10*, T11*, T12*, T13*, T14*,
                                  T15*),
  char const * name, int dim, int *range,
  ops_arg arg0,
  ops_arg arg1,
  ops_arg arg2,
  ops_arg arg3,
  ops_arg arg4,
  ops_arg arg5,
  ops_arg arg6,
  ops_arg arg7,
  ops_arg arg8,
  ops_arg arg9,
  ops_arg arg10,
  ops_arg arg11,
  ops_arg arg12,
  ops_arg arg13,
  ops_arg arg14,
  ops_arg arg15) {

  char** p_a[16];
  ops_arg args[16] = {arg0, arg1, arg2, arg3, arg4,
                      arg5, arg6, arg7, arg8, arg9,
                      arg10, arg11, arg12, arg13, arg14,
                      arg15};

  // consistency checks
  // ops_args_check(16,args,name);

  for (int i = 0; i < 16; i++) {
    if (args[i].argtype == OPS_ARG_DAT)
      p_a[i] = (char **)malloc(args[i].stencil->points * sizeof(char *));
    else if (args[i].argtype == OPS_ARG_GBL)
      p_a[i] = (char **)malloc(args[i].dim * sizeof(char *));
  }

  // loop over set elements

  if (dim == 1) {
    for (int n_x = range[0]; n_x < range[1]; n_x++) {
      ops_args_set(n_x, 16, args,p_a);
      // call kernel function, passing in pointers to data
      kernel( (T0 *)p_a[0], (T1 *)p_a[1], (T2 *)p_a[2], (T3 *)p_a[3],
              (T4 *)p_a[4], (T5 *)p_a[5], (T6 *)p_a[6], (T7 *)p_a[7],
              (T8 *)p_a[8], (T9 *)p_a[9], (T10 *)p_a[10], (T11 *)p_a[11],
              (T12 *)p_a[12], (T13 *)p_a[13], (T14 *)p_a[14], (T15 *)p_a[15]);
    }
  }
  else if (dim == 2) {
    for (int n_y = range[2]; n_y < range[3]; n_y++) {
      for (int n_x = range[0]; n_x < range[1]; n_x++) {
        ops_args_set(n_x, n_y, 16, args,p_a);
        // call kernel function, passing in pointers to data
        kernel( (T0 *)p_a[0], (T1 *)p_a[1], (T2 *)p_a[2], (T3 *)p_a[3],
              (T4 *)p_a[4], (T5 *)p_a[5], (T6 *)p_a[6], (T7 *)p_a[7],
              (T8 *)p_a[8], (T9 *)p_a[9], (T10 *)p_a[10], (T11 *)p_a[11],
              (T12 *)p_a[12], (T13 *)p_a[13], (T14 *)p_a[14], (T15 *)p_a[15]);
      }
    }
  }
  else if (dim == 3) {
    for (int n_z = range[4]; n_z < range[5]; n_z++) {
      for (int n_y = range[2]; n_y < range[3]; n_y++) {
        for (int n_x = range[0]; n_x < range[1]; n_x++) {
          ops_args_set(n_x, n_y, n_z, 16, args,p_a);
          // call kernel function, passing in pointers to data
          kernel( (T0 *)p_a[0], (T1 *)p_a[1], (T2 *)p_a[2], (T3 *)p_a[3],
              (T4 *)p_a[4], (T5 *)p_a[5], (T6 *)p_a[6], (T7 *)p_a[7],
              (T8 *)p_a[8], (T9 *)p_a[9], (T10 *)p_a[10], (T11 *)p_a[11],
              (T12 *)p_a[12], (T13 *)p_a[13], (T14 *)p_a[14], (T15 *)p_a[15]);
        }
      }
    }
  }

  for (int i = 0; i < 16; i++) {
    if (args[i].argtype == OPS_ARG_DAT)
      free(p_a[i]);
  }
}
