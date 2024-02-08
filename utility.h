#define BUFFER_SIZE 32     //byte inviati dal client e ricevuti dal server
#define RESPONSE_SIZE 256  //byte inviati dal server e ricevuti dal client

#define NOT_PLAYING 0      //stati di un giocatore
#define MAIN_CHARACTER 1
#define ORACLE 2

#define SCRIGNO 1          //possibili oggetti presi in mano
#define BAULE 2
#define TASTIERA 3

#define NO_TOKEN 0         //stati dei token per il protagonista, memorizzati in unlocked_token[2]
#define LOCKED 0
#define UNLOCKED 1 
#define COLLECTED 2

#define MAGIC_WAND 3       //stati dei token per l'oracolo, memorizzati in unlocked_token[2]
#define WIN 4 
#define LOSE 5
#define UNKNOWN 6

#define ENIGMA01 0      //possibili stanze implementate (1 sola in questo progetto)

extern char buffer[RESPONSE_SIZE]; //buffer lato server
//struct di appoggio
extern struct session* aux;
extern struct session* aux2;
//variabili d'appoggio utili per gestire il tempo di gioco
extern int x;
extern time_t rawtime;

//memorizzazione utenti online
struct session{ 
    int sd;                 //chiave primaria, socket descriptor
    int player_role;        //possibili stati: NOT_PLAYING \ MAIN_CHARACTER \ ORACLE
    int escape_room;        //codice escape room
    int take_parameter;     //permette di sapere se un giocatore ha qualcosa in mano (MAX 1 oggetto per volta)
    int unlocked_door;      //permette di sapere se il giocatore protagonista può accedere all'enigma della porta
    char helped_username[7];//permette di memorizzare quale utente è stato aiutato da un oracolo
                            //coerente solo se player_role == ORACLE   
    time_t time;            //gestione tempo
    int unlocked_token[2];  //spiegato sopra
    struct session *next;   //lista
    struct user* my_user;   //permette di risalire a username e password di un utent online
};

//memorizzazione utenti registrati
struct user{
    char username[7];
    char password[7];
    struct user *next; //lista
};

struct session* sd_is_in_session_list(int sd);
struct user* username_is_in_user_list(char *username);
struct session* username_is_in_session_list(char* username);
void login(int sd, char *username, char* password);
void signup(char *username, char* password);
void enter_escape_room(int escape_room, struct session* aux);
void play_room_as_main_character(struct session* aux);
void play_room_as_oracle(struct session* aux);
void handle_look_command(struct session* aux, char* obj1, char* obj2);
void handle_take_command(struct session* aux, char* obj1);
void handle_lay_command(struct session* aux, char* obj2);
void handle_objs_command(struct session* aux);
void answer_to_oracle(int value, struct session* aux);
void you_won(struct session* aux);
void you_lost(struct session* aux);
void print_session_list();
void print_user_list();
void delete_session_user(int sd);
void delete_users();
int players_in_a_room_as_main_character(int escape_room);
void game_checkpoint(struct session* aux);
void manage_client_command(int sd, char* command1, char* command2, char* command3);