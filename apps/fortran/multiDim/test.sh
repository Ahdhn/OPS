#!/bin/bash

#cd ../../../ops/c
#source ./ruby_intel_source
#make
#cd -
#make
#============================ Test SHSGC ==========================================================
echo '============> Running OpenMP'
KMP_AFFINITY=compact OMP_NUM_THREADS=12 ./multidim_openmp > perf_out
grep "Reduction result" perf_out
grep "Total Wall time" perf_out
rm perf_out
echo '============> Running MPI+OpenMP'
export OMP_NUM_THREADS=2;$MPI_INSTALL_PATH/bin/mpirun -np 12 ./multidim_mpi_openmp > perf_out
grep "Reduction result" perf_out
grep "Total Wall time" perf_out
rm perf_out
echo '============> Running MPI'
$MPI_INSTALL_PATH/bin/mpirun -np 22 ./multidim_mpi > perf_out
grep "Reduction result" perf_out
grep "Total Wall time" perf_out
rm perf_out
echo '============> Running CUDA'
./multidim_cuda OPS_BLOCK_SIZE_X=64 OPS_BLOCK_SIZE_Y=4 > perf_out
grep "Reduction result" perf_out
grep "Total Wall time" perf_out
rm perf_out
echo '============> Running MPI+CUDA'
$MPI_INSTALL_PATH/bin/mpirun -np 2 ./multidim_mpi_cuda OPS_BLOCK_SIZE_X=64 OPS_BLOCK_SIZE_Y=4 > perf_out
grep "Reduction result" perf_out
grep "Total Wall time" perf_out
rm perf_out
echo '============> Running MPI+CUDA with GPU-Direct'
MV2_USE_CUDA=1 $MPI_INSTALL_PATH/bin/mpirun -np 2 ./multidim_mpi_cuda -gpudirect OPS_BLOCK_SIZE_X=64 OPS_BLOCK_SIZE_Y=4 > perf_out
grep "Reduction result" perf_out
grep "Total Wall time" perf_out
rm perf_out
