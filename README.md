Aufgabe a.)
Path: cd broken/ // In diesen Path muss man um es auszuführen und zu compilen
Compile: gcc -fopenmp -O3 -Wall -o out/broken broken.c
Ausführen: 
export OMP_NUM_THREADS=8 (Oder andere Thread anzahl)
out/broken
Ausführen Batch: 
sbatch slurmSubmit.sh

Aufgabe b.)
Path: cd sierpinski/ // In diesen Path muss man um es auszuführen und zu compilen
Compile: gcc -fopenmp -O3 -Wall -o out/sierpinski sierpinski.c
Ausführen: 
export OMP_NUM_THREADS=8 (Oder andere Thread anzahl)
out/sierpinski
Ausführen Batch: 
sbatch slurmSubmit.sh

Aufgabe c.)
Path: cd heatmap_analysis/ // In diesen Path muss man um es auszuführen und zu compilen
Compile: g++ -fopenmp -O3 -Wall -o out/heatpmap_analysis heatmap_analysis.cpp
Ausführen: 
out/heatmap_analysis 3 4 42 0 10 2 1 1 1
Ausführen Batch: 
sbatch slurmSubmit.sh