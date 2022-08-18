#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <ctype.h>
#include <stdbool.h>

// Lunghezza massima della stringa in input da utente 
#define MAX_LEN_INPUT 40
// Lunghezza massima dei singoli operandi contenuti nella stringa 
#define MAX_LEN_NUMBER 14
// Lunghezza massima del risultato ricevuto dal server 
#define MAX_LEN_RESULT 180
// Risultato speciale server 
#define INF "inf"

// Funzione che controlla se il char passato come parametro rappresenta un simbolo + oppure -
int check_positive_or_negative(char );
// Funzione che controlla se il char passato come parametro rappresenta un simbolo delle operazioni standard
int check_operator(char );
// Funzione che si occupa di leggere l'input dall'utente e verificare che sia nel formato corretto
int lettura_input(char [], char [], char [] , char* );
// Funzione che stampa messaggio di errore nella formattazione della stringa formata in input 
void print_errore();


// array delle varie operazioni disponibili 
const char operazioni[4]= {'+', '-', '*', '/'};
// Stringa usata per prendere il risultato dal server 
char risposta[MAX_LEN_RESULT];


int main(int argc, char *argv[]) {
    // Numero di porta e socket descriptor 
    int server_port, socket_fd;
    struct hostent *server_host;
    struct sockaddr_in server_address;
	
    // Stringhe che rappresentano i due numeri letti da input
    char numero1[MAX_LEN_NUMBER];
    char numero2[MAX_LEN_NUMBER];
    // Simbolo dell'opeazione da svolgere 
    char operazione;
    // Stringa in input da cui ricavare gli operandi e l'operazione 
    char input[MAX_LEN_INPUT]; 
    // Stringa formattata da inviare al client    
    char *invio[MAX_LEN_INPUT+5];
    // Long double per estrarre il timestamp e calcolare il tempo di lavoro necessario
    long double timestamp_inizio;
    long double timestamp_fine;
    
    // Variabile di lavoro 
    int c;
    // Puntatore usato per contenere poi il risultato della funzione strtok per separare la stringa 
    char *stringa_conversione;

    /* Puntatore usato dalla funzione strtok per capire dove interrompere la conversione 
    ->   punta al primo carattere che non è stato convertito*/
    char *eptr;

    

    // Si sfrutta la porta 8082  n
    server_port = 8082;
    server_host = gethostbyname("127.0.0.1");

    /* Inizializzo la memoria
       Imposto che voglio usare IPv4 a livello di rete 
       Inizializzo l'indirizzo IPv4 del server con i dati dell'host locale 
       Copio gli indirizzi 
    */ 
    memset(&server_address, 0, sizeof server_address);
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(server_port);
    memcpy(&server_address.sin_addr.s_addr, server_host->h_addr, server_host->h_length);

    // Crea la socket TCP
    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Errore nella creazione del socket");
        exit(1);
    }

    // Connetto il client con il socket server
    if (connect(socket_fd, (struct sockaddr *)&server_address, sizeof (server_address)) <0) {
		perror("Errore nella connessione con il server");
       		exit(1);
	}

    


    while(1){

	
        // RIPULISCO L'ARRAY OGNI VOLTA 
	memset(input, 0, MAX_LEN_INPUT);
	
	// Inizializzo la stringa 
	bzero(risposta, 40);

        

        // Se ritorno 1 tutto ok e quindi posso comunicare con il server 
    	if(  lettura_input(input, numero1, numero2, &operazione) != 0 ){
      
		// Creo stringa da inviare al server 
		char invio[MAX_LEN_INPUT] = "";
		// Concateno il simbolo di operazione 
		strncat(invio, &operazione, 1);
		// Concateno separatore
		strcat(invio, ",");
		// Concateno il primo operando 
		strcat(invio, numero1);
		// Concateno separatore
		strcat(invio, ",");
		// Concateno il secondo operando 
		strcat(invio, numero2);
		
		// Invio al server la stringa formattata rappresentante l'operazione da svolgere
	    	if (  send(socket_fd, invio, MAX_LEN_INPUT, 0  )  == -1){
	      	  printf("Errore con il server. Riavviare il processo");
	      	  exit(EXIT_FAILURE);
	    	}

	    
	    	// Se server non ha risposto allora si stampa messaggio di errore e si termina il processo client
	  	if (  recv(socket_fd, risposta, MAX_LEN_RESULT, 0 )  <= 0){
		    printf("Errore con il server. Riavviare il processo");
		    exit(EXIT_FAILURE);
		}else{
	    
	    		// Suddivido la stringa sul carattere ','
		    	stringa_conversione = strtok(risposta, ",");
		    	timestamp_inizio = strtold( stringa_conversione, &eptr );
		    	stringa_conversione = strtok(NULL, ",");
		    	timestamp_fine = strtold( stringa_conversione, &eptr );
		    	stringa_conversione = strtok(NULL, ",");
		    	
		    	// Se il risultato dell'operazione è INF stampo errore se no stampo il risultato richiesto
		    	if (strcmp(stringa_conversione, INF)== 0){
		    		printf("L'operazione non e' consentita (Es: num/0)");
		    	}else{
		    	
		    	
		    	printf("Risultato:%s Tempo esecuzione:%Lfs",  stringa_conversione, timestamp_fine - 		timestamp_inizio);     	
		    	
		    	}
	    	
    	
    	
    	
    
    
    		}

    
   
    

    	} 


        
    	// svuoto lo standard input 
    	while( (c=getchar()) != '\n' && c != EOF)
        	;   
      

    }

    close(socket_fd);
    return 0;
}

 

/*  Funzione che si occupa di leggere in input le varie operazioni dall'utente e attraverso diversi controlli
    riesce a stabilire se la stringa passata rappresenta un operazione valida -> nel caso lo sia salva i dati nelle variabili passate per parametri, salvando i due operandi e l'operazione che successivamente il server dovrà eseguire
    La funzione ritorna 0 in caso di problema e 1 in caso la stringa passata sia del formato corretto 
*/
int lettura_input(char input[], char numero1[], char  numero2[], char* operaz){


    // Posizioni nei due interi
    int s1 = 0;
    int s2 = 0;
    // Variabile usata per contare il numero di operatori presenti nella stringa 
    int noperazioni = 0;
    // Operatore di cui verrà richiesto il calcolo
    char operazione = '\0';
    // Posizione nella stringa originale 
    int pos = 0;
    // Variabile usata per contare il numero di '-' e '+' -> posso avere al massimo -- o ++ non --- +++ 
    int n = 0; 
    // Variabile er la gestione di alcuni spazi che l'utente può inserire ( 4 + 4)
    bool space = false;
    // Variabile per gestire i . nei numeri decimali 
    int point = 0;

    // Stampo e prendo in input la stringa che rappresenterà l'operazione 
    printf("\nInserire operazione: ");
    scanf("%[^\n]s", input);
    // Inizializzo gli array se no potrei leggere dati rimasti dalle letture precedenti
    memset(numero1, 0, MAX_LEN_NUMBER);
    memset(numero2, 0, MAX_LEN_NUMBER);
   
    

    //Scorro la stringa fino alla fine oppure mi fermo prima in caso di errori nella stringa 
    while( input[pos] != '\0' ){

    	// Se si stanno cercando di eseguire piu' operazioni contemporaneamente ( 4+5*5)
	if(noperazioni>1){
	   print_errore();
	   return 0;
	}

	// Il primo carattere può essere + - oppure un numero
	if (pos == 0){
	   // si controlla se il carattere elaborato è un segno (+, -) oppure un numero 
	   if(  check_positive_or_negative(input[pos]) || isdigit(input[pos])  ){
		                        
		 numero1[s1] = input[pos];
		 s1+=1; 
		 // Si controlla se il carattere è quindi un segno (+, -) 
		 if(check_positive_or_negative(input[pos])){
		 	n+=1;
		  }

	    }else{
		 print_errore();
		 return 0;
		 }
		   

		
	} else if( pos != 0 ){
	/* In questa parte stiamo analizzando tutti i caratteri successivi al primo 
	 Controllo se sto leggendo uno spazio e se ne avevo uno gia' in precedenza (n1 2spazi OP 2spazi n2 non consentito)*/
	   if (input[pos] == ' ' && space ){
		    
	   	print_errore();
		return 0;
		// Controllo se sto leggendo uno spazio e non ne avevo già incontrato uni in precedenza
	   }else if(input[pos] == ' ' && space == false){
		space = true;
		       
	   // Se è un numero il carattere analizzato 
           }else if (isdigit(input[pos]) ){
		// Resetto lo spazio poichè ho incontrato un numero 
		space = false;
		//Resetto il contatore relativo a + o - una volta che incontro un numero 
		n = 0;       
		// Se sto leggendo il primo operando ( quello prima dell'operazione)
		if ( noperazioni == 0 ){
		     numero1[s1] = input[pos];
		     s1+=1;
	        // Sto leggendo il secondo operando 
	        }else if ( noperazioni == 1){
		     // Se sto leggendo la prima cifra del secondo numero allora resetto point 
		     if (s2 == 0){
	                 point = 0;
	             }
		     // Se sto leggendo il secondo operando ( quello dopo il simbolo di operazione)
		     numero2[s2] = input[pos];
	             s2+=1;
		}

 	   // Caso in cui riscontriamo la presenza di un operatore  '+' o '-'
	   }else if ( check_positive_or_negative(input[pos])){
	   // Resetto lo spazio -> sto incontrando l'operatore + o - 
		space = false;		
		// Caso in cui ho un - o un +
	        if( n ==0 && noperazioni == 0 ){
	            operazione = input[pos];
		    n+=1;
	            noperazioni+=1;

		 /* Caso n - + * /  [+ -] n quindi ho già un simbolo di operazione 
		    ma ho incontrato un '-' o '+' e non ho ancora iniziato a leggere il secondo operando */
		 }else if(n == 0 && noperazioni == 1 && s2 ==0 ){
		    point = 0;
		    n+=1;
		    numero2[s2] = input[pos];
		    s2+=1;
		        // Caso n -- ++ n
		 }else if( n == 1){
		 //Qui si gestisce il caso in cui ho già un '-' o '+' ma ancora non ho letto il secondo operando
		 // Considero quindi il simbolo come il segno del secondo operando  
		    if( n == 1 && s2 == 0){
		        point = 0;
		        numero2[s2] = input[pos];
		        s2+=1;
		    // Se ho già letto un carattere del primo operando allora restituisco errore 
		    }else if(n == 1 && s1 == 1){
		        print_errore();
		        return 0;
		    }else{
	            // Se stavo già leggendo il secondo operando stampo errore 
		        print_errore();
		        return 0;
		    }
		  }else{
		    print_errore();
	            return 0;
		  }
	 
		    // Caso in cui incontro un operatore + - * / -> la scelta in verità si restringe a * /
	   }else if(check_operator(input[pos]) ){
		  // Resetto lo spazio -> sto incontrando un operatore (* o /)
		  space = false;
	       	  // Incremento il numero di operazioni rilevato 
		  noperazioni+=1;
		  // Salvo il simbolo di operazione rilevato 
		  operazione = input[pos];
	   }else{
		  // Gestione dei numeri con virgola 
		  // Caso in cui sia il primo numero ad avere la virgola 
		  if(point == 0 && s1 != 0 && noperazioni == 0 && isalpha(input[pos]) ==0){
		     numero1[s1] = input[pos];
		     s1+=1;
		     point+=1;
		  // Caso in cui sia il secondo numero ad avere la virgola 
		  }else if ( point == 0 && s2 != 0 && noperazioni == 1 && isalpha(input[pos]) ==0){
		     numero2[s2] = input[pos];
		     s2+=1;
		     point+=1;
		  }else{
		  // Nessun caso analizzato fin'ora è stato trovato 
		     print_errore();
		     return 0;
		  }
	    }

	}
	// Sposto l'indice nella stringa in modo da anlizzare il carattere successivo 
	pos+=1; 
	   
	/* Controllo di non prendere numeri la cui lunghezza supera la massima consentita dagli array che
	   andranno a contenerli*/
	if (s1 >= MAX_LEN_NUMBER | s2 >=  MAX_LEN_NUMBER){
	    printf("Gli operandi devono essere di len massima %d", MAX_LEN_NUMBER); 
	    return 0;
 	}
    }
   
  
   
    /* Controllo se manca un operando oppure il simbolo dell'operazione
     (ad esempio se si invia nulla, se invia solo un numero oppure un simbolo oppure simbolo e un solo numero)*/
    if(numero1[0] == '\0' || numero2[0] == '\0' || operazione == '\0'||(s1 == 1 && (numero1[0] == '-' ||       numero1[0] == '+' ) )   ||(s2 == 1 && (numero2[0] == '-' || numero2[0] == '+' ) ) ){
        print_errore();
        return 0;
    }
    
    *operaz = operazione;

    return 1;
}
// Funzione che controlla se il carattere passato è un + o un -
int check_positive_or_negative(char c){

    if ( c == operazioni[0] || c == operazioni[1]   ){
           return 1;
    }
    return 0;


}

// Funzione che controlla se il caratter passato è un simbolo di operazione tra quelli standard 
int check_operator(char c){
     if ( c == operazioni[0] || c == operazioni[1] || c == operazioni[2] || c == operazioni[3]  ){
           return 1;
    }
    return 0;
}


//Funzione che stampa un messaggio di errore e indica all'utente alcuni dei formati in cui può scrivere l'operazione
void print_errore(){
    printf("La stringa e' formattata male provare con uno dei seguenti formati:\n- N1opN2\n- SegnoN1opSegnoN2\n- N1 op N2");
}

