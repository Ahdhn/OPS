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

/** @file
  * @brief OPS API calls and wrapper routines for Tridiagonal solvers
  * @author Gihan Mudalige, Istvan Reguly
  * @details Implementations of the OPS API calls, wrapper routines and other
  * functions for interfacing with external Tridiagonal libraries
  */

#include <trid_common.h>
#include <trid_mpi_cuda.hpp>
#include <cuda.h>

#include <ops_cuda_rt_support.h>
#include <ops_mpi_core.h>
#include <ops_exceptions.h>
#include <ops_tridiag.h>
#include <ops_lib_core.h>

#define TRID_MPI_CUDA_BATCH_SIZE 65536
#define TRID_MPI_CUDA_STRATEGY MpiSolverParams::ALLGATHER

void ops_initTridMultiDimBatchSolve(int ndim, int *dims) {
  // dummy routine for non-GPU backends
}

void rms(char *name, FP *array, int nx_pad, int nx, int ny, int nz) {
  // Sum the square of values in app.h_u
  double sum = 0.0;
  for (int k = 0; k < nz; k++) {
    for (int j = 0; j < ny; j++) {
      for (int i = 0; i < nx; i++) {
        int ind = k * nx_pad * ny + j * nx_pad + i;
        // sum += array[ind]*array[ind];
        sum += array[ind];
      }
    }
  }
  double global_sum = 0.0;
  MPI_Allreduce(&sum, &global_sum,1, MPI_DOUBLE,MPI_SUM, MPI_COMM_WORLD);
  ops_printf("intermediate %s sum = %lg\n", name, global_sum);

}

void ops_tridMultiDimBatch(
    int ndim,     // number of dimensions, ndim <= MAXDIM = 8
    int solvedim, // user chosen dimension to perform solve
    int *dims,    // array containing the sizes of each ndim dimensions
    ops_dat a, ops_dat b, ops_dat c, // left hand side coefficients of a
    // multidimensional problem. An array containing
    // A matrices of individual problems
    ops_dat d, // right hand side coefficients of a multidimensional problem. An
               // array containing d column vectors of individual problems
    ops_dat u//,
//    int *opts // indicates different algorithms to use -- not used for CPU
              // backends
    ) {

  // check if sizes match
  for (int i = 0; i < 3; i++) {
    if (a->size[i] != b->size[i] || b->size[i] != c->size[i] ||
        c->size[i] != d->size[i] || d->size[i] != u->size[i]) {
      throw OPSException(OPS_RUNTIME_ERROR, "Tridsolver error: the a,b,c,d datasets all need to be the same size");
    }
  }

  int dims_calc[OPS_MAX_DIM];
  int pads_m[OPS_MAX_DIM];
  int pads_p[OPS_MAX_DIM];
  sub_dat *sd_a = OPS_sub_dat_list[a->index];

  for(int i = 0; i < ndim; i++) {
    pads_m[i] = -1 * (a->d_m[i] + sd_a->d_im[i]);
    pads_p[i] = a->d_p[i] + sd_a->d_ip[i];
    dims_calc[i] = a->size[i] - pads_m[i] - pads_p[i];
  }

  int offset = pads_m[2] * a->size[1] * a->size[0] // z padding
               + pads_m[1] * a->size[0] // y padding
               + pads_m[0]; // x padding

  // compute tridiagonal system sizes
  ops_block block = a->block;
  sub_block *sb = OPS_sub_block_list[block->index];

  MpiSolverParams *trid_mpi_params =
    new MpiSolverParams(sb->comm, sb->ndim, sb->pdims, TRID_MPI_CUDA_BATCH_SIZE,
                        TRID_MPI_CUDA_STRATEGY);

  int device = OPS_DEVICE;
  int s3D_000[] = {0, 0, 0};
  ops_stencil S3D_000 = ops_decl_stencil(3, 1, s3D_000, "000");

  // Get raw pointer access to data held by OPS
  // Points to element 0, skipping MPI halo
  /*ops_put_data(a);
  ops_put_data(b);
  ops_put_data(c);
  ops_put_data(d);
  ops_put_data(u);

  tridDmtsvStridedBatchMPI(*trid_mpi_params, (const double*)&a->data_d[offset],
                           (const double*)&b->data_d[offset], (const double*)&c->data_d[offset],
                           (double*)&d->data_d[offset], (double*)&u->data_d[offset],
                           ndim, solvedim, dims_calc, a->size, offset);

  ops_set_dirtybit_device_dat(d);*/
  const double *a_ptr = (double *)ops_dat_get_raw_pointer(a, 0, S3D_000, &device);
  const double *b_ptr = (double *)ops_dat_get_raw_pointer(b, 0, S3D_000, &device);
  const double *c_ptr = (double *)ops_dat_get_raw_pointer(c, 0, S3D_000, &device);
  double *d_ptr = (double *)ops_dat_get_raw_pointer(d, 0, S3D_000, &device);
  double *u_ptr = (double *)ops_dat_get_raw_pointer(u, 0, S3D_000, &device);

  tridDmtsvStridedBatchMPI(*trid_mpi_params, a_ptr, b_ptr, c_ptr, d_ptr, u_ptr,
                           ndim, solvedim, dims_calc, a->size/*, offset*/);

  // Release pointer access back to OPS
  ops_dat_release_raw_data(u, 0, OPS_READ);
  ops_dat_release_raw_data(d, 0, OPS_RW);
  ops_dat_release_raw_data(c, 0, OPS_READ);
  ops_dat_release_raw_data(b, 0, OPS_READ);
  ops_dat_release_raw_data(a, 0, OPS_READ);

  delete trid_mpi_params;

  /* Right now, we are simply using the same memory allocated by OPS
  as can be seen by the use of a->data, b->data, c->data etc.

  These data is currently not padded to be 32 or 64 bit aligned
  in the x-lines and so is inefficient.

  In the ADI example currently the mesh size is 256^3 and so we are
  32/64 bit aligned, thus we do not see any performance deficiencies
  but other sizes will show this issue

  As such we will need to think on how to pad arrays.
  The problem is that on apps like Cloverleaf we see poorer performance
  due to extra x dim padding.
  */

  /*
  For MPI padding will be more important as the partition allocated per MPI proc
  will definitely not be a multiple of 32 or 64 in the x dimension

  Perhaps we make use of a setup phase to add padding to the ops data arrays
  and then use them in the tridiagonal solvers. But now the problem is
  that the original OPS lib will not be able to use these padded arrays
  and produce correct results -- need to think how to solve this
  */
}

void ops_tridMultiDimBatch_Inc(
    int ndim,     // number of dimensions, ndim <= MAXDIM = 8
    int solvedim, // user chosen dimension to perform solve
    int *dims,    // array containing the sizes of each ndim dimensions
    ops_dat a, ops_dat b, ops_dat c, // left hand side coefficients of a
    // multidimensional problem. An array containing
    // A matrices of individual problems
    ops_dat d, // right hand side coefficients of a multidimensional problem. An
               // array containing d column vectors of individual problems
    ops_dat u//,
//    int *opts // indicates different algorithms to use -- not used for CPU
//              // backends
    ) {

  // check if sizes match
  for (int i = 0; i < 3; i++) {
    if (a->size[i] != b->size[i] || b->size[i] != c->size[i] ||
        c->size[i] != d->size[i] || d->size[i] != u->size[i]) {
      throw OPSException(OPS_RUNTIME_ERROR, "Tridsolver error: the a,b,c,d datasets all need to be the same size");
    }
  }

  int dims_calc[OPS_MAX_DIM];
  int pads_m[OPS_MAX_DIM];
  int pads_p[OPS_MAX_DIM];
  sub_dat *sd_a = OPS_sub_dat_list[a->index];

  for(int i = 0; i < ndim; i++) {
    pads_m[i] = -1 * (a->d_m[i] + sd_a->d_im[i]);
    pads_p[i] = a->d_p[i] + sd_a->d_ip[i];
    dims_calc[i] = a->size[i] - pads_m[i] - pads_p[i];
  }

  int offset = pads_m[2] * a->size[1] * a->size[0] // z padding
               + pads_m[1] * a->size[0] // y padding
               + pads_m[0]; // x padding

  // compute tridiagonal system sizes
  ops_block block = a->block;
  sub_block *sb = OPS_sub_block_list[block->index];

  MpiSolverParams *trid_mpi_params =
    new MpiSolverParams(sb->comm, sb->ndim, sb->pdims, TRID_MPI_CUDA_BATCH_SIZE,
                        TRID_MPI_CUDA_STRATEGY);

  /*ops_put_data(a);
  ops_put_data(b);
  ops_put_data(c);
  ops_put_data(d);
  ops_put_data(u);

  tridDmtsvStridedBatchIncMPI(*trid_mpi_params, (const double*)&a->data_d[offset],
                              (const double*)&b->data_d[offset], (const double*)&c->data_d[offset],
                              (double*)&d->data_d[offset], (double*)&u->data_d[offset],
                              ndim, solvedim, dims_calc, a->size, offset);

  ops_set_dirtybit_device_dat(u);*/

  int device = OPS_DEVICE;
  int s3D_000[] = {0, 0, 0};
  ops_stencil S3D_000 = ops_decl_stencil(3, 1, s3D_000, "000");

  const double *a_ptr = (double *)ops_dat_get_raw_pointer(a, 0, S3D_000, &device);
  const double *b_ptr = (double *)ops_dat_get_raw_pointer(b, 0, S3D_000, &device);
  const double *c_ptr = (double *)ops_dat_get_raw_pointer(c, 0, S3D_000, &device);
  double *d_ptr = (double *)ops_dat_get_raw_pointer(d, 0, S3D_000, &device);
  double *u_ptr = (double *)ops_dat_get_raw_pointer(u, 0, S3D_000, &device);

  // For now do not consider adding padding
  tridDmtsvStridedBatchIncMPI(*trid_mpi_params, a_ptr, b_ptr, c_ptr, d_ptr, u_ptr,
                              ndim, solvedim, dims_calc, a->size/*, offset*/);

  ops_dat_release_raw_data(u, 0, OPS_RW);
  ops_dat_release_raw_data(d, 0, OPS_READ);
  ops_dat_release_raw_data(c, 0, OPS_READ);
  ops_dat_release_raw_data(b, 0, OPS_READ);
  ops_dat_release_raw_data(a, 0, OPS_READ);

  delete trid_mpi_params;
}

void ops_exitTridMultiDimBatchSolve() {
  // free memory allocated during tridiagonal solve e.g. mpi buffers
}
