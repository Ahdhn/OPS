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

/** @brief Allocates the data for each mesh chunk
 *  @author Wayne Gaudin, converted to OPS by Gihan Mudalige
 *  @details The data fields for the mesh chunk are allocated based on the mesh size
**/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

// OPS header file
#include "ops_seq.h"
#include "ops_mpi_seq.h"


#include "data.h"
#include "definitions.h"


void test_kernel(double *vertexdx) {

  printf("%lf ",vertexdx[OPS_ACC0(0,0)]);

}


void build_field()
{
  //initialize sizes using global values
  int x_cells = grid->x_cells;
  int y_cells = grid->y_cells;
  int x_min = field->x_min;
  int x_max = field->x_max;
  int y_min = field->y_min;
  int y_max = field->y_max;

  ops_printf("Global x_min = %d, y_min = %d\n",x_min,y_min);
  ops_printf("Global x_max = %d, y_max = %d\n",x_max,y_max);

  /**----------------------------OPS Declarations----------------------------**/

  int dims[2] = {x_cells+5, y_cells+5};  //cloverleaf 2D block dimensions: +5 because we allocate the largest ops_dat's size
  clover_grid = ops_decl_block(2, dims, "clover grid");

  //decompose the block
  ops_partition(2, dims, "2D_BLOCK_DECOMPSE");

  //
  //declare data on blocks
  //
  int d_p[2] = {-2,-2}; //max halo depths for the dat in the possitive direction
  int d_m[2] = {-2,-2}; //max halo depths for the dat in the negative direction
  int size[2] = {x_cells+5, y_cells+5}; //size of the dat -- should be identical to the block on which its define on
  double* temp = NULL;

  density0    = ops_decl_dat_mpi(clover_grid, 1, size, d_m, d_p, temp, "double", "density0");
  density1    = ops_decl_dat_mpi(clover_grid, 1, size, d_m, d_p, temp, "double", "density1");
  energy0     = ops_decl_dat_mpi(clover_grid, 1, size, d_m, d_p, temp, "double", "energy0");
  energy1     = ops_decl_dat_mpi(clover_grid, 1, size, d_m, d_p, temp, "double", "energy1");
  pressure    = ops_decl_dat_mpi(clover_grid, 1, size, d_m, d_p, temp, "double", "pressure");
  viscosity   = ops_decl_dat_mpi(clover_grid, 1, size, d_m, d_p, temp, "double", "viscosity");
  soundspeed  = ops_decl_dat_mpi(clover_grid, 1, size, d_m, d_p, temp, "double", "soundspeed");
  volume      = ops_decl_dat_mpi(clover_grid, 1, size, d_m, d_p, temp, "double", "volume");

  d_p[0]=-3;d_p[1]=-3;
  xvel0    = ops_decl_dat_mpi(clover_grid, 1, size, d_m, d_p, temp, "double", "xvel0");
  xvel1    = ops_decl_dat_mpi(clover_grid, 1, size, d_m, d_p, temp, "double", "xvel1");
  yvel0    = ops_decl_dat_mpi(clover_grid, 1, size, d_m, d_p, temp, "double", "yvel0");
  yvel1    = ops_decl_dat_mpi(clover_grid, 1, size, d_m, d_p, temp, "double", "yvel1");

  d_p[0]=-3;d_p[1]=-2;
  vol_flux_x  = ops_decl_dat_mpi(clover_grid, 1, size, d_m, d_p, temp, "double", "vol_flux_x");
  mass_flux_x = ops_decl_dat_mpi(clover_grid, 1, size, d_m, d_p, temp, "double", "mass_flux_x");
  xarea       = ops_decl_dat_mpi(clover_grid, 1, size, d_m, d_p, temp, "double", "xarea");

  d_p[0]=-2;d_p[1]=-3;
  vol_flux_y  = ops_decl_dat_mpi(clover_grid, 1, size, d_m, d_p, temp, "double", "vol_flux_y");
  mass_flux_y = ops_decl_dat_mpi(clover_grid, 1, size, d_m, d_p, temp, "double", "mass_flux_y");
  yarea       = ops_decl_dat_mpi(clover_grid, 1, size, d_m, d_p, temp, "double", "yarea");

  d_p[0]=-3;d_p[1]=-3;
  work_array1    = ops_decl_dat_mpi(clover_grid, 1, size, d_m, d_p, temp, "double", "work_array1");
  work_array2    = ops_decl_dat_mpi(clover_grid, 1, size, d_m, d_p, temp, "double", "work_array2");
  work_array3    = ops_decl_dat_mpi(clover_grid, 1, size, d_m, d_p, temp, "double", "work_array3");
  work_array4    = ops_decl_dat_mpi(clover_grid, 1, size, d_m, d_p, temp, "double", "work_array4");
  work_array5    = ops_decl_dat_mpi(clover_grid, 1, size, d_m, d_p, temp, "double", "work_array5");
  work_array6    = ops_decl_dat_mpi(clover_grid, 1, size, d_m, d_p, temp, "double", "work_array6");
  work_array7    = ops_decl_dat_mpi(clover_grid, 1, size, d_m, d_p, temp, "double", "work_array7");


  int size2[2] = {x_cells+5,1};
  d_m[0]=-2;d_m[1]=0;d_p[0]=-2;d_p[1]=0;
  cellx    = ops_decl_dat_mpi(clover_grid, 1, size2, d_m, d_p, temp, "double", "cellx");
  celldx   = ops_decl_dat_mpi(clover_grid, 1, size2, d_m, d_p, temp, "double", "celldx");

  int size3[2] = {1,y_cells+5};
  d_m[0]=0;d_m[1]=-2;d_p[0]=0;d_p[1]=-2;
  celly    = ops_decl_dat_mpi(clover_grid, 1, size3, d_m, d_p, temp, "double", "celly");
  celldy   = ops_decl_dat_mpi(clover_grid, 1, size3, d_m, d_p, temp, "double", "celldy");

  int size4[2] = {x_cells+5,1};
  d_m[0]=-2;d_m[1]=0;d_p[0]=-3;d_p[1]=0;
  vertexx  = ops_decl_dat_mpi(clover_grid, 1, size4, d_m, d_p, temp, "double", "vertexx");
  vertexdx = ops_decl_dat_mpi(clover_grid, 1, size4, d_m, d_p, temp, "double", "vertexdx");

  int size5[2] = {1,y_cells+5};
  d_m[0]=0;d_m[1]=-2;d_p[0]=0;d_p[1]=-3;
  vertexy  = ops_decl_dat_mpi(clover_grid, 1, size5, d_m, d_p, temp, "double", "vertexy");
  vertexdy = ops_decl_dat_mpi(clover_grid, 1, size5, d_m, d_p, temp, "double", "vertexdy");


  //contains x indicies from 0 to xmax+3 -- needed for initialization
  int* temp2 = NULL;
  d_m[0]=-2;d_m[1]=0;d_p[0]=-3;d_p[1]=0;
  xx  = ops_decl_dat_mpi(clover_grid, 1, size4, d_m, d_p, temp2, "int", "xx");
  for(int i=x_min-2; i<x_max+3; i++) ((int *)(xx->data))[i-d_m[0]] = i - x_min;

  d_m[0]=0;d_m[1]=-2;d_p[0]=0;d_p[1]=-3;
  yy  = ops_decl_dat_mpi(clover_grid, 1, size5, d_m, d_p, temp2, "int", "yy");
  for(int i=y_min-2; i<y_max+3; i++) ((int *)(yy->data))[i-d_m[1]] = i - y_min;



  /*int s2D_00[]         = {0,0};
  S2D_00         = ops_decl_stencil( 2, 1, s2D_00, "00");
  ops_arg arg_test = ops_arg_dat(density0, S2D_00, "double", OPS_READ);
  ops_exchange_halo(&arg_test,1);

  int rangexy[] = {x_min,x_max,y_min,y_max}; // range
  ops_par_loop_mpi(test_kernel, "test_kernel",  clover_grid, 2, rangexy,
    ops_arg_dat(density0, S2D_00, "double", OPS_READ));

  int stride2D_x[] = {1,0};
  S2D_00_STRID2D_X = ops_decl_strided_stencil( 2, 1, s2D_00, stride2D_x, "s2D_00_stride2D_x");
  ops_par_loop_mpi(test_kernel, "test_kernel",  clover_grid, 2, rangexy,
    ops_arg_dat(density0, S2D_00_STRID2D_X, "double", OPS_READ));


  int stride2D_y[] = {0,1};
  S2D_00_STRID2D_Y = ops_decl_strided_stencil( 2, 1, s2D_00, stride2D_y, "s2D_00_stride2D_y");
  ops_par_loop_mpi(test_kernel, "test_kernel",  clover_grid, 2, rangexy,
    ops_arg_dat(density0, S2D_00_STRID2D_Y, "double", OPS_READ));*/


  /*
  printf("Before on rank %d\n",ops_my_rank);
  for(int j = 0; j<OPS_sub_block_list[0]->sizes[1]+4; j++) {
    for(int i = 0; i<OPS_sub_block_list[0]->sizes[0]+4; i++) {
      printf("%3.2lf ", (double) density0->data[density0->size * (j* (OPS_sub_block_list[0]->sizes[0]+4) + i) ]);
    }
    printf("\n");
  }

  int size_1[2] = {(x_max+2)-(x_min-2),1};
  int offset_1[2] = {-2,0};
  int tail_1[2] = {-2,0};
  cellx       = ops_decl_dat_mpi(clover_grid, 1, size_1, offset_1, tail_1, temp, "double", "cellx");



  printf("After on rank %d\n",ops_my_rank);
  for(int j = 0; j<OPS_sub_block_list[0]->sizes[1]+4; j++) {
    for(int i = 0; i<OPS_sub_block_list[0]->sizes[0]+4; i++) {
      printf("%3.2lf ", (double) density0->data[density0->size * (j* (OPS_sub_block_list[0]->sizes[0]+4) + i) ]);
    }
    printf("\n");
  }

  density0    = ops_decl_dat_mpi(clover_grid, 1, size, offset, tail, temp, "double", "density0");
  density1    = ops_decl_dat(clover_grid, 1, size, offset, temp, "double", "density1");
  energy0     = ops_decl_dat(clover_grid, 1, size, offset, temp, "double", "energy0");
  energy1     = ops_decl_dat(clover_grid, 1, size, offset, temp, "double", "energy1");
  pressure    = ops_decl_dat(clover_grid, 1, size, offset, temp, "double", "pressure");
  viscosity   = ops_decl_dat(clover_grid, 1, size, offset, temp, "double", "viscosity");
  soundspeed  = ops_decl_dat(clover_grid, 1, size, offset, temp, "double", "soundspeed");
  volume      = ops_decl_dat(clover_grid, 1, size, offset, temp, "double", "volume");

  size[0] = (x_max+3)-(x_min-2); size[1] = (y_max+3)-(y_min-2);
  xvel0    = ops_decl_dat(clover_grid, 1, size, offset, temp, "double", "xvel0");
  xvel1    = ops_decl_dat(clover_grid, 1, size, offset, temp, "double", "xvel1");
  yvel0    = ops_decl_dat(clover_grid, 1, size, offset, temp, "double", "yvel0");
  yvel1    = ops_decl_dat(clover_grid, 1, size, offset, temp, "double", "yvel1");

  size[0] = (x_max+3)-(x_min-2); size[1] = (y_max+2)-(y_min-2);
  vol_flux_x  = ops_decl_dat(clover_grid, 1, size, offset, temp, "double", "vol_flux_x");
  mass_flux_x = ops_decl_dat(clover_grid, 1, size, offset, temp, "double", "mass_flux_x");
  xarea       = ops_decl_dat(clover_grid, 1, size, offset, temp, "double", "xarea");

  size[0] = (x_max+2)-(x_min-2); size[1] = (y_max+3)-(y_min-2);
  vol_flux_y  = ops_decl_dat(clover_grid, 1, size, offset, temp, "double", "vol_flux_y");
  mass_flux_y = ops_decl_dat(clover_grid, 1, size, offset, temp, "double", "mass_flux_y");
  yarea       = ops_decl_dat(clover_grid, 1, size, offset, temp, "double", "yarea");

  size[0] = (x_max+3)-(x_min-2); size[1] = (y_max+3)-(y_min-2);
  work_array1    = ops_decl_dat(clover_grid, 1, size, offset, temp, "double", "work_array1");
  work_array2    = ops_decl_dat(clover_grid, 1, size, offset, temp, "double", "work_array2");
  work_array3    = ops_decl_dat(clover_grid, 1, size, offset, temp, "double", "work_array3");
  work_array4    = ops_decl_dat(clover_grid, 1, size, offset, temp, "double", "work_array4");
  work_array5    = ops_decl_dat(clover_grid, 1, size, offset, temp, "double", "work_array5");
  work_array6    = ops_decl_dat(clover_grid, 1, size, offset, temp, "double", "work_array6");
  work_array7    = ops_decl_dat(clover_grid, 1, size, offset, temp, "double", "work_array7");

  int size2[2] = {(x_max+2)-(x_min-2),1};
  int size3[2] = {1, (y_max+2)-(y_min-2)};
  int size4[2] = {(x_max+3)-(x_min-2),1};
  int size5[2] = {1,(y_max+3)-(y_min-2)};
  int offsetx[2] = {-2,0};
  int offsety[2] = {0,-2};
  cellx    = ops_decl_dat(clover_grid, 1, size2, offsetx, temp, "double", "cellx");
  celly    = ops_decl_dat(clover_grid, 1, size3, offsety, temp, "double", "celly");
  vertexx  = ops_decl_dat(clover_grid, 1, size4, offsetx, temp, "double", "vertexx");
  vertexy  = ops_decl_dat(clover_grid, 1, size5, offsety, temp, "double", "vertexy");
  celldx   = ops_decl_dat(clover_grid, 1, size2, offsetx, temp, "double", "celldx");
  celldy   = ops_decl_dat(clover_grid, 1, size3, offsety, temp, "double", "celldy");
  vertexdx = ops_decl_dat(clover_grid, 1, size4, offsetx, temp, "double", "vertexdx");
  vertexdy = ops_decl_dat(clover_grid, 1, size5, offsety, temp, "double", "vertexdy");

  //contains x indicies from 0 to xmax+3 -- needed for initialization
  int* xindex = (int *)xmalloc(sizeof(int)*size4[0]);
  for(int i=x_min-2; i<x_max+3; i++) xindex[i-offsetx[0]] = i - x_min;
  xx  = ops_decl_dat(clover_grid, 1, size4, offsetx, xindex, "int", "xx");

  //contains y indicies from 0 to ymax+3 -- needed for initialization
  int* yindex = (int *)xmalloc(sizeof(int)*size5[1]);
  for(int i=y_min-2; i<y_max+3; i++) yindex[i-offsety[1]] = i - y_min;
  yy  = ops_decl_dat(clover_grid, 1, size5, offsety, yindex, "int", "yy");

  */

  //
  //Declare commonly used stencils
  //
  int s2D_00[]         = {0,0};
  int s2D_00_P10[]     = {0,0, 1,0};
  int s2D_00_0P1[]     = {0,0, 0,1};
  int s2D_00_M10[]     = {0,0, -1,0};
  int s2D_00_0M1[]     = {0,0, 0,-1};
  int s2D_00_P10_M10[] = {0,0, 1,0, -1,0};
  int s2D_00_0P1_0M1[] = {0,0, 0,1, 0,-1};
  int s2D_00_M10_M20[] = {0,0, -1,0, -2,0};
  int s2D_00_0M1_0M2[] = {0,0, 0,-1, 0,-2};
  int s2D_00_P20[]     = {0,0, 2,0};
  int s2D_00_0P2[]     = {0,0, 0,2};
  int s2D_00_M20[]     = {0,0, -2,0};
  int s2D_00_0M2[]     = {0,0, 0,-2};
  int s2D_00_P30[]     = {0,0, 3,0};
  int s2D_00_0P3[]     = {0,0, 0,3};
  int s2D_00_M30[]     = {0,0, -3,0};
  int s2D_00_0M3[]     = {0,0, 0,-3};
  int s2D_00_P40[]     = {0,0, 4,0};
  int s2D_00_0P4[]     = {0,0, 0,4};
  int s2D_00_M40[]     = {0,0, -4,0};
  int s2D_00_0M4[]     = {0,0, 0,-4};

  int s2D_00_P10_0P1_P1P1[]  = {0,0, 1,0, 0,1, 1,1};
  int s2D_00_M10_0M1_M1M1[]  = {0,0, -1,0, 0,-1, -1,-1};
  int s2D_00_P10_0M1_P1M1[] = {0,0, 1,0, 0,-1, 1,-1};
  int s2D_00_0P1_M10_M1P1[] = {0,0, 0,1, -1,0, -1,1};

  int s2D_10_M10_01_0M1[]  = {1,0, -1,0, 0,1, 0,-1};

  int s2D_00_P10_M10_M20[] = {0,0, 1,0, -1,0, -2,0};
  int s2D_00_0P1_0M1_0M2[] = {0,0, 0,1, 0,-1, 0,-2};
  int s2D_00_P10_P20_M10[] = {0,0, 1,0, 2,0, -1,0};
  int s2D_00_0P1_0P2_0M1[] = {0,0, 0,1, 0,2, 0,-1};


  int stride2D_x[] = {1,0};
  int stride2D_y[] = {0,1};

  int stride2D_null[] = {0,0};

  int xmax2D[] = {x_max+2,0};
  int ymax2D[] = {0,y_max+2};

  S2D_00         = ops_decl_stencil( 2, 1, s2D_00, "00");

  S2D_00_P10     = ops_decl_stencil( 2, 2, s2D_00_P10, "0,0:1,0");
  S2D_00_0P1     = ops_decl_stencil( 2, 2, s2D_00_0P1, "0,0:0,1");
  S2D_00_M10     = ops_decl_stencil( 2, 2, s2D_00_M10, "0,0:-1,0");
  S2D_00_0M1     = ops_decl_stencil( 2, 2, s2D_00_0M1, "0,0:0,-1");

  S2D_00_P10_M10 = ops_decl_stencil( 2, 3, s2D_00_P10_M10, "0,0:1,0:1,0");
  S2D_00_0P1_0M1 = ops_decl_stencil( 2, 3, s2D_00_0P1_0M1, "0,0:0,1:0,-1");

  S2D_00_M10_M20 = ops_decl_stencil( 2, 3, s2D_00_M10_M20, "0,0:-1,0:-2,0");
  S2D_00_0M1_0M2 = ops_decl_stencil( 2, 3, s2D_00_0M1_0M2, "0,0:0,-1:0,-2");

  S2D_00_P20     = ops_decl_stencil( 2, 2, s2D_00_P20, "0,0:2,0");
  S2D_00_0P2     = ops_decl_stencil( 2, 2, s2D_00_0P2, "0,0:0,2");

  S2D_00_M20     = ops_decl_stencil( 2, 2, s2D_00_M20, "0,0:-2,0");
  S2D_00_0M2     = ops_decl_stencil( 2, 2, s2D_00_0M2, "0,0:0,-2");

  S2D_00_P30     = ops_decl_stencil( 2, 2, s2D_00_P30, "0,0:3,0");
  S2D_00_0P3     = ops_decl_stencil( 2, 2, s2D_00_0P3, "0,0:0,3");

  S2D_00_M30     = ops_decl_stencil( 2, 2, s2D_00_M30, "0,0:-3,0");
  S2D_00_0M3     = ops_decl_stencil( 2, 2, s2D_00_0M3, "0,0:0,-3");

  S2D_00_P40     = ops_decl_stencil( 2, 2, s2D_00_P40, "0,0:4,0");
  S2D_00_0P4     = ops_decl_stencil( 2, 2, s2D_00_0P4, "0,0,0,-4");

  S2D_00_M40     = ops_decl_stencil( 2, 2, s2D_00_M40, "0,0:-4,0");
  S2D_00_0M4     = ops_decl_stencil( 2, 2, s2D_00_0M4, "0,0:0,-4");

  S2D_00_P10_0P1_P1P1 = ops_decl_stencil( 2, 4, s2D_00_P10_0P1_P1P1, "0,0:1,0:0,1:1,1");
  S2D_00_M10_0M1_M1M1 = ops_decl_stencil( 2, 4, s2D_00_M10_0M1_M1M1, "0,0:-1,0:0,-1:-1,-1");
  S2D_00_P10_0M1_P1M1= ops_decl_stencil( 2, 4, s2D_00_P10_0M1_P1M1, "0,0:1,0:0,-1:1,-1");
  S2D_00_0P1_M10_M1P1= ops_decl_stencil( 2, 4, s2D_00_0P1_M10_M1P1, "0,0:0,1:-1,0:-1,1");

  S2D_10_M10_01_0M1 = ops_decl_stencil( 2, 4, s2D_10_M10_01_0M1, "1,0:-1,0:0,1:0,-1");

  S2D_00_P10_M10_M20 = ops_decl_stencil( 2, 4, s2D_00_P10_M10_M20, "0,0:1,0:-1,0:-2,0");
  S2D_00_P10_M10_M20_STRID2D_X = ops_decl_strided_stencil( 2, 4, s2D_00_P10_M10_M20, stride2D_x, "self_stride2D_x");

  S2D_00_0P1_0M1_0M2 = ops_decl_stencil( 2, 4, s2D_00_0P1_0M1_0M2, "0,0:0,1:0-1:0,-2");
  S2D_00_0P1_0M1_0M2_STRID2D_Y = ops_decl_strided_stencil( 2, 4, s2D_00_0P1_0M1_0M2, stride2D_y, "self_stride2D_y");

  S2D_00_P10_P20_M10 = ops_decl_stencil( 2, 4, s2D_00_P10_P20_M10, "0,0:1,0:2,0:-1,0}");
  S2D_00_0P1_0P2_0M1 = ops_decl_stencil( 2, 4, s2D_00_0P1_0P2_0M1, "0,0,:0,1,:0,2,:0,-1");


  S2D_00_STRID2D_X = ops_decl_strided_stencil( 2, 1, s2D_00, stride2D_x, "s2D_00_stride2D_x");
  S2D_00_STRID2D_Y = ops_decl_strided_stencil( 2, 1, s2D_00, stride2D_y, "s2D_00_stride2D_y");

  s2D_00_P10_STRID2D_X = ops_decl_strided_stencil( 2, 2, s2D_00_P10, stride2D_x, "s2D_00_P10_stride2D_x");
  S2D_00_0P1_STRID2D_Y = ops_decl_strided_stencil( 2, 2, s2D_00_0P1, stride2D_y, "s2D_00_0P1_stride2D_y");

  S2D_00_M10_STRID2D_X = ops_decl_strided_stencil( 2, 2, s2D_00_M10, stride2D_x, "s2D_00_M10_stride2D_x");
  S2D_00_0M1_STRID2D_Y = ops_decl_strided_stencil( 2, 2, s2D_00_0M1, stride2D_y, "s2D_00_0M1_stride2D_y");


  //print ops blocks and dats details
  ops_diagnostic_output();

  /*int rangexy[] = {x_min,x_max,y_min,y_max}; // range
  ops_par_loop_mpi(test_kernel, "test_kernel",  clover_grid, 2, rangexy,
    ops_arg_dat(density0, S2D_00, "double", OPS_READ));

  ops_exit();//exit for now
  exit(0);*/

}
