/* Crown Copyright 2012 AWE.

 This file is part of CloverLeaf.

 CloverLeaf is free software: you can redistribute it and/or modify it under
 the terms of the GNU General Public License as published by the
 Free Software Foundation, either version 3 of the License, or (at your option)
 any later version.

 CloverLeaf is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 details.

 You should have received a copy of the GNU General Public License along with
 CloverLeaf. If not, see http://www.gnu.org/licenses/. */

/** @brief Reset fields
 *  @author Wayne Gaudin, converted to OPS by Gihan Mudalige
 *  @details Invokes the user specified field reset kernels
**/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

// OPS header file
#include "ops_seq_opt.h"
#include "ops_seq_macro.h"

#include "data.h"
#include "definitions.h"

#include "reset_field_kernel.h"

void reset_field_kernel1_macro( double *density0, double *density1,
                        double *energy0, double *energy1) {

  density0[OPS_ACC0(0,0)]  = density1[OPS_ACC1(0,0)] ;
  energy0[OPS_ACC2(0,0)]  = energy1[OPS_ACC3(0,0)] ;

}

void reset_field_kernel2_macro( double *xvel0, double *xvel1,
                        double *yvel0, double *yvel1) {

  xvel0[OPS_ACC0(0,0)]  = xvel1[OPS_ACC1(0,0)] ;
  yvel0[OPS_ACC2(0,0)]  = yvel1[OPS_ACC3(0,0)] ;

}

void reset_field()
{
  error_condition = 0; // Not used yet due to issue with OpenA reduction

  //initialize sizes using global values
  int x_cells = grid->x_cells;
  int y_cells = grid->y_cells;
  int x_min = field->x_min;
  int x_max = field->x_max;
  int y_min = field->y_min;
  int y_max = field->y_max;

  int rangexy_inner[] = {x_min,x_max,y_min,y_max}; // inner range without border

  ops_par_loop_opt(reset_field_kernel1_macro, "reset_field_kernel1_macro", 2, rangexy_inner,
    ops_arg_dat(density0, S2D_00, "double", OPS_READ),
    ops_arg_dat(density1, S2D_00, "double", OPS_READ),
    ops_arg_dat(energy0, S2D_00, "double", OPS_READ),
    ops_arg_dat(energy1, S2D_00, "double", OPS_READ));

  int rangexy_inner_plus1xy[] = {x_min,x_max+1,y_min,y_max+1}; // inner range without border

  ops_par_loop_opt(reset_field_kernel2_macro, "reset_field_kernel2_macro", 2, rangexy_inner_plus1xy,
    ops_arg_dat(xvel0, S2D_00, "double", OPS_READ),
    ops_arg_dat(xvel1, S2D_00, "double", OPS_READ),
    ops_arg_dat(yvel0, S2D_00, "double", OPS_READ),
    ops_arg_dat(yvel1, S2D_00, "double", OPS_READ));

}
