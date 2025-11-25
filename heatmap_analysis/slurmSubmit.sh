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
#SBATCH --output=out/heatmap_analysis.out.%j
#SBATCH --error=out/error/heatmap_analysis.err.%j
    
#path/to/binary
out/heatmap_analysis 10000 10000 123 0 100 5000 0 8 100