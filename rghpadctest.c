#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <error.h>
#include <stdint.h>
#include <sys/mman.h>
#include "adcLinuxDriverAPI.h"
// api udp
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/uio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

int dimensionDeEnvio = 1024;

void errorPrograma(char *msg) {
    perror(msg);
    exit(0);
}


int main(int argc, char ** argv)
{
    //variables de la captura
    int i;
    int adcFile;
    unsigned int valorLeido = 0;
    int res;
    int canalElegido;
    int dimensionLecturaRecomendada;
    int dimensionLecturaTotal;
    int paquetesLeidos = 0;
    char* bufferMM = 0;
    //variables UDP
    int sockfd, portno, n;
    int serverlen;
    char *hostname;
    struct sockaddr_in serveraddr;
    struct hostent *server;


    //COMPROBAR EL NUMERO DE PARAMETROS
    if(argc!=4){
        printf("Argumentos del programa invalidos\n");
        printf("Invocarlo como: rghpadctest <canal> <direccionDestino>  <puertoDestino>\n");
        exit(EXIT_FAILURE);
    }

    canalElegido = atoi(argv[1]);
    hostname = argv[2];
    portno = atoi(argv[3]);

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


    int sizeMask = (dimensionLecturaTotal-1);
    printf("mascaradeDatos : %x\n", sizeMask);
    bufferMM = mmap(0, dimensionLecturaTotal, PROT_READ, MAP_PRIVATE, adcFile, 0);





    printf("Se va a iniciar la captura\n");
    //printf("Revise mensajes del sistema durante 1 minuto\n");
    res = ioctl(adcFile, RGHPADC_INICIAR_CAPTURA, 0);
    if(res!=0){
        printf("Error iniciando la captura:  %d\n", res);
        exit(EXIT_FAILURE);
    }

    //ABRIR EL PUERTO UDP
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0){
            errorPrograma("ERROR abriendo el socket");
    }
    server = gethostbyname(hostname);
    if(server == NULL) {
        fprintf(stderr,"ERROR, no such host as %s\n", hostname);
        exit(0);
    }
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
      (char *)&serveraddr.sin_addr.s_addr, server->h_length);
    serveraddr.sin_port = htons(portno);
    serverlen = sizeof(serveraddr);


    int desplazamiento = 0;
    while(1){
        res = read(adcFile, 0, dimensionLecturaRecomendada);
        if(res<0){
            printf("Error leyend datos %d\n", res);
            break;
        }
        if(res!=0){
            paquetesLeidos++;
            printf("Paquetes leidos: %d\r\n", paquetesLeidos);
            char* nuevosDatos = bufferMM+desplazamiento;
            unsigned short* ultimosDatosLeidos = (unsigned short*)(nuevosDatos);
            unsigned short ultimoDatoLeido = *ultimosDatosLeidos;
            //printf("Primer dato del ultimo paquete leido: %x\r\n", (int)ultimoDatoLeido);
            //TRANSMITIR DATOS POR UDP
            char* datosAEnviar = nuevosDatos;
            int datosEnviados = 0;
            while(datosEnviados<dimensionLecturaRecomendada){
                sendto(
                    sockfd,
                    datosAEnviar,
                    dimensionDeEnvio,
                    0,
                    &serveraddr,
                    serverlen
                );
                datosAEnviar+=dimensionDeEnvio;
                datosEnviados+=dimensionDeEnvio;
            }



            desplazamiento+=dimensionLecturaRecomendada;
            desplazamiento&=sizeMask;
        }
        fflush(stdout);
        usleep(5000);
    }

    munmap(bufferMM, dimensionLecturaTotal);

    printf("Prueba finalizada\n");
    close(adcFile);
    exit(EXIT_SUCCESS);
}



