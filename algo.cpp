//Uncomment the following line if you are compiling this code in Visual Studio
//#include "stdafx.h"

#include <opencv2/opencv.hpp>
#include <iostream>
#include "mpi.h"

using namespace cv;
using namespace std;

int main(int argc, char** argv)
{
    int mi_rango; /* rango del proceso    */
    int procesadores; /* numero de procesos   */
    int tam = 0;
    int datosPP;
    const char* filename;
    Mat input;
    Mat output;
    Mat og_image;
    Mat gs_image;
    int width, height, channels;

    if (argc > 0) {
        /* Comienza las llamadas a MPI */
        MPI_Init(&argc, &argv);

        /* Averiguamos el rango de nuestro proceso */
        MPI_Comm_rank(MPI_COMM_WORLD, &mi_rango);

        /* Averiguamos el número de procesos que estan 
         * ejecutando nuestro porgrama 
         */
        MPI_Comm_size(MPI_COMM_WORLD, &procesadores);

        if (procesadores < 1) {
            fprintf(stderr, "\nLa implementación requiere al menos 2 procesadores\n");
            return 0;
        }

        if (mi_rango == 0) {
            // std::string operacion(argv[1]);
            // std::string ruta(argv[2]);
            input = imread("Fubuki.jpg");
            width = input.cols;
            height = input.rows;
            channels = input.channels();
            tam = width*height*channels;
            datosPP = height/procesadores * width * channels;
            output = Mat(height, width, CV_8UC3);
            // output = Mat(height/2, width/2, CV_8UC3);
        }

        MPI_Bcast(&width, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&height, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&channels, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&tam, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&datosPP, 1, MPI_INT, 0, MPI_COMM_WORLD);

        Mat test = Mat(height/procesadores, width, CV_8UC3);
        MPI_Scatter(input.data, datosPP, MPI_BYTE, test.data, datosPP, MPI_BYTE, 0, MPI_COMM_WORLD);
        
        GaussianBlur(test, test, Size(15, 15), 0);
        //resize(test,test,Size(width/2,(height/procesadores)/2));
        MPI_Barrier(MPI_COMM_WORLD);

        MPI_Gather(test.data, datosPP, MPI_BYTE, output.data,datosPP, MPI_BYTE, 0, MPI_COMM_WORLD);
        // MPI_Gather(test.data, datosPP / 4, MPI_BYTE, output.data,datosPP/4, MPI_BYTE, 0, MPI_COMM_WORLD);
        MPI_Barrier(MPI_COMM_WORLD);

        if(mi_rango==0){
            imwrite("elbuenblur.png", output);
        }
        MPI_Finalize();
    }
    return 0;
}