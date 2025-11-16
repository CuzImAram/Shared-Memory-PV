#!/bin/bash
####### Mail Notify / Job Name / Comment #######
#SBATCH --job-name="sierpinski.c"

####### Partition #######
#SBATCH --partition=pub17

####### Ressources #######
#SBATCH --time=0-00:05:00
#SBATCH --mem-per-cpu=1000

####### Node Info #######
#SBATCH --exclusive
#SBATCH --nodes=1

####### Output #######
#SBATCH --output=/home/users/0019/uk097201/PV/Shared-Memory-PV/sierpinski/out/sierpinski.out.%j
#SBATCH --error=/home/users/0019/uk097201/PV/Shared-Memory-PV/sierpinski/out/sierpinski.err.%j

export OMP_NUM_THREADS=8
/home/users/0019/uk097201/PV/Shared-Memory-PV/sierpinski/out/sierpinski