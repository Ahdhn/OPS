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

/** @brief ops mpi run-time support routines
  * @author Gihan Mudalige, Istvan Reguly
  * @details Implements the runtime support routines for the OPS mpi backend
  */

#include <ops_lib_core.h>

#include <mpi.h>
#include <ops_mpi_core.h>

// Timing
double t1,t2,c1,c2;

int ops_is_root()
{
  int my_rank;
  MPI_Comm_rank(MPI_COMM_WORLD,&my_rank);
  return (my_rank==MPI_ROOT);
}

void ops_exchange_halo(ops_arg* arg, int d/*depth*/)
{
  ops_dat dat = arg->dat;

  if(arg->opt == 1 && dat->dirtybit == 1) { //need to check OPS accs
  //if( dat->dirtybit == 1) { //need to check OPS accs
  // ops_printf("exchanging %s\n",arg->dat->name);

    sub_block_list sb = OPS_sub_block_list[dat->block->index];
    sub_dat_list sd = OPS_sub_dat_list[dat->index];

    int i1,i2,i3,i4; //indicies for halo and boundary of the dat
    int* d_m = sd->d_m;
    int* d_p = sd->d_p;
    int* prod = sd->prod;
    int size = dat->size;
    MPI_Status status;

    for(int n=0;n<sb->ndim;n++){
      if(dat->block_size[n] > 1 && d > 0) {

        i1 = (-d_m[n] - d) * prod[n-1];
        i2 = (-d_m[n]    ) * prod[n-1];
        i3 = (prod[n]/prod[n-1] - (-d_p[n]) - d) * prod[n-1];
        i4 = (prod[n]/prod[n-1] - (-d_p[n])    ) * prod[n-1];

        //send in positive direction, receive from negative direction
        //printf("Exchaning 1 From:%d To: %d\n", i3, i1);
        MPI_Sendrecv(&dat->data[i3*size],1,sd->mpidat[MAX_DEPTH*n+d],sb->id_p[n],0,
                     &dat->data[i1*size],1,sd->mpidat[MAX_DEPTH*n+d],sb->id_m[n],0,
                     OPS_CART_COMM, &status);

        //send in negative direction, receive from positive direction
        //printf("Exchaning 2 From:%d To: %d\n", i2, i4);
        MPI_Sendrecv(&dat->data[i2*size],1,sd->mpidat[MAX_DEPTH*n+d],sb->id_m[n],1,
                     &dat->data[i4*size],1,sd->mpidat[MAX_DEPTH*n+d],sb->id_p[n],1,
                     OPS_CART_COMM, &status);
      }
    }

    dat->dirtybit = 0;
  }

}

void ops_exchange_halo2(ops_arg* arg, int* d_pos, int* d_neg /*depth*/)
{
  ops_dat dat = arg->dat;

  if(arg->opt == 1 && dat->dirtybit == 1) { //need to check OPS accs
  //if( dat->dirtybit == 1) { //need to check OPS accs
  // ops_printf("exchanging %s\n",arg->dat->name);

    sub_block_list sb = OPS_sub_block_list[dat->block->index];
    sub_dat_list sd = OPS_sub_dat_list[dat->index];

    int i1,i2,i3,i4; //indicies for halo and boundary of the dat
    int* d_m = sd->d_m;
    int* d_p = sd->d_p;
    int* prod = sd->prod;
    int size = dat->size;
    MPI_Status status;

    for(int n=0;n<sb->ndim;n++){
      int d_min = abs(d_neg[n]);
      //d_pos[n] = 2; //hard coded for now .. change for dynamic halo depth
      if(dat->block_size[n] > 1 ) {//&& (d_pos[n] > 0 || d_min > 0) ) {

        //send in positive direction, receive from negative direction
        //printf("Exchaning 1 From:%d To: %d\n", i3, i1);
        int actual_depth = 0;
        for (int d = 0; d < d_min; d++) if(dat->dirty_dir[2*MAX_DEPTH*n + d] == 1) actual_depth = d;

        i1 = (-d_m[n] - actual_depth) * prod[n-1];
        i3 = (prod[n]/prod[n-1] - (-d_p[n]) - actual_depth) * prod[n-1];

        if(actual_depth > 0)
        MPI_Sendrecv(&dat->data[i3*size],1,sd->mpidat[MAX_DEPTH*n+actual_depth],sb->id_p[n],0,
                     &dat->data[i1*size],1,sd->mpidat[MAX_DEPTH*n+actual_depth],sb->id_m[n],0,
                     OPS_CART_COMM, &status);
        for (int d = 0; d < actual_depth; d++)  dat->dirty_dir[2*MAX_DEPTH*n + d] = 0;

        actual_depth = 0;
        for (int d = 0; d < d_pos[n]; d++) if(dat->dirty_dir[2*MAX_DEPTH*n + MAX_DEPTH + d] == 1) actual_depth = d;

        i2 = (-d_m[n]    ) * prod[n-1];
        i4 = (prod[n]/prod[n-1] - (-d_p[n])    ) * prod[n-1];

        //send in negative direction, receive from positive direction
        //printf("Exchaning 2 From:%d To: %d\n", i2, i4);
        if(actual_depth > 0)
        MPI_Sendrecv(&dat->data[i2*size],1,sd->mpidat[MAX_DEPTH*n+actual_depth],sb->id_m[n],1,
                     &dat->data[i4*size],1,sd->mpidat[MAX_DEPTH*n+actual_depth],sb->id_p[n],1,
                     OPS_CART_COMM, &status);

        for (int d = 0; d < actual_depth; d++)  dat->dirty_dir[2*MAX_DEPTH*n + MAX_DEPTH + d] = 0;

      }
    }

    dat->dirtybit = 0;
  }

}

void ops_mpi_reduce_double(ops_arg* arg, double* data)
{
  (void)data;
  //if(arg->argtype == OPS_ARG_GBL && arg->acc != OPS_READ) {
    double result[arg->dim*ops_comm_size];

    if(arg->acc == OPS_INC)//global reduction
      MPI_Allreduce((double *)arg->data, result, arg->dim, MPI_DOUBLE, MPI_SUM, OPS_MPI_WORLD);
    else if(arg->acc == OPS_MAX)//global maximum
      MPI_Allreduce((double *)arg->data, result, arg->dim, MPI_DOUBLE, MPI_MAX, OPS_MPI_WORLD);
    else if(arg->acc == OPS_MIN)//global minimum
      MPI_Allreduce((double *)arg->data, result, arg->dim, MPI_DOUBLE, MPI_MIN, OPS_MPI_WORLD);
    else if(arg->acc == OPS_WRITE) {//any
      MPI_Allgather((double *)arg->data, arg->dim, MPI_DOUBLE, result, arg->dim, MPI_DOUBLE, OPS_MPI_WORLD);
      for (int i = 1; i < ops_comm_size; i++) {
        for (int j = 0; j < arg->dim; j++) {
          if (result[i*arg->dim+j] != 0.0)
            result[j] = result[i*arg->dim+j];
        }
      }
    }
    memcpy(arg->data, result, sizeof(double)*arg->dim);
  //}
}



void ops_mpi_reduce_float(ops_arg* arg, float* data)
{
  (void)data;

  //if(arg->argtype == OPS_ARG_GBL && arg->acc != OPS_READ) {
    float result[arg->dim*ops_comm_size];

    if(arg->acc == OPS_INC)//global reduction
      MPI_Allreduce((float *)arg->data, result, arg->dim, MPI_FLOAT, MPI_SUM, OPS_MPI_WORLD);
    else if(arg->acc == OPS_MAX)//global maximum
      MPI_Allreduce((float *)arg->data, result, arg->dim, MPI_FLOAT, MPI_MAX, OPS_MPI_WORLD);
    else if(arg->acc == OPS_MIN)//global minimum
      MPI_Allreduce((float *)arg->data, result, arg->dim, MPI_FLOAT, MPI_MIN, OPS_MPI_WORLD);
    else if(arg->acc == OPS_WRITE) {//any
      MPI_Allgather((float *)arg->data, arg->dim, MPI_FLOAT, result, arg->dim, MPI_FLOAT, OPS_MPI_WORLD);
      for (int i = 1; i < ops_comm_size; i++) {
        for (int j = 0; j < arg->dim; j++) {
          if (result[i*arg->dim+j] != 0.0f)
            result[j] = result[i*arg->dim+j];
        }
      }
    }
    memcpy(arg->data, result, sizeof(float)*arg->dim);
  //}
}


void ops_mpi_reduce_int(ops_arg* arg, int* data)
{
  (void)data;

  //if(arg->argtype == OPS_ARG_GBL && arg->acc != OPS_READ) {
    int result[arg->dim*ops_comm_size];

    if(arg->acc == OPS_INC)//global reduction
      MPI_Allreduce((int *)arg->data, result, arg->dim, MPI_INT, MPI_SUM, OPS_MPI_WORLD);
    else if(arg->acc == OPS_MAX)//global maximum
      MPI_Allreduce((int *)arg->data, result, arg->dim, MPI_INT, MPI_MAX, OPS_MPI_WORLD);
    else if(arg->acc == OPS_MIN)//global minimum
      MPI_Allreduce((int *)arg->data, result, arg->dim, MPI_INT, MPI_MIN, OPS_MPI_WORLD);
    else if(arg->acc == OPS_WRITE) {//any
      MPI_Allgather((int *)arg->data, arg->dim, MPI_INT, result, arg->dim, MPI_INT, OPS_MPI_WORLD);
      for (int i = 1; i < ops_comm_size; i++) {
        for (int j = 0; j < arg->dim; j++) {
          if (result[i*arg->dim+j] != 0)
            result[j] = result[i*arg->dim+j];
        }
      }
    }
    memcpy(arg->data, result, sizeof(int)*arg->dim);
  //}
}

void ops_set_halo_dirtybit(ops_arg *arg)
{
  arg->dat->dirtybit = 1;
  for(int i = 0; i<2*arg->dat->block->dims*MAX_DEPTH;i++) arg->dat->dirty_dir[i] = 1;
}


void ops_H_D_exchanges(ops_arg *args, int nargs)
{
  (void)nargs;
  (void)args;
}

void ops_set_dirtybit_host(ops_arg *args, int nargs)
{
  (void)nargs;
  (void)args;
}
