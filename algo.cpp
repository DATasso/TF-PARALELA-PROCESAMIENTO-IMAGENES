//Uncomment the following line if you are compiling this code in Visual Studio
//#include "stdafx.h"

#include <opencv2/opencv.hpp>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <string.h>
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
    Mat filasFaltantes;
    int width, height, channels, cantFaltantes;
    std::string ruta;
    std::string operacion(argv[1]);

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
            return EXIT_FAILURE;
        }

        if (mi_rango == 0) {
            ruta = argv[2];
            input = imread(ruta);
            width = input.cols;
            height = input.rows;
            channels = input.channels();
            cantFaltantes = (height) - height%procesadores;
            tam = width*height*channels;
            datosPP = height/procesadores * width * channels;
            if(height%procesadores != 0){
                    for (int i = cantFaltantes; i<input.rows;i++){
                    if(i==cantFaltantes){
                        filasFaltantes = input.row(i).clone();
                    }
                    else{
                        vconcat(filasFaltantes, input.row(i).clone(), filasFaltantes);
                    }
                }
            }
            if(argv[1] == std::string("1") || argv[1] == std::string("2")){
                output = Mat(height - height%procesadores, width, CV_8UC3);
            }
            else if(argv[1] == std::string("3")){
                output = Mat(height*2 - (height%procesadores)*2, width*2, CV_8UC3);
            }
            else{
                std::cout << "No existe una argv[1] con ese numero." << std::endl;
                return EXIT_FAILURE;
            }
        }

        // MPI_Bcast: Envio de el mismo dato hacia todos los procesos, desde el raiz (mi_rango == 0), incluyendose.
        MPI_Bcast(&width, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&height, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&channels, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&tam, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&datosPP, 1, MPI_INT, 0, MPI_COMM_WORLD);

        // Creación de las imagenes de un tamaño de (height/procesadores) filas por procesador.
        // MPI_Scatter: a diferencia de Bcast, separa un mensaje en partes o trozos iguales y las envia individualmente al resto de los procesos y a si mismo.
        Mat test = Mat(height/procesadores, width, CV_8UC3);
        MPI_Scatter(input.data, datosPP, MPI_BYTE, test.data, datosPP, MPI_BYTE, 0, MPI_COMM_WORLD);
        
        //Aplicación de cada operación.
        if(argv[1] == std::string("1")){
            GaussianBlur(test, test, Size(15, 15), 0);
            if(mi_rango == 0 && (height%procesadores != 0)){
                GaussianBlur(filasFaltantes, filasFaltantes, Size(15, 15), 0);
            }
        }
        else if(argv[1] == std::string("2")){
            cvtColor(test, test, cv::COLOR_BGR2GRAY);
            cvtColor(test, test, cv::COLOR_GRAY2BGR);
            if(mi_rango == 0 && (height%procesadores != 0)){
                cvtColor(filasFaltantes, filasFaltantes, cv::COLOR_BGR2GRAY);
                cvtColor(filasFaltantes, filasFaltantes, cv::COLOR_GRAY2BGR);
            }
        }
        else if(argv[1] == std::string("3")){
            resize(test,test,Size(width*2,(height/procesadores)*2));
            if(mi_rango == 0 && (height%procesadores != 0)){
                resize(filasFaltantes,filasFaltantes,Size(width*2,(height%procesadores)*2));
            }
        }

        // Se bloquea el proceso hasta que todos los procesos lo ejecuten.
        MPI_Barrier(MPI_COMM_WORLD);

        // MPI_Gather: Inverso de MPI_Scatter, cada proceso envía el contenido de su buffer de salida al proceso raíz, este guarda por rango del proceso. Finalmente se reune en el proceso raiz (mi_rango == 0) el mensaje completo y ordenado.
        // Si la argv[1] es difuminar o pasar a escala de grises, se tiene la misma cantidad de datos de entrada y salida. 
        // En el caso 3 (escalar la imagen sin perder la calidad), se cambia la cantidad de datos de salida.
        if(argv[1] == std::string("1") || argv[1] == std::string("2")){
            MPI_Gather(test.data, datosPP, MPI_BYTE, output.data,datosPP, MPI_BYTE, 0, MPI_COMM_WORLD);
        }
        else if(argv[1] == std::string("3")){
            MPI_Gather(test.data, datosPP*4, MPI_BYTE, output.data,datosPP*4, MPI_BYTE, 0, MPI_COMM_WORLD);
        }

        MPI_Barrier(MPI_COMM_WORLD);

        if(mi_rango==0){
            time_t rawtime;
            struct tm * timeinfo;
            char buffer[80];

            time (&rawtime);
            timeinfo = localtime(&rawtime);

            strftime(buffer,sizeof(buffer),"%Y%m%d%H%M%S",timeinfo);
            std::string str(buffer);
            std::string textoSalida = "operacion_" + operacion + "_" + str +".png";

            if(height%procesadores != 0){
                //vconcat(output, filasFaltantes, output);
            }

            imwrite(textoSalida, output);
        }
        MPI_Finalize();
    }
    return EXIT_SUCCESS;
}
