Alle Aufgaben befinden sich in **separaten Ordnern**. Jeder Ordner enthält die Quelldatei, eine `slurmSubmit.sh` Datei zur Ausführung im Batch-System und einen **`out/` Ordner** für die Kompilate.

-----

## Aufgabe a.) - broken.c

  * **Pfad zum Ausführen und Kompilieren:**
    ```bash
    cd broken/
    ```
  * **Kompilieren (mit OpenMP und Debug-Optimierung):**
    ```bash
    gcc -fopenmp -O0 -Wall -lm -o out/broken broken.c
    ```
  * **Ausführen (Manuell):**
    ```bash
    export OMP_NUM_THREADS=8  # Oder andere Thread-Anzahl einstellen
    out/broken
    ```
  * **Ausführen (Batch-System):**
    ```bash
    sbatch slurmSubmit.sh
    ```

-----

## Aufgabe b.) - sierpinski.c

  * **Pfad zum Ausführen und Kompilieren:**
    ```bash
    cd sierpinski/
    ```
  * **Kompilieren (mit OpenMP und Optimierungslevel 3):**
    ```bash
    gcc -fopenmp -O3 -Wall -o out/sierpinski sierpinski.c
    ```
  * **Ausführen (Manuell):**
    ```bash
    export OMP_NUM_THREADS=8  # Oder andere Thread-Anzahl einstellen
    out/sierpinski
    ```
  * **Ausführen (Batch-System):**
    ```bash
    sbatch slurmSubmit.sh
    ```

-----

## Aufgabe c.) - heatmap_analysis.cpp

  * **Pfad zum Ausführen und Kompilieren:**
    ```bash
    cd heatmap_analysis/
    ```
  * **Kompilieren (mit OpenMP und Optimierungslevel 3 - C++):**
    ```bash
    g++ -fopenmp -O3 -Wall -o out/heatmap_analysis heatmap_analysis.cpp
    ```
  * **Ausführen (Manuell) - Achtung: Argumente sind erforderlich:**
    ```bash
    out/heatmap_analysis 3 4 42 0 10 2 1 1 1
    ```
    > *Hinweis: Die Argumente sind beispielhaft und müssen ggf. angepasst werden.*
  * **Ausführen (Batch-System):**
    ```bash
    sbatch slurmSubmit.sh
    ```
