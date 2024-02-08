#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define BUFFER_SIZE 32
#define RESPONSE_SIZE 256

int main(int argc, char* argv[]){

    //variabili per la connessione
    int ret, sd, port; 
    struct sockaddr_in srv_addr;

    
    char buffer[RESPONSE_SIZE]; //buffer lato client, invia 32 byte e riceve 256 byte
                                //come da protocollo

    //gestione numero di porta inserita da comando, errore se assente
    if(argc > 1)
        port = atoi(argv[1]);
    else{
        printf("Non hai specificato una porta!\n");
        exit(1);
    }

    //gestione porta direttamente da codice
    port = 4242;

    // **GESTIONE CONNESSIONE**
    sd = socket(AF_INET, SOCK_STREAM, 0);

    memset(&srv_addr, 0, sizeof(srv_addr)); 
    srv_addr.sin_family = AF_INET;  
    srv_addr.sin_port = htons(port);
    inet_pton(AF_INET, "127.0.0.1", &srv_addr.sin_addr);

    ret = connect(sd, (struct sockaddr*)&srv_addr, sizeof(srv_addr));

    if(ret < 0){
        printf("Errore connessione!");
        exit(1);
    }
    // **FINE GESTIONE CONNESSIONE**

    printf("****************************** BENVENUTO/A! ******************************\n\n");
    printf("Digita uno dei seguenti comandi per iniziare!\n\n");
    printf("-- login [username] [password]--> accedi\n");
    printf("-- signup [username] [password]--> accedi per la prima volta\n");
    printf("-- end --> esci\n");
    printf("NB! Username e Password devono essere al massimo di 7 caratteri!\n");
    printf("**************************************************************************\n\n");


    for(;;){
        fgets(buffer, BUFFER_SIZE, stdin);
        strtok(buffer, "\n"); //tolgo lo \n che nella fgets viene considerato
        strncat(buffer, " ", 1); //aggiungo uno spazio infondo per discriminare
                                 //il terzo parametro del comando
        if(strcmp(buffer, "end ") == 0){ //unico comando gestito lato client
                                         //il server si accorge della terminazione 
                                         //vedendo restituito 0 nella recv
            printf("Già te ne vai? Peccato...\n");
            printf("Arrivederci!\n");
            exit(0);
        }

        ret = send(sd, buffer, BUFFER_SIZE, 0);
        //printf("Ho inviato: %s\n", buffer); //utile per debug
        printf("\n"); //utile per indentazione output
        if(ret < 0){
            perror("Errore in fase di invio comando: \n");
            exit(1);
        }
    
        //pulisco il buffer prima di ricevere qualcosa di nuovo
        memset(&buffer, 0, sizeof(buffer));
        //Attendo risposta 
        ret = recv(sd, (void*)buffer, RESPONSE_SIZE, 0);
        //printf("Ho ricevuto: %s\n", buffer); //utile per debug
        printf("%s\n", buffer); //stampo risposta server

        if(ret < 0){
            perror("Errore in fase di ricezione\n");
            exit(1);
        }

        //pulisco il buffer prima di inviare qualcosa di nuovo
        memset(&buffer, 0, sizeof(buffer));
    }    
}