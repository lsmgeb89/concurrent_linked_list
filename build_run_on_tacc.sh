#!/bin/bash
#

# build and run a batch session
module load gcc
make -C src
sbatch tacc_batch.sh
