#include <string.h>
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h>
#include <time.h>
#include "utility.h"

char buffer[RESPONSE_SIZE]; //buffer lato server
//struct di appoggio
struct session* aux;
struct session* aux2;
//variabili di appoggio per gestire il tempo
int x;
time_t rawtime;

//testa della lista user
struct user* user_list = NULL;
//testa della lista session
struct session* session_list = NULL;
//buffer ausiliario di appoggio
char aux_buffer[20];

//dato un sd, restituisce puntatorre a struct session se esiste, NULL altrimenti.
struct session* sd_is_in_session_list(int sd){
    struct session *aux = session_list;
    while(aux){
        if(aux->sd == sd)
            break;
        aux = aux->next;
    }
    return aux;
}

//dato un username, restituisce puntatorre a struct session corrispondente se esiste, NULL altrimenti.
//NB! anche username è univoco
struct session* username_is_in_session_list(char* username){
    struct session *aux = session_list;
    while(aux){
        if(strcmp(aux->my_user->username, username) == 0)
            break;
        aux = aux->next;
    }
    return aux;
}

//dato un username, restituisce puntatorre a struct user se esiste, NULL altrimenti.
//NB! anche username è univoco
struct user* username_is_in_user_list(char *username){
    struct user *aux = user_list;
    if(username == NULL)
        return NULL;
    while(aux){
        if(strcmp(aux->username, username)==0)
            break;
        aux = aux->next;
    }
    return aux;
}


void login(int sd, char *username, char* password){
    struct user *elem = username_is_in_user_list(username);
    struct session *new_elem;

    if(username == NULL || password == NULL){
        strcpy(buffer, "Accesso fallito, mancano dei parametri!\n");
        return;
    }

    if(strcmp(elem->username, username) == 0 && strcmp(elem->password, password) == 0){
        //allocazione memoria per la nuova sessione
        new_elem = malloc(sizeof(struct session));
        new_elem->sd = sd;
        new_elem->player_role = NOT_PLAYING;
        new_elem->unlocked_token[0] = NO_TOKEN;
        new_elem->unlocked_token[1] = NO_TOKEN;
        new_elem->my_user = elem;
        new_elem->escape_room = -1; //NO ESCAPE ROOM
        new_elem->take_parameter = -1; //NESSUN OGGETTO TENUTO IN MANO
        new_elem->unlocked_door = 1; //PORTA BLOCCATA
        strcpy(new_elem->helped_username, ""); //DEFAULT
        //inserimento in lista session_list
        if(session_list){
            new_elem->next = session_list->next;
            session_list->next = new_elem;
        }
        else
            session_list = new_elem;
        //strcpy(buffer, "Login avvenuto con successo!\n"); //utile per debug
        strcpy(buffer, "\n***************************** ESCAPE ROOM *****************************\n\nUtilizza il comando start ed il codice di una stanza disponibile!\n\n0)--> ENIGMA01\n***********************************************************************\n");
    }
    else
        strcpy(buffer, "Password non corretta, riprova!\n");
}

void signup(char *username, char* password){
    struct user *new_elem;
    if(username == NULL || password == NULL){
        strcpy(buffer, "Registrazione fallita, mancano dei parametri!\n");
        return;
    }
    //allocazione memoria per la registrazione del nuovo utente
    new_elem = malloc(sizeof(struct user));
    new_elem->next = NULL;
    strcpy(new_elem->username, username);
    strcpy(new_elem->password, password);
    //inserimento lista degli utenti
    if(user_list){
        new_elem->next = user_list->next;
        user_list->next = new_elem;
    }
    else
        user_list = new_elem;
    strcpy(buffer, "Registrazione avvenuta con successo, adesso puoi loggarti!\n");
}

//permette di selezionare una stanza e in seguito viene chiesto la modalità di gioco
void enter_escape_room(int escape_room, struct session* aux){
    if(escape_room != ENIGMA01){
        strcpy(buffer, "Room inserita non valida!\n");
        return;
    } //al momento è presente una sola escape room all'interno dell'applicazione
    aux->escape_room = escape_room;
    strcpy(buffer, "Seleziona la modalità di gioco\n\n-- protagonista \n-- oracolo\n");
}

void play_room_as_main_character(struct session* aux){
    aux->player_role = MAIN_CHARACTER;
    //settiamo il tempo
    time(&rawtime);
    aux->time = rawtime;
    strcpy(buffer, "Sei entrato nella stanza! Buon divertimento, a tuo rischio e pericolo!\n");
}

void play_room_as_oracle(struct session* aux){
    struct session* aux2 = session_list;
    aux->player_role = ORACLE;
    strcpy(buffer, "Scegli di quale giocatore essere l'oracolo: \n"); //cerco utenti protagonisti che stiano giocando nella stessa stanza
    while(aux2){
        if(aux2->escape_room == aux->escape_room && aux2->player_role == MAIN_CHARACTER){
            strcat(buffer, "- ");
            strcat(buffer, aux2->my_user->username);
            strcat(buffer, "\n");
        }
        aux2 = aux2->next;
    }
}

void handle_look_command(struct session* aux, char* obj1, char* obj2){
    if(aux->escape_room == ENIGMA01){
        if(aux->player_role == ORACLE && strcmp(aux->helped_username, "") != 0){
            if(aux->unlocked_token[1] == WIN){
                strcpy(buffer, "Complimenti! Il tuo compagno ha vinto!\nEsci dal gioco e poi rientra se vuoi iniziare una nuova partita!\n");
            }
            else if(aux->unlocked_token[1] == LOSE){
                strcpy(buffer, "Nulla ha potuto il tuo aiuto! Il tuo compagno ha perso!\nEsci dal gioco e poi rientra se vuoi iniziare una nuova partita!\n");
            }
            if(aux->unlocked_token[1] == UNKNOWN){
                strcpy(buffer, "Nulla si può ancora dire sul destino del tuo compagno!\n");
            }
        }
        else if(obj1 == NULL && obj2 == NULL){
            strcpy(buffer, "Ti trovi in una stanza buia... davanti a te vedi un tavolo con sopra uno **scrigno** e un piccolo **baule**, alla tua destra vedi una ++porta++.");
            return;
        }
        else if(strcmp(obj1, "porta") == 0 && obj2 == NULL){
            strcpy(buffer, "Questa porta è bloccata. Devi riuscire ad aprirla per uscire. La porta si apre con un codice, lo devi inserire nella **tastiera** attaccata alla porta.");
            return;
        }
        else if(strcmp(obj1, "scrigno") == 0 && obj2 == NULL){
            strcpy(buffer, "Questo scrigno sembra si apra in qualche modo...");
        }
        else if(strcmp(obj1, "baule") == 0 && obj2 == NULL){
            strcpy(buffer, "Questo baule sembra si apra in qualche modo...");
        }
        else if(strcmp(obj1, "tastiera") == 0 && obj2 == NULL){
            strcpy(buffer, "Sembra una vecchia e polverosa tastiera analogica...");
        }
        else{
            strcpy(buffer, "La sintassi del comando look che hai inserito non è corretta!\n");
        }
    }
}

void handle_take_command(struct session* aux, char* obj1){
    if(aux->escape_room == ENIGMA01){ //se sono nella escape room 0
        if(strcmp(obj1, "scrigno") == 0){
            if(aux->unlocked_token[0] == LOCKED){
                strcpy(buffer, "Per aprire questo scrigno devi risolvere un enigma... 'Se guardi i numeri che ho sulla faccia noterai che del 13 non vi è traccia. Cosa sono?'");
                aux->take_parameter = SCRIGNO;
            }
            else if(aux->unlocked_token[0] == UNLOCKED){
                strcpy(buffer, "Hai raccolto questo token!\n");
                aux->unlocked_token[0] = COLLECTED;
            }
            else if(aux->unlocked_token[0] == COLLECTED){
                strcpy(buffer, "Hai già raccolto il token di questo oggetto!\n");
                aux->unlocked_token[0] = COLLECTED;
            }
        }
        else if(strcmp(obj1, "baule") == 0){
            if(aux->unlocked_token[1] == LOCKED){
                strcpy(buffer, "Per aprire questo baule devi risolvere un enigma... 'Siamo piccole cose di uso quotidiano e ci trovi tutte in UN CAMPO DA TENNIS'. Cosa siamo?'");
                aux->take_parameter = BAULE;
            }
            else if(aux->unlocked_token[1] == UNLOCKED){
                strcpy(buffer, "Hai raccolto questo token!\n");
                aux->unlocked_token[1] = COLLECTED;
            }
            else if(aux->unlocked_token[1] == COLLECTED){
                strcpy(buffer, "Hai già raccolto il token di questo oggetto!\n");
                aux->unlocked_token[0] = COLLECTED;
            }
        }  
        else if(strcmp(obj1, "tastiera") == 0){
            time(&rawtime);
            if(aux->unlocked_token[0] != COLLECTED && aux->unlocked_token[1] != COLLECTED){
                strcpy(buffer, "Per accedere all'enigma finale devi prima sbloccare tutti i token!\n");
                aux->take_parameter = TASTIERA;
                return;
            }
            else if(!aux->unlocked_door){
                strcpy(buffer, "Un oracolo ti ha aiutato, puoi accedere all'indovinello in anticipo... Devi inserire 3 cifre per sbloccare la porta e vincere, quali inserisci?");
                aux->take_parameter = TASTIERA;
            }             
            else if(difftime(rawtime, aux->time) > 1500){ //se sono passati almeno 25 minuti
                strcpy(buffer, "Il tempo necessario per accedere all'indovinello è trascorso... Devi inserire 3 cifre per sbloccare la porta e vincere, quali inserisci?");
                aux->take_parameter = TASTIERA;
            }
            else{
                strcpy(buffer, "Il tempo necessario per accedere a questo oggetto non è ancora trascorso o nessun oracolo te lo ha ancora sbloccato!\n");
                aux->take_parameter = TASTIERA;
            }
        }
        else{
            strcpy(buffer, "L'oggetto che vuoi prendere in mano non esiste!\n");
        }
    }
}

void handle_lay_command(struct session* aux, char* obj2){
    if(aux->escape_room == ENIGMA01){
        if(obj2 == NULL){
            if(aux->take_parameter == -1)
                strcpy(buffer, "Non hai oggetti in mano da posare!\n");
            else
                strcpy(buffer, "Hai posato l'oggetto che avevi in mano!\n");
            aux->take_parameter = -1;
            return;
        }
        else if(strcmp(obj2, "scrigno") !=0 && strcmp(obj2, "baule") !=0 && strcmp(obj2, "tastiera") !=0){
            strcpy(buffer, "Sintassi errata, non puoi posare un oggetto che non esiste!\n");
        }
        else if(((aux->take_parameter == BAULE && strcmp(obj2, "baule")==0) || (aux->take_parameter == SCRIGNO && strcmp(obj2, "scrigno")==0) || (aux->take_parameter == TASTIERA && strcmp(obj2, "tastiera") == 0))){
            aux->take_parameter = -1;
            strcpy(buffer, "Hai posato l'oggetto che avevi in mano!\n");
        }
        else{
            strcpy(buffer, "Non hai questo oggetto in mano al momento!\n"); 
        }
    }
}

void handle_objs_command(struct session* aux){
    if(aux->escape_room == ENIGMA01){
        strcpy(buffer, "");
        strcat(buffer, "Oggetto in mano: \n");
        if(aux->take_parameter == SCRIGNO){
            strcat(buffer, "- scrigno\n");
        }
        if(aux->take_parameter == BAULE){
            strcat(buffer, "- baule\n");
        }        
        if(aux->take_parameter == TASTIERA){
            strcat(buffer, "- tastiera\n");
        }
        strcat(buffer, "Token in mano: ");
        if(aux->unlocked_token[0] == COLLECTED){
            strcat(buffer, "\n- token 11");
        }
        if(aux->unlocked_token[0] == MAGIC_WAND){
            strcat(buffer, "\n- bacchetta");
        }
        if(aux->unlocked_token[1] == COLLECTED){
            strcat(buffer, "\n- token 10");
        }
        strcat(buffer, "\n");
    }
}

void answer_to_oracle(int value, struct session* aux){ //informa l'oracolo del risultato del giocatore che ha aiutato
    struct session* aux2 = session_list;
    while(aux2){
        if(strcmp(aux2->helped_username, aux->my_user->username) == 0)
            aux2->unlocked_token[1] = value == 1 ? WIN : LOSE;
        aux2 = aux2->next;
    }
}

void you_won(struct session* aux){
    aux->player_role = NOT_PLAYING;
    aux->unlocked_token[0] = NO_TOKEN;
    aux->unlocked_token[1] = NO_TOKEN;
    aux->escape_room = -1;
    aux->take_parameter = -1;
    aux->unlocked_door = 1;
    strcpy(aux->helped_username, "");
    answer_to_oracle(1, aux);
    strcpy(buffer, "COMPLIMENTI, HAI VINTO!\n\nSe vuoi, rigiochiamo insieme!\n\n***************************** ESCAPE ROOM *****************************\n\nSeleziona il codice di una stanza disponibile!\n\n0)--> ENIGMA01\n");
}

void you_lost(struct session* aux){
    aux->player_role = NOT_PLAYING;
    aux->unlocked_token[0] = NO_TOKEN;
    aux->unlocked_token[1] = NO_TOKEN;
    aux->escape_room = -1;
    aux->take_parameter = -1;
    aux->unlocked_door = 1;
    strcpy(aux->helped_username, "");
    answer_to_oracle(0, aux);
    strcpy(buffer, "TEMPO SCADUTO, HAI PERSO!\n\nSe vuoi, rigiochiamo insieme!\n\n***************************** ESCAPE ROOM *****************************\n\nSeleziona il codice di una stanza disponibile!\n\n0)--> ENIGMA01\n");
}

void print_session_list(){
    struct session* aux2 = session_list;
    while(aux2){
        printf("\nSession_list\n");
        printf("sd: %d\n", aux2->sd);
        printf("username: %s\n", aux2->my_user->username);
        aux2 = aux2->next;
    }
}

void print_user_list(){
    struct user* aux2 = user_list;
    while(aux2){
        printf("\nUser_list\n");
        printf("username: %s\n", aux2->username);
        printf("password: %s\n", aux2->password);
        aux2 = aux2->next;
    }
}

void delete_session_user(int sd){
    struct session* aux = sd_is_in_session_list(sd);
    struct session* aux2 = session_list;
    if(aux){
        //prima rimuovo dalla lista session
        if(aux == session_list){
            session_list = session_list->next;
            free(aux);
            //print_session_list();
            return;
        }
        while(aux2){
            if(aux2->next == aux){
                aux2->next = aux->next;
                break;
            }
            aux2 = aux2->next;
        }
        free(aux);
    }
    //print_session_list();
}

void delete_users(){
    struct user* aux2;
    while(user_list){
        //printf("A questa iterazione sono rimasti i seguenti utenti registrati\n");
        //print_user_list(); //utile per debug
        aux2 = user_list;
        user_list = user_list->next;
        free(aux2);
    } 
}

int players_in_a_room_as_main_character(int escape_room){
    int pl_number = 0;
    struct session* aux2 = session_list;
    while(aux2){
        if(aux2->escape_room == escape_room && aux2->player_role == MAIN_CHARACTER){
            pl_number++;
        }
        aux2 = aux2->next;
    }
    printf("numero: %d\n", pl_number);
    return pl_number;
}

void game_checkpoint(struct session* aux){ //informo il giocatore della situazione sull'istanza di gioco
    double difftimeFLOAT;
    int difftimeInt;
    if(aux->escape_room == ENIGMA01){
        strcat(buffer, "\n\n**ENIGMA01**\nTEMPO RIMANENTE: ");
        time(&rawtime);
        difftimeFLOAT = difftime(rawtime, aux->time);
        difftimeInt = (int)difftimeFLOAT;
        difftimeInt = 30 - difftimeInt/60;
        sprintf(aux_buffer, "%d", difftimeInt);
        strcat(buffer, aux_buffer);
        strcat(buffer, " minuti");
        strcat(buffer, " TOKEN TOTALI DEL GIOCO: 2\nTOKEN RACCOLTI: \n");
        if(aux->unlocked_token[0] == COLLECTED)
            strcat(buffer, "- token 11\n");
        if(aux->unlocked_token[1] == COLLECTED)
            strcat(buffer, "- token 10\n"); 
    }   
}

