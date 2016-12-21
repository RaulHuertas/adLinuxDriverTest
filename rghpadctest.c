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

    res = ioctl(adcFile, RGHPADC_CAMBIAR_PUERTO, &canalElegido);
    if(res!=0){
        printf("Error configurando el canal a usar:  %d\n", res);
        exit(EXIT_FAILURE);
    }

    printf("Captura ADC\n\n");
    for(i=0;i<20;i++){
        res = ioctl(adcFile, RGHPADC_LEEARVALORADC, &valorLeido);
        if(res!=0){
            printf("Error leyend datos %d\n", res);
                break;
        }
        printf("\rDato leido: %x         ", valorLeido);
        fflush(stdout);
        usleep(500000);
    }

    printf("\nFin captura ADC\n");


    close(adcFile);
    exit(EXIT_SUCCESS);
}



