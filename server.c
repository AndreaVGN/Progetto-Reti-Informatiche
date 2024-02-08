#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/select.h>
#include "utility.h"

int main(int argc, char* argv[]){
    //variabili per la connessione e la gestione dei socket descriptor
    int ret, newfd, listener, addrlen, i, port, sock_number, fdmax;

    fd_set master;
    fd_set read_fds;

    struct sockaddr_in my_addr, cl_addr;

    //per come ho organizzato il progetto, un comando può essere al massimo un insieme di tre "parole"
    //command [obj1] [obj2]

    char* command1_buffer;
    char* command2_buffer;
    char* command3_buffer;

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
    listener = socket(AF_INET, SOCK_STREAM, 0);

    memset(&my_addr, 0, sizeof(my_addr)); //Pulizia 
    my_addr.sin_family = AF_INET; 
    my_addr.sin_port = htons(port);
    my_addr.sin_addr.s_addr = INADDR_ANY;
    ret = bind(listener, (struct sockaddr*)&my_addr, sizeof(my_addr));

    if(ret < 0){
        perror("Errore bind!");
        exit(1);
    } 

    // ipotizzo al massimo una coda di 10 richieste
    ret = listen(listener, 10);

    if(ret < 0){
        perror("Errore listen!");
        exit(1);
    }
    // **FINE GESTIONE CONNESSIONE**

    // ***INIZIALIZZAZIONI PER GESTIONE SELECT***

    FD_ZERO(&master);
    FD_ZERO(&read_fds);

    FD_SET(listener, &master);

    //monitoro anche lo stdin (necessario per impartire comandi direttamente al server)
    FD_SET(STDIN_FILENO, &master);

    fdmax = listener;
    sock_number = 1;

    // ***FINE INIZIALIZZAZIONI GESTIONE SELECT***

    printf("***************************** SERVER STARTED *****************************\n\n");
    printf("Server di gioco avviato correttamente sulla porta %d\n\n", port);
    printf("Digita un comando: \n\n");
    printf("-- stop --> termina il server\n");
    printf("**************************************************************************\n\n");

    while(1){
        read_fds = master;

        ret = select(fdmax + 1, &read_fds, NULL, NULL, NULL);

        if(ret < 0){
            perror("Errore select!");
            exit(1);
        }

        for(i = 0; i <= fdmax; i++){
            if(FD_ISSET(i, &read_fds)){
                if(i == listener){
                    printf("Nuovo client rilevato!\n");
                    addrlen = sizeof(cl_addr);
                    newfd = accept(listener, (struct sockaddr *)&cl_addr, (socklen_t *)&addrlen);

                    sock_number++; //nuovo client monitorato

                    FD_SET(newfd, &master);

                    if(newfd > fdmax){
                        fdmax = newfd;
                    }
                }
                else if(i == STDIN_FILENO){ //monitoro lo stdin
                    fgets(buffer, BUFFER_SIZE, stdin);
                    if(strncmp(buffer, "stop", 4)==0){ //se il comando inserito al server è stop
                        if(sock_number == 1){          //se non ho più utenti collegati, quindi l'unico rimasto è il listener
                            printf("Chiusura server di gioco...\n");
                            //printf("Chiudo il listener\n");
                            delete_users(); //dealloco tutte le strutture dati relativi alle registrazioni (ho deciso di non salvarle su file)
                            close(listener);
                            exit(0);
                        }
                        else{
                            printf("Ci sono ancora utenti collegati, il server non si può chiudere!\n");
                        }
                    }
                    else{
                        printf("Comando inserito non valido!\n");
                    }
                    //printf("Input da stdin: %s", buffer); //utile per debug
                }
                else{ //se non è listener
                    ret = recv(i, (void*)buffer, BUFFER_SIZE, 0);
                    if(ret == 0){ //il server capisce che il client è terminato perché riceve 0 dalla recv
                        printf("Chiusura client rilevata\n");
                        delete_session_user(i); //dealloco la sessione relativa all'utente del socket "i"
                        //il client ha chiuso il socket, procedo a chiudere il socket connesso col server
                        close(i);
                        //rimuovo il descrittore newfd da quelli da monitorare 
                        FD_CLR(i, &master);
                        //decremento il numero di socket monitorati
                        sock_number--;
                    }
                    else if(ret < 0){
                        perror("Errore nella ricezione!\n");
                        close(i);
                         //rimuovo il descrittore newfd da quelli da monitorare 
                        FD_CLR(i, &master);
                    }
                    else{
                        printf("Nuovo comando rilevato!\n");
                        printf("Ho ricevuto: %s\n", buffer); //utile per descrivere le azioni del server
                        //divido il comando inviato dal client
                        //nel peggiore dei casi:
                        //command obj1 obj2
                        command1_buffer = strtok(buffer, " ");
                        command2_buffer = strtok(NULL, " ");
                        command3_buffer = strtok(NULL, " ");
                        printf("command [obj1] [obj2]:\n");               
                        printf("%s\n%s\n%s\n", command1_buffer, command2_buffer, command3_buffer);
                        //gestiamo il comando inviato dal client 
                        manage_client_command(i, command1_buffer, command2_buffer, command3_buffer);
                        //invio la risposta al client
                        ret = send(i, buffer, RESPONSE_SIZE, 0);                      
                    }
                }
            }
        }
    }
}