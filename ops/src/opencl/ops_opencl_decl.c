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

/** @brief ops opencl backend implementation
  * @author Gihan Mudalige
  * @details Implements the OPS API calls for the opencl backend
  */

#if defined(__APPLE__) || defined(__MACOSX)
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif


#include <ops_lib_cpp.h>
#include <ops_opencl_rt_support.h>

extern ops_opencl_core OPS_opencl_core;

#ifndef __XDIMS__ //perhaps put this into a separate headder file
#define __XDIMS__
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
int xdim18;
int xdim19;
#endif /* __XDIMS__ */

void ops_init ( int argc, char ** argv, int diags )
{
  ops_init_core ( argc, argv, diags );

  if ((OPS_block_size_x*OPS_block_size_y) > 1024) {
    printf ( " OPS_block_size_x*OPS_block_size_y should be less than 1024 -- error OPS_block_size_*\n" );
    exit ( -1 );
  }
  for ( int n = 1; n < argc; n++ )
  {
    if ( strncmp ( argv[n], "OPS_CL_DEVICE=", 14 ) == 0 )
    {
      OPS_cl_device = atoi ( argv[n] + 14 );
      printf ( "\n OPS_cl_device = %d \n", OPS_cl_device );
    }
  }
    
  openclDeviceInit ( argc, argv );
}

void ops_exit()
{
  ops_opencl_exit(); // frees dat_d memory
  ops_exit_core(); // frees lib core variables
}

ops_dat ops_decl_dat_char(ops_block block, int size, int *dat_size, int* d_m,
                           int* d_p, char* data,
                           int type_size, char const * type, char const * name )
{
  int edge_dat = 0; //flag indicating that this is an edge dat

  sub_block_list sb = OPS_sub_block_list[block->index]; //get sub-block geometries

  int* sub_size = (int *)xmalloc(sizeof(int) * sb->ndim);

  for(int n=0;n<sb->ndim;n++){
    if(dat_size[n] != 1) { //i.e. this dat is a regular data block that needs to decomposed
      //compute the local array sizes for this dat for this dimension
      //including max halo depths

      //do check to see if the sizes match with the blocks size
      /** TO DO **/

      //compute allocation size - which includes the halo
      sub_size[n] = sb->sizes[n] - d_m[n] - d_p[n];
    }
    else { // this dat is a an edge data block that needs to be replicated on each MPI process
      //apply the size as 1 for this dimension, later to be replicated on each process
      sub_size[n] = 1;
      edge_dat = 1;

    }
  }

  /** ---- allocate an empty dat based on the local array sizes computed
         above on each MPI process                                      ---- **/

  ops_dat dat = ops_decl_dat_temp_core(block, size, sub_size, d_m, d_p, data, type_size, type, name );
  if( edge_dat == 1) dat->e_dat = 1; //this is an edge dat

  int bytes = size*type_size;
  for (int i=0; i<sb->ndim; i++) bytes = bytes*sub_size[i];
  dat->data = (char*) calloc(bytes, 1); //initialize data bits to 0
  dat->user_managed = 0;

  ops_cpHostToDevice ( ( void ** ) &( dat->data_d ),
    ( void ** ) &( dat->data ), bytes );

  return dat;
}


ops_arg ops_arg_dat( ops_dat dat, ops_stencil stencil, char const * type, ops_access acc )
{
  return ops_arg_dat_core( dat, stencil, acc );
}

ops_arg ops_arg_dat_opt( ops_dat dat, ops_stencil stencil, char const * type, ops_access acc, int flag )
{
  ops_arg temp = ops_arg_dat_core( dat, stencil, acc );
  (&temp)->opt = flag;
  return temp;
}

ops_arg ops_arg_gbl_char( char * data, int dim, int size, ops_access acc )
{
  return ops_arg_gbl_core( data, dim, size, acc );
}

void ops_print_dat_to_txtfile(ops_dat dat, const char *file_name)
{
  //need to get data from GPU
  ops_opencl_get_data(dat);
  ops_print_dat_to_txtfile_core(dat, file_name);
}

void ops_printf(const char* format, ...)
{
  va_list argptr;
  va_start(argptr, format);
  vprintf(format, argptr);
  va_end(argptr);
}

void ops_fprintf(FILE *stream, const char *format, ...)
{
  va_list argptr;
  va_start(argptr, format);
  vfprintf(stream, format, argptr);
  va_end(argptr);
}

void ops_decomp(ops_block block, int g_ndim, int* g_sizes)
{
  sub_block_list sb_list= (sub_block_list)xmalloc(sizeof(sub_block));
  int *coords = (int *) xmalloc(g_ndim*sizeof(int));
  int *disps = (int *) xmalloc(g_ndim*sizeof(int));
  int *sizes = (int *) xmalloc(g_ndim*sizeof(int));
  int *id_m = (int *) xmalloc(g_ndim*sizeof(int));
  int *id_p = (int *) xmalloc(g_ndim*sizeof(int));
  int *istart = (int *) xmalloc(g_ndim*sizeof(int));
  int *iend = (int *) xmalloc(g_ndim*sizeof(int));

  for(int n=0; n<g_ndim; n++){
    coords[n] = 0;
    disps[n]  = 0;
    sizes[n]  = g_sizes[n];
    istart[n] = 0;
    iend[n]   = g_sizes[n] -1;
    id_m[n]   = -2;
    id_p[n]   = -2;
  }

  sb_list->block = block;
  sb_list->ndim = g_ndim;
  sb_list->coords = coords;
  sb_list->id_m = id_m;
  sb_list->id_p = id_p;
  sb_list->sizes = sizes;
  sb_list->disps = disps;
  sb_list->istart = istart;
  sb_list->iend = iend;

  OPS_sub_block_list[block->index] = sb_list;
}

void ops_partition(int g_ndim, int* g_sizes, char* routine)
{
  //create list to hold sub-grid decomposition geometries for each mpi process
  OPS_sub_block_list = (sub_block_list *)xmalloc(OPS_block_index*sizeof(sub_block_list));

  OPS_sub_block_list = (sub_block_list *)xmalloc(OPS_block_index*sizeof(sub_block_list));

  for(int b=0; b<OPS_block_index; b++){ //for each block
    ops_block block=OPS_block_list[b];
    ops_decomp(block, g_ndim, g_sizes); //for now there is only one block
  }
}

void ops_compute_moment(double t, double *first, double *second) {
  *first = t;
  *second = t*t;
}

/*void ops_decl_const_char ( int dim, char const * type, int size, char * dat,
                         char const * name )
{
  //#warning "const_d is not kept track of, therefore it will not be freed up!"
  // Add constant to constant array
  // Create temporary cl_mem array for constants
  
  cl_mem *const_tmp;
  const_tmp = (cl_mem*) malloc((OPS_opencl_core.n_constants)*sizeof(cl_mem));
  // Copy already existing constant to const_tmp
  for(int i=0; i<OPS_opencl_core.n_constants; i++)
    const_tmp[i] = OPS_opencl_core.constant[i];

  // Allocate memory on host for a larger array of constant data structure
  cl_int ret = 0;
  if(OPS_opencl_core.n_constants != 0) 
    free(OPS_opencl_core.constant);
  
  OPS_opencl_core.n_constants++;
  OPS_opencl_core.constant = (cl_mem*) malloc((OPS_opencl_core.n_constants)*sizeof(cl_mem));

  // Copy already existing pointers
  for(int i=0; i<OPS_opencl_core.n_constants-1; i++)
    OPS_opencl_core.constant[i] = const_tmp[i];
  
  // Allocate new memory space for the new constant
  OPS_opencl_core.constant[OPS_opencl_core.n_constants-1] = clCreateBuffer(OPS_opencl_core.context, CL_MEM_READ_ONLY, dim*size, NULL, &ret);
  clSafeCall( ret );

  // Write the new constant to the memory of the device
  clSafeCall( clEnqueueWriteBuffer(OPS_opencl_core.command_queue, OPS_opencl_core.constant[OPS_opencl_core.n_constants-1], CL_TRUE, 0, dim*size, (void*) dat, 0, NULL, NULL) );
  clSafeCall( clFlush(OPS_opencl_core.command_queue) );
  clSafeCall( clFinish(OPS_opencl_core.command_queue) );
}*/



