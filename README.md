# TF-PARALELA-PROCESAMIENTO-IMAGENES

### Compilación:
```
mpicxx -std=gnu++14 -g algo.cpp -o algo.o `pkg-config --cflags --libs opencv4`
```
### Ejecución:
```
time mpirun ./algo.o
```
