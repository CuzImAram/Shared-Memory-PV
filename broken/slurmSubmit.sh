#!/bin/bash
####### Mail Notify / Job Name / Comment #######
#SBATCH --job-name="broken.c"

####### Partition #######
#SBATCH --partition=pub23

####### Ressources #######
#SBATCH --time=0-00:05:00
#SBATCH --mem-per-cpu=1000

####### Node Info #######
#SBATCH --exclusive
#SBATCH --nodes=1

####### Output #######
#SBATCH --output=out/broken.out.%j
#SBATCH --error=out/error/broken.err.%j

export OMP_NUM_THREADS=4
#path/to/binary
out/broken