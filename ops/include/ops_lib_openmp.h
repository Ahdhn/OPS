#ifndef LIB_OPENMP_H
#define LIB_OPENMP_H
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

#ifndef OPS_ACC_MACROS
#define OPS_ACC0(x,y) (x+xdim0*(y))
#define OPS_ACC1(x,y) (x+xdim1*(y))
#define OPS_ACC2(x,y) (x+xdim2*(y))
#define OPS_ACC3(x,y) (x+xdim3*(y))
#define OPS_ACC4(x,y) (x+xdim4*(y))
#define OPS_ACC5(x,y) (x+xdim5*(y))
#define OPS_ACC6(x,y) (x+xdim6*(y))
#define OPS_ACC7(x,y) (x+xdim7*(y))
#define OPS_ACC8(x,y) (x+xdim8*(y))
#define OPS_ACC9(x,y) (x+xdim9*(y))
#define OPS_ACC10(x,y) (x+xdim10*(y))
#define OPS_ACC11(x,y) (x+xdim11*(y))
#define OPS_ACC12(x,y) (x+xdim12*(y))
#define OPS_ACC13(x,y) (x+xdim13*(y))
#define OPS_ACC14(x,y) (x+xdim14*(y))
#define OPS_ACC15(x,y) (x+xdim15*(y))
#endif

extern int xdim0;
extern int xdim1;
extern int xdim2;
extern int xdim3;
extern int xdim4;
extern int xdim5;
extern int xdim6;
extern int xdim7;
extern int xdim8;
extern int xdim9;
extern int xdim10;
extern int xdim11;
extern int xdim12;
extern int xdim13;
extern int xdim14;
extern int xdim15;


inline int ops_offs_set(int n_x,
                        int n_y, ops_arg arg){
        return
        arg.dat->block_size[0] * //multiply by the number of
        (n_y - arg.dat->offset[1])  // calculate the offset from index 0 for y dim
        +
        (n_x - arg.dat->offset[0]); //calculate the offset from index 0 for x dim
}

#endif
