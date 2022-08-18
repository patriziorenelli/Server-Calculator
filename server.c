#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <math.h>
#include <stdbool.h>
#include <time.h>
#include <arpa/inet.h>


// Lunghezza massima della coda di connessioni in sospeso 
#define BACKLOG 10
// Lunghezza massima della stringa ricevuta in input dal client 
#define MAX_LEN_INPUT 40
// Lunghezza massima della stringa inviata al client come risultato dell'operazione
#define MAX_LEN_RESULT 180
// Lunghezza massima dei timestamp da inviare al client
#define DIM_TIMESTAMP 30
// Lunghezza massima della stringa che verrà salvata nel file di log 
#define DIM_OUTPUT 220

// Stringa ricevuta dall'utente 
char stringa[MAX_LEN_INPUT];
// Variabile usata per accertarsi che il client non sia terminato
int n;
// Variabile usata per consentire o bloccare la scrittura sul file 
bool accesso_file = true;
// Puntatore al file di log 
FILE *logFile;

// Struttura per passare dati ai thread 
typedef struct pthread_arg_t {
    int new_socket_fd;
    struct sockaddr_in client_address;
    /* TODO: Put arguments passed to threads here. See lines 116 and 139. */
} pthread_arg_t;

// Routine che verrà eseguita dai vari thread per soddisfare le richieste del cliente 
void *esecuzione_server(void *arg);

void signal_handler(int signal_number);

int main(int argc, char *argv[]) { 
    // Numero di porta, socket descriptor e socket descriptor ausiliario 
    int port, socket_fd, new_socket_fd;
    struct sockaddr_in address;
    pthread_attr_t pthread_attr;
    pthread_arg_t *pthread_arg;
    pthread_t pthread;
    socklen_t client_address_len;

    // Apertura del file 
    logFile = fopen("Log","a" );
    // Imposto la porta su cui creare il socket 
    port = 8082;

    //Inizializzo la memoria di address 
    memset(&address, 0, sizeof address);
    // Imposto che voglio usare IPv4 a livello di rete 
    address.sin_family = AF_INET;
    // Imposto la porta 
    address.sin_port = htons(port);
    // Associo la socket a tutte le interfacce locali 
    address.sin_addr.s_addr = INADDR_ANY;
  


    // Creo la socket TCP
    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Errore nella creazione della socket");
        exit(1);
    }

    // Associo l'indirizzo ip di address alla socket_fd 
    if (bind(socket_fd, (struct sockaddr *)&address, sizeof address) == -1) {
        perror("Errore bind");
        exit(1);
    }

    // Metto la socket_fd in ascolto aspettando le richieste di connessioni 
    if (listen(socket_fd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }
    
    
    // Gestori dei seganli ai vari segnali -> se ritornano SIG_ERR significa che c'è stato un errore


    /*Segnale generato in caso si cerca di scrivere su una socket non più esistente 
    o che non permette la scrittura, con SIG_IGN indico di ignorare il segnale SIG_PIPE */ 
    if(signal(SIGPIPE,SIG_IGN) == SIG_ERR){
        exit(1);
    }
    /* Segnale di terminazione 
    -> viene eseguito signal_handler quando ricevo SIGTERM */
    if(signal(SIGTERM, signal_handler) == SIG_ERR){
        exit(1);
    }
    /* Segnale di interruzione generato dall'utente dell'applicazione 
    -> viene eseguito signal_handler quando ricevo SIGINT */
    if(signal(SIGINT, signal_handler) == SIG_ERR){
        exit(1);
    }
    
    

    //Inizializzo gli attributi di pthread_attr con i loro valori di default
    if (pthread_attr_init(&pthread_attr) != 0) {
        perror("pthread_attr_init");
        exit(1);
    }
    // Indico che voglio creare thread distaccati 
    if (pthread_attr_setdetachstate(&pthread_attr, PTHREAD_CREATE_DETACHED) != 0) {
        perror("pthread_attr_setdetachstate");
        exit(1);
    }



    // Inizio ciclo infinito per ricevere e servire le richieste 
    while (1) {
	// Alloco la memoria per pthread_arg
        pthread_arg = (pthread_arg_t *)malloc(sizeof *pthread_arg);
        // Controllo che l'allocazione sia andata a buon fine 
        if (!pthread_arg) {
            perror("Errore nell'allocare la memoria");
            continue;
        }

        // Imposto la lunghezza possibile dell'indirizzo del client 
        client_address_len = sizeof pthread_arg->client_address;
        /* Si estrae la prima richiesta di connessione dalla coda e
          restituisce il descrittore del socket che salvo in socket_fd */
        new_socket_fd = accept(socket_fd, (struct sockaddr *)&pthread_arg->client_address, &client_address_len);
        // Controllo se la connessione sia andata a buon fine 
        if (new_socket_fd == -1) {
            perror("Connessione non accettata");
            free(pthread_arg);
            continue;
        }

        //Imposto gli argomenti del thread 
        pthread_arg->new_socket_fd = new_socket_fd;

        /* Creo il thread che servirà la connessione del socket eseguendo la routine esecuzione_Server
           Controllo che la creazione sia andata a buon fine */
        if (pthread_create(&pthread, &pthread_attr, esecuzione_server, (void *)pthread_arg) != 0) {
            perror("Errore nella creazione del Thread");
            free(pthread_arg);
            continue;
        }


        // Si aspetta che tutti i thread terminino 
        pthread_join(pthread, NULL);

    }

    return 0;
}

/* Routine eseguita dai vari thread del server che manipolano la stringa ricevuta dall'utente, 
   calcolano l'operazione, salvano il risultato dell'operazione con l'identificativo dell'utente e i timestamp
   sul file di log e spediscono al client il risultato dell'operazione con i timestamp */
void *esecuzione_server(void *arg) {

    // COMMENTARE QUI 
    // Creo struttura e copio i valori
    pthread_arg_t *pthread_arg = (pthread_arg_t *)arg;
    // Assegno il valore del socket descriptor 
    int new_socket_fd = pthread_arg->new_socket_fd;
    // Copio i dati sul client 
    struct sockaddr_in client_address = pthread_arg->client_address;

    // Rilascio la memoria allocata per arg
    free(arg);

  
    
    // I due numeri ricevuti dall'utente client -> sono stati usati double per maggiore precisione 
    double numero1 = 0.0;
    double numero2 = 0.0;
    // Risultato dell'operazione
    double risultato = 0.0;
    // Puntatore usato per contenere poi il risultato della funzione strtok per separare la stringa 
    char *stringa_conversione;
    
    // Risultato da inviare al client 
    char risultato_stringa[MAX_LEN_RESULT];

    // Timestamp ricezione
    char timestamp_ricezione[DIM_TIMESTAMP];
    // Timestamp terminazione
    char timestamp_fine[DIM_TIMESTAMP];
    // Output che va salvato nel file di log 
    char output_log[DIM_OUTPUT];

    // Carattere che indica l'operazione 
    char op;
   
      	
    /* Puntatore usato dalla funzione strtok per capire dove interrompere la conversione 
    ->   punta al primo carattere che non è stato convertito*/
    char *eptr;
    
    // Tipo di dato usato per le costanti per indicare quale orologio di sistema si usa 
    clockid_t c;
    // Struttura formata da  time_t   tv_sec e long     tv_nsec che indicano i secondi e nanosecondi 
    struct timespec t;
    
    
    // Inizializzo la stringa che conterrà la stringa ricevuta dal client 
    bzero(stringa, 30);
    
    //Inizio ciclo infinito che sarà terminato o dall'utente o se il client viene terminato
    while(1){


	    // Si attende finchè non si riceve la stringa dal client 
	    n = recv(new_socket_fd, stringa, MAX_LEN_INPUT, 0);
	    // Si ottiene il timestamp attuale e lo si salva in t 
	    clock_gettime(c,  &t);
	    // Si converte il timestamp salvato in una stringa e lo si salva in timestamp_ricezione 
	    sprintf( timestamp_ricezione,  "%.6ld.%ld", (long) t.tv_sec, t.tv_nsec ); 

	    /* 	Si controlla se il client stia ancora trasmettendo,
	     se  la recv ritorna 0 allora significa che il client sarà terminato poichè socket terminato
	     Non si termina il processo server ma soltanto il thread che serviva il processo client che è stato terminato
	    */
	    if (n<=0){
            // Elimino il thread del server 
	    	pthread_cancel( pthread_self());
		
	    }else{
	      	// Fuzione strtok serve per fare lo slicing della stringa su una stringa specifica come per noi ','
	      	stringa_conversione = strtok(stringa, ",");
		
	    	// Salvo il simbolo dell'operazione in modo da poterlo usare per lo switch
	      	op = stringa_conversione[0]; 
	      	// si manda avanti lo slicing della stringa sul carattere di separazione
	      	stringa_conversione = strtok(NULL, ",");
	      	//Converto e salvo il valore del primo operando 
	      	numero1 = strtod(stringa_conversione, &eptr);
	      	stringa_conversione = strtok(NULL, ",");
	      	// Converto e salvo il valore del secondo operando
	      	numero2 = strtod(stringa_conversione, &eptr);
	   
	      	// Switch per scegliere quale operazione eseguire 
	      	switch(op){
	      		case '+':
	      		risultato = numero1 + numero2;
	      		break;
	      		
	      		case '-':
	      		risultato = numero1 - numero2;
	      		break;
	      		
	      		case '*':
	      		risultato = numero1 * numero2;
	      		break;
	      		
	      		case '/':
	      		// Controllare che non si stiano facendo le operazioni n/0 0/0
	      		if( (numero2 == 0.0 && numero1 != 0.0) || ( numero2 == 0.0 && numero1 == 0.0 ) ){
	      			risultato = INFINITY;
	      		}else{
	      			risultato = numero1 / numero2;
	      		}
	      		break;
	      		
	      	}
	      	
	      	/* Converto il risultato che è in formato double in una stringa e salvo il valore in
	      	 risultato_stringa */
	      	sprintf(risultato_stringa, "%f", risultato);
	      	
		    // Anche con questa precisione penso dia sempre 0 
		    clock_gettime(c, &t);
		    // Si converte il timestamp salvato in una stringa e lo si salva in timestamp_fine 
	        sprintf( timestamp_fine,  "%.6ld.%ld", (long) t.tv_sec, t.tv_nsec );  
	    	// Aspetto che il file di log sia disponibile 
		    while(accesso_file == false){;}
		    // Imposto che il thread sta usando il file 
		    accesso_file = false;
	    	// Array che conterrà l'indirzzo ip del client che ha richiesto l'operazione calcolata 
		    char ip[20];
	    	// Ottengo e salvo indirizzo IP del client 
		    inet_ntop( AF_INET, &pthread_arg->client_address.sin_addr , ip, 20);
		    // Creazione della stringa da salvare nel file di log  
		    sprintf(output_log,"CLIENT IP:%s PORT:%d OP:%f%c%f = %f INIZIO:%s FINE:%s\n", ip,  htons( client_address.sin_port) ,  numero1, op, numero2,  	risultato, timestamp_ricezione, timestamp_fine );
		

            // Qui fare controllo sulla scrittura 

		    // Scrittura sul file 
		    fprintf( logFile,"%s", output_log);
		    // Rilascio il file 
		    accesso_file = true;
		    
		    
		    // Creazione stringa da inviare al client, contiene i timestamp e il risultato dell'operazione
		    sprintf(risultato_stringa, "%s,%s,%f", timestamp_ricezione, timestamp_fine, risultato );

		    // Invia risposta al client  
		    if ( send(new_socket_fd, risultato_stringa ,sizeof(risultato_stringa), 0  ) == -1){
			    printf("Errore durante la scrittura\n");
			    pthread_cancel( pthread_self());
		    }else{
		    	printf("Operazione: %f %c %f = %f\n", numero1, op, numero2, risultato);
		    }
		
		



	    }

	    
       


    }
    // Chiudo la socket per la comunicazione
    close(new_socket_fd);
    // Chiudo il file di log 
    fclose(logFile);
    return NULL;
}
// Routine per la gestione dei segnali 
void signal_handler(int signal_number){
    exit(0);
}

