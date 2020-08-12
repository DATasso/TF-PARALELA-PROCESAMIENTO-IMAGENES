#include <opencv2/opencv.hpp> // librería opencv
#include <iostream> // STL (cout,cin,etc)
#include <cstdlib> // biblioteca estandar EXIT_SUCCESS , EXIT_FAILURE
#include <ctime> // libreria para obtener fecha y hora
#include <string.h> // libreria para el manejo de strings
#include "mpi.h" // libreria MPI

using namespace cv;
using namespace std;

/**
 * Función que muestra los participantes del grupo
 */
void participante();

int main(int argc, char** argv)
{
    int mi_rango; /* rango del proceso    */
    int procesadores; /* numero de procesos   */
    int tam = 0; /* tamaño total de la imagen */
    int datosPP; /* tamaño o datos (por filas) segmentados y enviado a los procesadores */
    Mat input; /* Imagen de entrada */
    Mat output; /* Imagen de salida */
    Mat filasFaltantes; /* Imagen para casos especiales de filas faltantes */
    int width, height, channels, cantFaltantes; /* ancho(columnas), altura(filas), canales y cantidad de filas faltantes para casos especiales. */
    int kyk, kxk; /* Valores kernel para difuminado gaussiano, debe ser impar */

    if (argc > 2) {
        std::string ruta; /* string con la ruta del fichero (imagen) */
        std::string operacion(argv[1]); /* string con la operación deseada ingresada */

        /* Comienza las llamadas a MPI */
        MPI_Init(&argc, &argv);

        /* Averiguamos el rango de nuestro proceso */
        MPI_Comm_rank(MPI_COMM_WORLD, &mi_rango);

        /* Averiguamos el número de procesos que estan 
         * ejecutando nuestro porgrama 
         */
        MPI_Comm_size(MPI_COMM_WORLD, &procesadores);
        /* El procesador maestro (0), lee el archivo, obtiene los datos generales (ancho,altura,canales,tamaño) y realiza el calculo de las n filas 
         * y cantidad de datos que se distribuiran a cada procesador y la obtención de filas faltantes, es decir, filas que no se incluyen al realizar
         * la segmentación. Creandose una imagen con las filas faltantes.
         */
        if (mi_rango == 0) {
            if(operacion != std::string("1") && operacion != std::string("2") && operacion != std::string("3") ){
                std::cout << std::endl << "No existe una operación: " << argv[1] << "." << std::endl;
                participante();
                return EXIT_FAILURE; // o utilizar MPI_Abort(MPI_COMM_WORLD, 1);
            }
            ruta = argv[2];
            input = imread(ruta);
            if (input.empty()){
                std:: cout << std::endl << "Fallo al leer imagen, la imagen no fue encontrada.!!" << std::endl;
                participante();
                return EXIT_FAILURE; // o utilizar MPI_Abort(MPI_COMM_WORLD, 1);
            }
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

            /* Para el caso de la difuminación y escala de grises, la cantidad de datos de salida es igual a la de entrada.
             * Se excluyen las faltantes en la distribución y se agregan al final.
             * Para el caso de la difuminación, se calcula el tamaño del kernel en en x(width) e y(height)
             * Para imagenes menores a 300x300, se utiliza un kernel 5x5
            */
            if(argv[1] == std::string("1") || argv[1] == std::string("2")){
                output = Mat(height - height%procesadores, width, CV_8UC3);

                if(argv[1] == std::string("1")){
                    if(width > 300){
                        kxk = width/50;
                        if (kxk%2==0){
                            kxk--;
                        }
                    }
                    else{
                        kxk = 5;
                    }
                    if(height > 300){
                        kyk = height/50;
                        if (kyk%2==0){
                            kyk--;
                        }
                    }
                    else{
                        kyk = 5;
                    }
                    std::cout << "Se utiliza un kernel de tamaño: " << kxk << "x" << kyk << std::endl;
                }
            }

            /* Para el caso del escalado, la cantidad de datos de salida es distinta (x2). 
             * Se excluyen las faltantes en la distribución y se agregan al final.
             */
            else if(argv[1] == std::string("3")){
                output = Mat(height*2 - (height%procesadores)*2, width*2, CV_8UC3);
            }
        }

        // MPI_Bcast: Envio de el mismo dato hacia todos los procesos, desde el raiz (mi_rango == 0), incluyendose.
        MPI_Bcast(&width, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&height, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&channels, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&tam, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&datosPP, 1, MPI_INT, 0, MPI_COMM_WORLD);
        if(argv[1] == std::string("1")){
            MPI_Bcast(&kxk, 1, MPI_INT, 0, MPI_COMM_WORLD);
            MPI_Bcast(&kyk, 1, MPI_INT, 0, MPI_COMM_WORLD);
        }
        /* Creación de las imagenes de un tamaño de (height/procesadores) filas por procesador.
         * MPI_Scatter: a diferencia de Bcast, separa un mensaje en partes o trozos iguales y las envia individualmente al resto de los procesos y a si mismo.
         */
        Mat test = Mat(height/procesadores, width, CV_8UC3);
        MPI_Scatter(input.data, datosPP, MPI_BYTE, test.data, datosPP, MPI_BYTE, 0, MPI_COMM_WORLD);
        
        /* Aplicación de difuminación gaussiana de cada segmento de imagen por procesador, función GaussianBlur() de openCV.
         * Se incluye la aplicación del filtro a las filas faltantes, en caso de que la segmentación excluya height%procesadores filas.
         */
        if(argv[1] == std::string("1")){
            GaussianBlur(test, test, Size(kyk, kyk), 0);
            if(mi_rango == 0 && (height%procesadores != 0)){
                GaussianBlur(filasFaltantes, filasFaltantes, Size(kyk, kyk), 0);
            }
        }

        /* Aplicación de escala a grises de cada segmento de imagen por procesador, función cvtColor() de openCV.
         * Al pasar a grayscale, los canales pasan a ser uno solo, por lo que es necesario volver a BGR mediante para conservar el tamaño y cantidad de pixeles del archivo.
         * Se incluye la aplicación del filtro a las filas faltantes, en caso de que la segmentación excluya height%procesadores filas.
        */
        else if(argv[1] == std::string("2")){
            cvtColor(test, test, cv::COLOR_BGR2GRAY);
            cvtColor(test, test, cv::COLOR_GRAY2BGR);
            if(mi_rango == 0 && (height%procesadores != 0)){
                cvtColor(filasFaltantes, filasFaltantes, cv::COLOR_BGR2GRAY);
                cvtColor(filasFaltantes, filasFaltantes, cv::COLOR_GRAY2BGR);
            }
        }

        /*
            Aplicación de escalado*2 de cada segmento de imagen por procesador, función resize() de openCV.
            Se incluye la aplicación del filtro a las filas faltantes, en caso de que la segmentación excluya height%procesadores filas.
        */
        else if(argv[1] == std::string("3")){
            resize(test,test,Size(width*2,(height/procesadores)*2));
            if(mi_rango == 0 && (height%procesadores != 0)){
                resize(filasFaltantes,filasFaltantes,Size(width*2,(height%procesadores)*2));
            }
        }

        // Se bloquea el proceso hasta que todos los procesos lo ejecuten.
        MPI_Barrier(MPI_COMM_WORLD);

        /* 
            MPI_Gather: Inverso de MPI_Scatter, cada proceso envía el contenido de su buffer de salida al proceso raíz, este guarda por rango del proceso. Finalmente se reune en el proceso raiz (mi_rango == 0) el mensaje completo y ordenado.
            Si la argv[1] es difuminar o pasar a escala de grises, se tiene la misma cantidad de datos de entrada y salida. 
            En el caso 3 (escalar la imagen sin perder la calidad), se cambia la cantidad de datos de salida.
        */
        if(argv[1] == std::string("1") || argv[1] == std::string("2")){
            MPI_Gather(test.data, datosPP, MPI_BYTE, output.data,datosPP, MPI_BYTE, 0, MPI_COMM_WORLD);
        }
        else if(argv[1] == std::string("3")){
            MPI_Gather(test.data, datosPP*4, MPI_BYTE, output.data,datosPP*4, MPI_BYTE, 0, MPI_COMM_WORLD);
        }

        MPI_Barrier(MPI_COMM_WORLD);

        /* Finalmente, en el rango 0, se obtiene la fecha y hora actual, para luego obtener un string operacion_numeroDeOperación_yyyyymmddhhmmss.png
         * Se concatenan las filas faltantes en output para obtener imagen final. Funcion vconcat() de opencv.
         */
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
                vconcat(output, filasFaltantes, output);
            }
            
            /* Ya que se utiliza un kernel (AxB), para aplicar el difuminado gaussiano correctamente, se debe trabajar con las (B-1)/2
             * siguientes y anteriores a cada fila, es en este caso al ser 15x15, las 7 anteriores y 7 siguientes.
             * Por lo que las primeras 6 y las ultimas 6 filas, necesitan trabajarse por separado
             * (pueden enviarse a cada procesador, sin embargo es mas tedioso y carga extra a cada procesador)
             * de esta forma, el procesador 0 se encarga de arreglar las -6 a +6 filas desde donde inicia a trabajar la imagen cada procesador,
             * sin incluir el primero y el ultimo dado que es donde comienza y termina cada segmento respectivamente de la original.
             */
            if(procesadores>1 && operacion == std::string("1")){
                Mat extraGauss;
                int filasKernel = (kyk-1)/2;
                for(int i=1; i < procesadores; i++){
                    extraGauss = input.row((i*height/procesadores) - (filasKernel*2) - 1);
                    for(int k = (filasKernel*-2 + 2); k < (filasKernel*2); k++){
                        vconcat(extraGauss, input.row((i*height/procesadores) + k), extraGauss);
                    }
                    GaussianBlur(extraGauss, extraGauss, Size(kyk,kyk), 0);
                    for (int j=0; j<(filasKernel*2);j++){
                        extraGauss.row(j+filasKernel).copyTo(output.row(i*height/procesadores +(j-(filasKernel-1))));
                    }
                }
            }

            imwrite(textoSalida, output);
            std:: cout << std::endl <<"Se creo correctamente la imagen: " << textoSalida << std:: endl;
            participante();
        }
        MPI_Finalize();
    }
    return EXIT_SUCCESS;
}

void participante() {
    std::cout << std::endl << "=== Realizado por ===" << std::endl;
    std::cout << "- Daniel Aguilera Tasso" << std::endl << "- Nicolas Andrews Sandoval" << std::endl;
}
