#define OPS_3D
#include "ops_seq.h"

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "write_kernel.h"

int main(int argc, char **argv) {

  //*******************************************************************
  // INITIALISE OPS
  //---------------------------------------
  ops_init(argc, argv, 5);
  ops_printf("Initialize OPS\n\n");
  //*******************************************************************

  ops_block grid0 = ops_decl_block(3, "grid0");

  int d_p[3] = {1, 1,
                0}; // max halo depths for the dat in the positive direction
  int d_m[3] = {-1, -1,
                0}; // max halo depths for the dat in the negative direction
  int size[3] = {4, 5, 1}; // size of the dat
  int base[3] = {0, 0, 0};

  double *temp = NULL;

  ops_dat single =
      ops_decl_dat(grid0, 1, size, base, d_m, d_p, temp, "double", "single");
  ops_dat multi =
      ops_decl_dat(grid0, 2, size, base, d_m, d_p, temp, "double", "multi");

  int range_full[6];
  range_full[0] = 0;
  range_full[1] = 4;
  range_full[2] = 0;
  range_full[3] = 5;
  range_full[4] = 0;
  range_full[5] = 1;

  int s3D_000[] = {0, 0, 0};
  ops_stencil S3D_000 = ops_decl_stencil(3, 1, s3D_000, "0,0,0");

  ops_partition("empty_string_that_does_nothing_yet");
  ops_diagnostic_output();

  ops_par_loop(write_kernel, "write_kernel", grid0, 3, range_full,
               ops_arg_dat(multi, 2, S3D_000, "double", OPS_WRITE),
               ops_arg_dat(single, 1, S3D_000, "double", OPS_WRITE),
               ops_arg_idx());

  ops_fetch_block_hdf5_file(grid0, "write_data.h5");

  ops_fetch_dat_hdf5_file(multi, "write_data.h5");
  ops_fetch_dat_hdf5_file(single, "write_data.h5");

  //*******************************************************************
  // EXIT OPS AND PRINT TIMING INFO
  //---------------------------------------
  ops_timing_output(stdout);
  ops_printf("\nSucessful Exit from OPS!\n");
  ops_exit();
  // return 0;
  //*******************************************************************
}
