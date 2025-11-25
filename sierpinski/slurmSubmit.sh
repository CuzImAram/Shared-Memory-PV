#!/bin/bash
####### Mail Notify / Job Name / Comment #######
#SBATCH --job-name="sierpinski.c"

####### Partition #######
#SBATCH --partition=pub23

####### Ressources #######
#SBATCH --time=0-00:05:00
#SBATCH --mem-per-cpu=1000

####### Node Info #######
#SBATCH --exclusive
#SBATCH --nodes=1

####### Output #######
#SBATCH --output=out/sierpinski.out.%j
#SBATCH --error=out/error/sierpinski.err.%j

#path/to/binary
export OMP_NUM_THREADS=8
out/sierpinski