#!/bin/bash
####### Mail Notify / Job Name / Comment #######
#SBATCH --job-name="heatmap_analysis.c"

####### Partition #######
#SBATCH --partition=pub23

####### Ressources #######
#SBATCH --time=0-00:05:00
#SBATCH --mem-per-cpu=1000

####### Node Info #######
#SBATCH --exclusive
#SBATCH --nodes=1

####### Output #######
#SBATCH --output=/home/users/0019/uk097201/PV/Shared-Memory-PV/heatmap_analysis/out/heatmap_analysis.out.%j
#SBATCH --error=/home/users/0019/uk097201/PV/Shared-Memory-PV/heatmap_analysis/out/heatmap_analysis.err.%j

export OMP_NUM_THREADS=48
/home/users/0019/uk097201/PV/Shared-Memory-PV/heatmap_analysis/out/heatmap_analysis 1024 786 123 0 100 55 0 1 100