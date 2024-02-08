
#include <string.h>
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h>
#include <time.h>
#include "utility.h"

void manage_client_command(int sd, char* command1, char* command2, char* command3){
    if(command1 == NULL){
        strcpy(buffer, "Inseriti campi troppo lunghi o non validi, riprova!\n");
        return;
    }

    if(sd_is_in_session_list(sd)){
        aux = sd_is_in_session_list(sd);
        time(&rawtime);
        if(difftime(rawtime, aux->time) > 1800 && aux->player_role == MAIN_CHARACTER){
            you_lost(aux);
            return;
        }

    }

    if(strcmp(command1, "signup") == 0){
        if(command2 == NULL || command3 == NULL){
            strcpy(buffer, "Mancanti campi username e/o password, riprova!\n");
            return;
        }
        if(strlen(command2) > 7 || strlen(command3) > 7){
            strcpy(buffer, "Inseriti campi troppo lunghi o non validi, riprova!\n");
            return;
        }
        if(username_is_in_user_list(command2)){
            strcpy(buffer, "Username già utilizzato, riprova!\n");
            return;
        }
        else{
            printf("Registrazione in corso per %s...\n", command2);
            signup(command2, command3);
            return;
        }
    }

    else if(strcmp(command1, "login") == 0){
        if(command2 == NULL || command3 == NULL){
            strcpy(buffer, "Mancanti campi username e/o password, riprova!\n");
            return;
        }
        if(strlen(command2) > 7 || strlen(command3) > 7){
            strcpy(buffer, "Inseriti campi troppo lunghi o non validi, riprova!\n");
            return;
        }
        if(!username_is_in_user_list(command2)){
            if(command2 == NULL || command3 == NULL)
                strcpy(buffer, "Accesso fallito, mancano dei parametri!\n");
            else
                strcpy(buffer, "Username non registrato, riprova!\n");
            return;
        }
        else{
            if(username_is_in_session_list(command2))
                strcpy(buffer, "Utente già loggato!\n");
            else{
                printf("Accesso in corso per %s...\n", command2);
                login(sd, command2, command3);
            }
            return;
        }
    }

    else if(strcmp(command1, "start") == 0){
        if(sd_is_in_session_list(sd) == NULL){
            strcpy(buffer, "Comando non autorizzato, non sei loggato!\n");
            return;
        }

        if(command2== NULL || command3 != NULL){
            strcpy(buffer, "Hai sbagliato i parametri del comando!\n");
            return;
        }

        aux = sd_is_in_session_list(sd);
        if(aux->player_role != NOT_PLAYING || aux->escape_room != -1){
            strcpy(buffer, "Comando non autorizzato, stai già giocando!\n");
            return;
        }

        sscanf(command2, "%d", &x);
        enter_escape_room(x, aux);
    }

    else if(strcmp(command1, "protagonista") == 0){
        if(sd_is_in_session_list(sd) == NULL){
            strcpy(buffer, "Comando non autorizzato, non sei loggato!\n");
            return;
        }

        if(command2!= NULL || command3 != NULL){
            strcpy(buffer, "Questo comando non richiede altri parametri!\n");
            return;
        }

        aux = sd_is_in_session_list(sd);
        play_room_as_main_character(aux);
    }

    else if(strcmp(command1, "oracolo") == 0){
        if(sd_is_in_session_list(sd) == NULL){
            strcpy(buffer, "Comando non autorizzato, non sei loggato!\n");
            return;
        }

        if(command2!= NULL || command3 != NULL){
            strcpy(buffer, "Questo comando non richiede altri parametri!\n");
            return;
        }

        aux = sd_is_in_session_list(sd);
        if(players_in_a_room_as_main_character(aux->escape_room) > 0)
            play_room_as_oracle(aux); 
        else
            strcpy(buffer, "Non puoi giocare in modalità oracolo, non ci sono giocatori che stanno giocando nella tua stanza come protagonisti!\n");     
    }

    else if(strcmp(command1, "look") == 0){
        if(sd_is_in_session_list(sd) == NULL){
            strcpy(buffer, "Comando non autorizzato, non sei loggato!\n");
            return;
        }

        aux = sd_is_in_session_list(sd);
        if(aux->escape_room == -1){
            strcpy(buffer, "Comando non autorizzato, non stai giocando!\n");
            return;
        }

        handle_look_command(aux, command2, command3);
        if(aux->player_role == MAIN_CHARACTER)
            game_checkpoint(aux);
    }

    else if(strcmp(command1, "take") == 0){
        if(sd_is_in_session_list(sd) == NULL){
            strcpy(buffer, "Comando non autorizzato, non sei loggato!\n");
            return;
        }

        aux = sd_is_in_session_list(sd);
        if(aux->escape_room == -1){
            strcpy(buffer, "Comando non autorizzato, non stai giocando!\n");
            return;
        }

        if(command2 == NULL || command3 != NULL){
            strcpy(buffer, "Hai sbagliato i parametri del comando!\n");
            return;
        }

        if(aux->take_parameter != -1){
            if(!((aux->take_parameter == BAULE && strcmp(command2, "baule")==0) || (aux->take_parameter == SCRIGNO && strcmp(command2, "scrigno")==0) || (aux->take_parameter == TASTIERA && strcmp(command2, "tastiera") == 0))){
                strcpy(buffer, "Hai già un oggetto in mano, rilascialo prima di prenderne un altro!\n");
                return;
            }
        }

        handle_take_command(aux, command2);
        game_checkpoint(aux);
    }

    else if(strcmp(command1, "lay") == 0){
        if(sd_is_in_session_list(sd) == NULL){
            strcpy(buffer, "Comando non autorizzato, non sei loggato!\n");
            return;
        } 

        aux = sd_is_in_session_list(sd);
        if(aux->escape_room == -1){
            strcpy(buffer, "Comando non autorizzato, non stai giocando!\n");
            return;
        }

        if(command3 != NULL){
            strcpy(buffer, "Hai sbagliato i parametri del comando!\n");
            return;
        }

        if(aux->take_parameter == -1){
            strcpy(buffer, "Non hai nessuno oggetto in mano da posare!\n");
            return;
        }
        else{
            handle_lay_command(aux, command2);
        }
        game_checkpoint(aux);
    }

    else if(strcmp(command1, "orologio") == 0){
        if(sd_is_in_session_list(sd) == NULL){
            strcpy(buffer, "Comando non autorizzato, non sei loggato!\n");
            return;
        } 

        aux = sd_is_in_session_list(sd);
        if(aux->escape_room != 0){ //la stanza 0 è quella dell'indovinello dell'orologio
            strcpy(buffer, "Comando non autorizzato, non sei nella stanza corretta o non sei in nessuna stanza!\n");
            return;
        }

        if(aux->take_parameter != SCRIGNO){
            strcpy(buffer, "Devi avere lo scrigno in mano per risolvere questo indovinello!\n");
            return;
        }

        aux->unlocked_token[0] = UNLOCKED; //Ho sbloccato il primo token "10"
        aux->take_parameter = -1; //in questo caso l'oggetto è come se venisse automaticamente posato!
        strcpy(buffer, "Risposta corretta! Scrigno sbloccato, riutilizza il comando take sull'oggetto per collezionare il token!\n");
        game_checkpoint(aux);
    }

    else if(strcmp(command1, "vocali") == 0){
        if(sd_is_in_session_list(sd) == NULL){
            strcpy(buffer, "Comando non autorizzato, non sei loggato!\n");
            return;
        } 

        aux = sd_is_in_session_list(sd);
        if(aux->escape_room != 0){ //la stanza 0 è quella dell'indovinello dell'orologio
            strcpy(buffer, "Comando non autorizzato, non sei nella stanza corretta o non sei in nessuna stanza!\n");
            return;
        }

        if(aux->take_parameter != BAULE){
            strcpy(buffer, "Devi avere il baule in mano per risolvere questo indovinello!\n");
            return;
        }

        aux->unlocked_token[1] = UNLOCKED; //Ho sbloccato il secondo token "11"
        aux->take_parameter = -1; //in questo caso l'oggetto è come se venisse automaticamente posato!
        strcpy(buffer, "Risposta corretta! Baule sbloccato, riutilizza il comando take sull'oggetto per collezionare il token!\n");
        game_checkpoint(aux);
    }

    else if(strcmp(command1, "objs") == 0){
        if(sd_is_in_session_list(sd) == NULL){
            strcpy(buffer, "Comando non autorizzato, non sei loggato!\n");
            return;
        } 

        aux = sd_is_in_session_list(sd);
        if(aux->escape_room == -1){
            strcpy(buffer, "Comando non autorizzato, non stai giocando!\n");
            return;
        }

        if(command2 != NULL || command3 != NULL){
            strcpy(buffer, "Hai sbagliato i parametri del comando!\n");
            return;
        }

        handle_objs_command(aux);
    }

    else if(strcmp(command1, "100") == 0){
        if(sd_is_in_session_list(sd) == NULL){
            strcpy(buffer, "Comando non autorizzato, non sei loggato!\n");
            return;
        } 

        aux = sd_is_in_session_list(sd);
        if(aux->escape_room != 0){
            strcpy(buffer, "Comando non autorizzato, non sei nella stanza corretta o non stai giocando!\n");
            return;
        }

        if(aux->take_parameter != TASTIERA){
            strcpy(buffer, "Devi avere la tastiera in mano per risolvere questo indovinello!\n");
            return;
        }

        if(aux->unlocked_token[0] != COLLECTED || aux->unlocked_token[1] != COLLECTED){
            strcpy(buffer, "Devi raccogliere gli altri due token prima per risolvere questo indovinello!\n");
            return;
        }

        time(&rawtime);
        if(aux->unlocked_door != 0 && difftime(rawtime, aux->time) < 1500){
            strcpy(buffer, "Per rispondere all'indovinello della porta devi aspettare gli ultimi 5 minuti o sperare che un oracolo te la sblocchi prima!\n");
            return;
        }

        you_won(aux);
    }

    else if(username_is_in_session_list(command1)){
        aux = sd_is_in_session_list(sd);
        aux2 = username_is_in_session_list(command1);
        if(aux->player_role == ORACLE && aux2->player_role == MAIN_CHARACTER && aux->escape_room == aux2->escape_room){
            if(!aux2->unlocked_door){
                strcpy(buffer, "Questo utente è già stato aiutato!\n");
            }
            else{
                aux->unlocked_token[0] = MAGIC_WAND;
                aux->unlocked_token[1] = UNKNOWN;
                strcpy(aux->helped_username, command1);
                strcpy(buffer, "Hai ottenuto la bacchetta, usala attraverso il comando use per aiutare il tuo compagno!\n");
            }
        }
    }

    else if(strcmp(command1, "use") == 0){
        if(sd_is_in_session_list(sd) == NULL){
            strcpy(buffer, "Comando non autorizzato, non sei loggato!\n");
            return;
        } 

        aux = sd_is_in_session_list(sd);
        if(aux->escape_room != 0){
            strcpy(buffer, "Comando non autorizzato, non sei nella stanza corretta o non stai giocando!\n");
            return;
        }

        if(aux->player_role != ORACLE){
            strcpy(buffer, "Devi essere oracolo per usare questo comando!\n");
            return;
        }

        if(strcmp(command2, "bacchetta") != 0 || command3 != NULL){
            strcpy(buffer, "Parametri del comando sbagliati!\n");
            return;
        }

        if(aux->unlocked_token[0] != MAGIC_WAND){
            strcpy(buffer, "Non hai la bacchetta in mano, scegli prima un giocatore da aiutare!\n");
            return;
        }

        aux2 = username_is_in_session_list(aux->helped_username);
        aux2->unlocked_door = 0;
        strcpy(buffer, "MISSIONE PORTATA A TERMINE! Puoi rimanere in attesa per scoprire se il tuo compagno riuscirà a vincere usando il comando look!\n");
    }

    else{
        strcpy(buffer, "Comando o formato del comando sconosciuto, se è una risposta ad un enigma allora è sbagliata, riprova!\n");
    }
}
