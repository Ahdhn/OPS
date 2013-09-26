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

/** @brief Top level initialisation routine
 *  @author Wayne Gaudin
 *  @details Checks for the user input and either invokes the input reader or
 *  switches to the internal test problem. It processes the input and strips
 *  comments before writing a final input file.
 *  It then calls the start routine.
**/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

// OPS header file
#include "ops_seq.h"

#include "data.h"
#include "definitions.h"

void update_halo(int* fields, int depth);
void advec_cell(int sweep_number, int direction);
void advec_mom(int which_vel, int sweep_number, int dir);

void advection(int step)
{
  int sweep_number, direction;
  int xvel,yvel;

  sweep_number = 1;
  if(advect_x == TRUE) direction = g_xdir;
  else direction = g_ydir;

  xvel = g_xdir;
  yvel = g_ydir;

  fields[FIELD_DENSITY0]  = 0;
  fields[FIELD_ENERGY0]   = 0;
  fields[FIELD_DENSITY1]  = 1;
  fields[FIELD_ENERGY1]   = 1;
  fields[FIELD_PRESSURE]  = 0;
  fields[FIELD_VISCOSITY] = 0;
  fields[FIELD_SOUNDSPEED] = 0;
  fields[FIELD_XVEL0]     = 0;
  fields[FIELD_YVEL0]     = 0;
  fields[FIELD_XVEL1]     = 0;
  fields[FIELD_YVEL1]     = 0;
  fields[FIELD_VOL_FLUX_X] = 1;
  fields[FIELD_VOL_FLUX_Y] = 1;
  fields[FIELD_MASS_FLUX_X] = 0;
  fields[FIELD_MASS_FLUX_Y] = 0;
  update_halo(fields,2);

  advec_cell(sweep_number, direction);



  fields[FIELD_DENSITY0]  = 0;
  fields[FIELD_ENERGY0]   = 0;
  fields[FIELD_DENSITY1]  = 1;
  fields[FIELD_ENERGY1]   = 1;
  fields[FIELD_PRESSURE]  = 0;
  fields[FIELD_VISCOSITY] = 0;
  fields[FIELD_SOUNDSPEED] = 0;
  fields[FIELD_XVEL0]     = 0;
  fields[FIELD_YVEL0]     = 0;
  fields[FIELD_XVEL1]     = 1;
  fields[FIELD_YVEL1]     = 1;
  fields[FIELD_VOL_FLUX_X] = 0;
  fields[FIELD_VOL_FLUX_Y] = 0;
  fields[FIELD_MASS_FLUX_X] = 1;
  fields[FIELD_MASS_FLUX_Y] = 1;
  update_halo(fields,2);

  if(step == 2)
  {
    ops_print_dat_to_txtfile_core(work_array7, "cloverdats.dat");
  }

  //printf("advect_x: %d\n",advect_x);
  advec_mom(xvel, sweep_number, direction);

  if(step == 2)
  {
    ops_print_dat_to_txtfile_core(work_array7, "cloverdats.dat");
    exit(-2);
  }
  //printf("advect_x: %d\n",advect_x);
  advec_mom(xvel, sweep_number, direction);

  sweep_number = 2;
  if(advect_x == TRUE) direction = g_ydir;
  else direction= g_xdir;

  advec_cell(sweep_number,direction);

  fields[FIELD_DENSITY0]  = 0;
  fields[FIELD_ENERGY0]   = 0;
  fields[FIELD_PRESSURE]  = 0;
  fields[FIELD_VISCOSITY] = 0;
  fields[FIELD_DENSITY1]  = 1;
  fields[FIELD_ENERGY1]   = 1;
  fields[FIELD_SOUNDSPEED] = 0;
  fields[FIELD_XVEL0]     = 0;
  fields[FIELD_YVEL0]     = 0;
  fields[FIELD_XVEL1]     = 1;
  fields[FIELD_YVEL1]     = 1;
  fields[FIELD_VOL_FLUX_X] = 0;
  fields[FIELD_VOL_FLUX_Y] = 0;
  fields[FIELD_MASS_FLUX_X] = 1;
  fields[FIELD_MASS_FLUX_Y] = 1;
  update_halo(fields,2);

  //printf("advect_x: %d\n",advect_x);
  advec_mom(xvel, sweep_number, direction);
  //printf("advect_x: %d\n",advect_x);
  advec_mom(xvel, sweep_number, direction);

}
