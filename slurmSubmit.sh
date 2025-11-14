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
#SBATCH --output=/home/users/0019/uk097201/out/broken.out.%j
#SBATCH --error=/home/users/0019/uk097201/out/broken.err.%j

export OMP_NUM_THREADS=4
#cd /path/to/broken
./broken