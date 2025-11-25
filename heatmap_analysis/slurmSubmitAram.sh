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
#SBATCH --output=/home/users/0019/uk097201/PV/Shared-Memory-PV/heatmap_analysis/out/heatmap_analysis_new.out.%j
#SBATCH --error=/home/users/0019/uk097201/PV/Shared-Memory-PV/heatmap_analysis/out/error/heatmap_analysis_new.err.%j
    
/home/users/0019/uk097201/PV/Shared-Memory-PV/heatmap_analysis/out/heatmap_analysis 10000 10000 123 0 100 5000 0 8 100