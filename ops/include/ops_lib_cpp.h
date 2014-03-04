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

/** @brief ops c++ header file
  * @author Gihan Mudalige
  * @details This header file should be included by all C++ OPS applications
  */


#ifndef __OPS_LIB_CPP_H
#define __OPS_LIB_CPP_H

#include <ops_lib_core.h>


/*
* run-time type-checking routines
*/

inline int type_error (const double * a, const char *type ) {
  (void)a; return (strcmp ( type, "double" ) && strcmp ( type, "double:soa" ));
}
inline int type_error (const float * a, const char *type ) {
  (void)a; return (strcmp ( type, "float" ) && strcmp ( type, "float:soa" ));
}
inline int type_error (const int * a, const char *type ) {
  (void)a; return (strcmp ( type, "int" ) && strcmp ( type, "int:soa" ));
}
inline int type_error (const uint * a, const char *type ) {
  (void)a; return (strcmp ( type, "uint" ) && strcmp ( type, "uint:soa" ));
}
inline int type_error (const ll * a, const char *type ) {
  (void)a; return (strcmp ( type, "ll" ) && strcmp ( type, "ll:soa" ));
}
inline int type_error (const ull * a, const char *type ) {
  (void)a; return (strcmp ( type, "ull" ) && strcmp ( type, "ull:soa" ));
}
inline int type_error (const bool * a, const char *type ) {
  (void)a; return (strcmp ( type, "bool" ) && strcmp ( type, "bool:soa" ));
}

/*
* zero constants
*/

#define ZERO_double 0.0;
#define INFINITY_double INFINITY;
#define ZERO_float 0.0f;
#define INFINITY_float INFINITY;
#define ZERO_int 0;
#define INFINITY_int INFINITY;
#define ZERO_uint 0;
#define INFINITY_uint INFINITY;
#define ZERO_ll 0;
#define INFINITY_ll INFINITY;
#define ZERO_ull 0;
#define INFINITY_ull INFINITY;
#define ZERO_bool 0;


extern int OPS_diags;

extern int OPS_block_index, OPS_block_max,
           OPS_dat_index, OPS_dat_max;

extern ops_block * OPS_block_list;
extern Double_linked_list OPS_dat_list; //Head of the double linked list
extern ops_arg *OPS_curr_args;

extern sub_block_list *OPS_sub_block_list;

void ops_init( int argc, char **argv, int diags_level );
void ops_exit();

ops_dat ops_decl_dat_char(ops_block, int, int*, int*, int*, char *, int, char const*, char const* );
ops_dat ops_decl_dat_mpi_char(ops_block block, int size, int *dat_size, int* offset, int* tail,
                           char* data, int type_size, char const * type, char const * name );

ops_arg ops_arg_dat( ops_dat dat, ops_stencil stencil, char const * type, ops_access acc );
ops_arg ops_arg_idx( );

ops_arg ops_arg_gbl_char( char * data, int dim, int size, ops_access acc );
void ops_decl_const_char( int, char const *, int, char *, char const* );


template < class T >
ops_arg ops_arg_gbl ( T * data, int dim, char const * type, ops_access acc )
{
  return ops_arg_gbl_char ( ( char * ) data, dim, sizeof(T), acc );
}

template < class T >
void ops_decl_const2 ( char const * name, int dim, char const *type, T * data )
{
  if ( type_error ( data, type ) )
  {
    printf ( "incorrect type specified for constant \"%s\" \n", name ); exit ( 1 );
  }

  ops_decl_const_char ( dim, type, sizeof ( T ), (char *) data, name );
}

template < class T >
void ops_decl_const ( char const * name, int dim, char const * type, T * data )
{
  (void)dim;
  if ( type_error ( data, type ) )
  {
    printf ( "incorrect type specified for constant in op_decl_const" );
    exit ( 1 );
  }
}

/*template < class T >
ops_dat ops_decl_dat ( ops_block block, int data_size,
                      int *block_size, int* offset, T *data,
                      char const * type,
                      char const * name )
{

  if ( type_error ( data, type ) ) {
    printf ( "incorrect type specified for dataset \"%s\" \n", name );
    exit ( 1 );
  }

  return ops_decl_dat_char(block, data_size, block_size, offset, (char *)data, sizeof(T), type, name );

}*/


template < class T >
ops_dat ops_decl_dat ( ops_block block, int data_size,
                      int *block_size, int* offset, int* tail, T *data,
                      char const * type,
                      char const * name )
{

  if ( type_error ( data, type ) ) {
    printf ( "incorrect type specified for dataset \"%s\" \n", name );
    exit ( 1 );
  }

  return ops_decl_dat_char(block, data_size, block_size, offset, tail, (char *)data, sizeof(T), type, name );

}


void ops_timers( double *cpu, double *et );
void ops_print_dat_to_txtfile(ops_dat dat, const char *file_name);



//
// wrapper functions to handle MPI global reductions
//

inline void ops_mpi_reduce(ops_arg* args, float *data)
{
  ops_mpi_reduce_float(args,data);
}

inline void ops_mpi_reduce(ops_arg* args, double *data)
{
  ops_mpi_reduce_double(args,data);
}

inline void ops_mpi_reduce(ops_arg* args, int *data)
{
  ops_mpi_reduce_int(args,data);
}

//needed as a dummy, "do nothing" routine for the non-mpi backends
template <class T>
void ops_mpi_reduce(ops_arg* args, T* data)
{
  //printf("should not be here\n");
}





#endif /* __OPS_LIB_CPP_H */
