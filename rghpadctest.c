#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <error.h>
#include <stdint.h>
#include <sys/mman.h>
#include "adcLinuxDriverAPI.h"

// The start address and length of the Lightweight bridge


int main(int argc, char ** argv)
{
	int i;
    int adcFile;
    unsigned int valorLeido = 0;
    int res;
    int canalElegido;
    int dimensionLecturaRecomendada;
    int dimensionLecturaTotal;
    int paquetesLeidos = 0;
    char* bufferMM = 0;
    if(argc!=2){
            printf("Se necesita saber que canal desea usar\n");
            exit(EXIT_FAILURE);

    }
    canalElegido = atoi(argv[1]);
    // Open up the /dev/mem device (aka, RAM)
    adcFile = open("/dev/rghpadc", O_RDWR);
    if(adcFile < 0) {
        printf("No se pudo abrir el archivo :(");
        exit(EXIT_FAILURE);
    }

    

//    printf("Captura ADC\n\n");
//    for(i=0;i<20;i++){
//        res = ioctl(adcFile, RGHPADC_LEEARVALORADC, &valorLeido);
//        if(res!=0){
//            printf("Error leyend datos %d\n", res);
//                break;
//        }
//        printf("\rDato leido: %x         ", valorLeido);
//        fflush(stdout);
//        usleep(500000);
//    }
//    printf("\nFin captura bloqueante ADC\n");
    res = ioctl(adcFile, RGHPADC_CAMBIAR_PUERTO, &canalElegido);
    if(res!=0){
        printf("Error configurando el canal a usar:  %d\n", res);
        exit(EXIT_FAILURE);
    }
    res = ioctl(adcFile, RGHPADC_REINICIAR, 0);
    if(res!=0){
        printf("Error iniciando la captura:  %d\n", res);
        exit(EXIT_FAILURE);
    }
    res = ioctl(adcFile, RGHPADC_MEJORDIMLECTURA, &dimensionLecturaRecomendada);
    if(res!=0){
        printf("Error RGHPADC_MEJORDIMLECTURA:  %d\n", res);
        exit(EXIT_FAILURE);
    }
    printf("RGHPADC_MEJORDIMLECTURA : %d\n", dimensionLecturaRecomendada);

    res = ioctl(adcFile, RGHPADC_DIMTOTAL, &dimensionLecturaTotal);
    if(res!=0){
        printf("Error RGHPADC_DIMTOTAL:  %d\n", res);
        exit(EXIT_FAILURE);
    }
    printf("RGHPADC_DIMTOTAL : %d\n", dimensionLecturaTotal);


    bufferMM = mmap(0, dimensionLecturaTotal, PROT_READ, MAP_PRIVATE, adcFile, 0);





    printf("Se va a iniciar la captura\n");
    //printf("Revise mensajes del sistema durante 1 minuto\n");
    res = ioctl(adcFile, RGHPADC_INICIAR_CAPTURA, 0);
    if(res!=0){
        printf("Error iniciando la captura:  %d\n", res);
        exit(EXIT_FAILURE);
    }



    for(i=0;i<10000;i++){
        res = read(adcFile, 0, dimensionLecturaRecomendada);
        if(res<0){
            printf("Error leyend datos %d\n", res);
            break;
        }
        if(res!=0){
            paquetesLeidos++;
            printf("\rPaquetes leidos: %d            ", paquetesLeidos);
        }

        fflush(stdout);
        usleep(5000);
    }

    printf("Prueba finalizada\n");
    close(adcFile);
    exit(EXIT_SUCCESS);
}



