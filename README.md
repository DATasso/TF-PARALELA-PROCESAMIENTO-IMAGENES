# Proyecto Procesamiento de Imagenes con MPI y OpenCV - Computación Paralela
Proyecto para la asignatura "Computación Paralela y Distribuida" en la UTEM. Docente: Sebastían Salazar.

El proyecto tiene como objetivo explicar la resolución de un problema de procesamiento de imágenes con clustering (MPI) para el estudio del procesamiento en múltiples máquinas por medio de este protocolo. El programa deberá recibir una entrada por medio de CLI y procesa la imagen según los parámetros enviados (Escala de grises, Difuminado o Ampliado de imágen).

## Especificaciones
Es un programa en C++ capaz de procesar imágenes por CLI, además este procesamiento debe ser en múltiples “procesadores” distribuidos en múltiples máquinas, siendo el objeto de estudio la interacción con otros “computadores” por medio de MPI (Message Passing Interface) que permite que éstos se comuniquen entre sí.

La implementación debe escoger una imagen en colores y hacer las siguientes actividades con algoritmos paralelos:
1) Difuminar la imagen.
2) A partir de la imagen en colores pasar a escala de grises.
3) Escalar la imagen sin perder calidad.

## Entorno de desarrollo y requisitos:
- Ubuntu 20.04 LTS de 64 bits.
- C++.
- OpenMPI.
- OpenCV (guía práctica de instalación: https://linuxize.com/post/how-to-install-opencv-on-ubuntu-20-04/)

### Archivos
Archivos de Entrada:
- `algo.cpp` contiene el programa en C++ con MPI y OpenCV,
- `Fubuki.jpg` imagen de ejemplo.

Formato archivos de salida:
`operacion_numOperacion_YYYYmmddHHmmss`

### Compilación:
Método 1:
```
mpicxx -std=gnu++14 -g algo.cpp -o algo.o `pkg-config --cflags --libs opencv4`
```

Método 2: utilizar makefile
```
make
```

### Ejecución (con tiempos de ejecución total):
Método 1:
```
time mpirun ./algo.o numOperacion path
```

Método 2:
```
time mpirun ./dist/programa numOperacion path
```

### Operaciones:
Como se mencionó anteriormente, existen 3 operaciones, estas son:
1) Difuminado Gaussiano.
2) Grayscale.
3) Ampliación de la imagén (ancho*2 , alto*2)
Para utilizar alguna, se escribe el número correspondiente en el argumento para la ejecución (numOperacion)

#### Ejemplos de ejecución Método 1 | Método 2:
`time mpirun ./algo.o 1 Fubuki.jpg` o `time mpirun ./dist/programa 1 Fubuki.jpg`

`time mpirun ./algo.o 2 Fubuki.jpg` o `time mpirun ./dist/programa 2 Fubuki.jpg`

`time mpirun ./algo.o 3 Fubuki.jpg` o `time mpirun ./dist/programa 3 Fubuki.jpg`


## Autores
- Daniel Aguilera Tasso
- Nicolás Andrews Sandoval
