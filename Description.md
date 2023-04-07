## Descrizione generale
  L’esercizio richiedeva di creare una calcolatrice di tipo client-server dove il client prende in input le operazioni da svolgere, dopo 
  aver verificato che è un’operazione è valida, la formatta nel formato [operazione, Operando1, Operando2], e lo invia al server che 
  esegue il calcolo e invia al client il risultato insieme ai timestamp di inizio e fine calcolo nel formato [timestamp ricezione richiesta, 
  timestamp invio risposta, risultato operazione]. Il client calcola il tempo di esecuzione e lo stampa in output insieme al risultato 
  dell’operazione che era stata inviata al server.

## Librerie utilizzate sia per client che server 
  • stdio.h : utilizzata per gestione input/output;<br />
  • stdbool.h : per implementare le variabili booleane usate, si sarebbe potuto evitare ed usare un int;<br />
  • unistd.h : per funzione close();<br />
  • stdlib.h : per funzione exit();<br />
  • string.h : per sfruttare funzioni come strcmp(), memset(), memcpy(), bzero(), strncat(), strtok();<br />
  • netdb.h: per funzione gethostbyname();<br />
  • sys/socket.h: struttura dati sockaddr, funzioni connect(), recv();<br />
  • ctype.h: per funzione isdigit();<br />

## Costanti utilizzate
### Client:
  • MAX_LEN_INPUT lunghezza massima della stringa letta in input e usata poi per gli scambi<br />
  • MAX_LEN_NUMBER lunghezza massima dei singoli operatori presi dalla stringa letta <br />
  • MAX_LEN_RESULT lunghezza massima del risultato ricevuto dal server <br />
  • INF stringa che rappresenta infinito eventualmente ricevuto dal server <br />
  • char operazioni[4] = {‘+’, ‘-‘, ‘*’,’/’} array che rappresenta le varie operazioni che il server può svolgere e che sono 
    quindi consentite <br />
### Server:
  • BACKLOG: usata per la listen rappresenta la lunghezza massima in cui può crescere la coda di connessioni in 
    sospeso per la sockfd<br />
  • MAX_LEN_INPUT: lunghezza massima della stringa ricevuta in input dal client <br />
  • MAX_LEN_RESULT: lunghezza massima della stringa inviata al client come risultato dell’operazione richiesta<br />
  • DIM_TIMESTAMP: lunghezza massima dei timestamp inviati al client<br />
  • DIM_OUTPUT: lunghezza massima della stringa che verrà salvata all’interno del file di log<br />
  
## Funzioni dichiarate
### Client:
  • int check_positive_or_negative(char): funzione che controlla se il carattere che è stato passato come parametro 
    rappresenta il simbolo ‘+’ oppure ‘-‘, la funzione prende in input un singolo carattere;<br />
  • int check_operator(char): funzione che controlla se il carattere che è stato passato come parametro rappresenta un 
    simbolo delle operazioni standard possibili da eseguire, specificate nell’array ‘operazioni’, la funzione prende in input 
    un singolo carattere;<br />
  • int lettura_input(char [], char [], char [] , char* ): funzione che effettua la lettura della stringa dall’utente, controlla 
    carattere per carattere se la stringa è formattata bene, se i caratteri inseriti sono consentiti e se la lunghezza dei 
    singoli operandi rientra nella lunghezza massima consentita, la funzione prende in input tre array (usati poi per: la 
    stringa che viene letta in input dall’utente| il primo numero letto| il secondo numero letto) e un puntatore ad un 
    carattere (che conterrà il simbolo dell’operazione richiesta);<br />
  • void print_errore(): funzione che stampa un messaggio di errore per indicare all’utente che sta inserendo una stringa 
    che non rispetta lo standard richiesto e mostra i modi in cui è possibile scrivere una stringa corretta;
    Server:<br />
  • void *esecuzione_server( void *arg): routine eseguita dai vari thread creati;<br />
  
## Variabili dichiarate
  Le varie variabili usate sono state adeguatamente commentate all’interno dei vari file contenenti il codice del client e del server
  
## Funzionamento programma 
### Lato client 
  Il client dopo aver creato le varie variabili, imposta la porta utilizzata per la comunicazione, usiamo la 8082, e si ottiene i 
  dati sull’host locale, che eseguira’ il server, attraverso la funzione gethostbyname, che ritornera’ i dati in una struttura 
  hostent. Si inizializza la memoria relativa alla variabile “server_address” attraverso memset(), si indica che si vuole 
  comunicare in rete usando il protocollo IPv4 (impostando la family ad AF_INET), si converte la porta in ordine di byte della 
  rete secondo il protocollo TCP/IP e si copiano dei campi tra le strutture server_host e server_address. 
  Si crea la struttura socket necessaria per la comunicazione e lo si salva nella struttura “socket_fd”, si tenta la connessione 
  con il server e si verifica se ci si riesce a connettere, il caso negativo si stampa un messaggio di errore e si termina il 
  processo.
  Si inizia un ciclo while che non avrà fine finchè avverrà un errore oppure il client sarà terminato, all’interno del ciclo
  saranno inizializzate la stringhe lette in input e ricevute dal server, con memset() e bzero(), dopo di che si effettua un 
  controllo sul valore ritornato dalla funzione lettura_input(), la seguente funzione leggerà in input dall’utente e controllerà 
  che la stringa fornita sia nel formato corretto, se darà esito positivo ritornerà 1.
  Lettura_input() prenderà in input il riferimento al buffer per la lettura della stringa di input, le 2 stringhe per salvare i due 
  operandi e il puntatore ad un carattere che conterrà l’operazione richiesta dall’utente, controllerà carattere per carattere la 
  stringa, andando a controllare che la stringa sia in un formato consentito (permette di inserire stringhe del tipo “1+1”, “-1+-
  1”, “1.02+1.02”, “-1.05*+0.5”, “-4 *6”, “-5 + -8”). 
  Nel dettaglio consentirà:
  • che il primo carattere sia un numero oppure sia un segno del tipo ‘+’ o ‘-‘ , quest’ultima condizione viene 
  verificata attraverso la funzione check_positive_or_negative();
  • controlla che i vari caratteri siano tutti numeri oppure un segno, quest’ultima condizione viene verificata 
  attraverso la funzione check_operator();
  • le stringhe possono contenere anche spazi, ma solo uno tra i vari operandi e il simbolo di operazione;
  • le stringhe possono contenere anche ‘.’ per consentire l’inserimento di anche i numeri decimali;
  • i due operatori potranno avere una lunghezza massima consentita indicata dalla costante 
  MAX_LEN_NUMBER;
  Se una di queste condizioni non sarà verificata allora verrà richiamata la funzione print_errore() e si ritornerà 0.
  Tornando nel nostro ciclo while, se la funzione lettura_input() ritornerà un numero diverso da 0, creeremo la stringa di 
  invio, e concateniamo attraverso la funzione strcat() gli operandi con l’operatore nel formato richiesto dalla traccia, cioè 
  “operatore ,operando1,operando2”, si tenta quindi di inviare con la funzione send() la stringa “invio” al server, in caso di 
  fallimento nella trasmissione, si stamperà un messaggio di errore e il client sarà terminato.
  In caso positivo invece, si attende finchè la recv() non ci consente di ottenere il risultato dell’operazione ricevuto dal server, 
  che viene salvato nella stringa “risposta”, in caso ci sia un errore di stampa un messaggio di errore e si termina il client. Se 
  abbiamo ricevuto la stringa allora attraverso la funzione strtok() andiamo a suddividere la stringa “risposta” sul carattere ‘,’ 
  andando a salvare timestap_inizio e timestamp_fine e convertendoli in long double in modo da poter successivamente 
  calcolare il tempo di esecuzione. Se il risultato è uguale alla costante INF, cioè è uguale ad “inf”, allora indica che 
  l’operazione non era consentita (come ad esempio operando/0), oppure in caso negativo si stampa il risultato 
  dell’operazione e il tempo impiegato per il calcolo.
  Infine si svuota lo standard input per evitare errori dalla lettura automatica dopo una trasmissione.
### Lato server
  Si apre il file “Log_file” in modalità ‘a’, cioè aggiunge i nuovi dati alla fine del file e se il file non esiste viene creato. Questo 
  consentirà in seguito di scrivere le varie operazioni, gli identificatori dei client e i timestamp delle operazioni richieste. 
  Il server imposta la porta utilizzata per la comunicazione, usiamo la 8082, si indica che si vuole comunicare in rete usando il 
  protocollo IPv4 (impostando la family ad AF_INET), si converte la porta in ordine di byte della rete secondo il protocollo 
  TCP/IP, si imposta INADDR_ANY che ci consente di dichiarare che la socket sarà associata a tutte le interfacce locali.
  Si crea la struttura socket necessaria per la comunicazione e lo si salva nella struttura “socket_fd”, con la funzione bind() si 
  associa poi l’indirizzo ip specificato da “address” alla socket “socket_fd”, con la funz listen() indica che “socket_fd” sarà 
  nello stato passivo, cioè aspetterà le varie richieste di connessione da parte dei client, dopo di che si vanno ad associare vari 
  gestori di segnali ad alcuni segnali attraverso la funzione signal(). Abbiamo:
  • SIGTERM: si richiede la terminazione del client, viene invocata la routine signal_handler;
  • SIGPIPE: errore con la connessione socket, si ignora il segnale;
  • SIGINT: l’utente richiede la terminazione del processo, viene invocata la routine signal_handler;
  Dopo di che con la funz pthread_attr_init() si inizializzano gli attributi di “pthread_attr” con i valori di default dei campi, con 
  la funz pthread_attr_setdetachstate() si imposta l’attributo relativo allo stato di associazione degli attributi dei thread, con 
  PTHREAD_CREATE_DETACHED si indica che i thread creati usando “&pthread_attr” verranno creati in uno stato 
  “distaccato”. 
  Si inizia un ciclo while che non avrà fine finchè avverrà un errore oppure il server sarà terminato, all’interno del ciclo con 
  una malloc viene alloca la memoria necessaria per “pthread_arg”, senza inizializzarla, controllando se la variabile è diversa 
  da NULL allora l’allocazione sarà avvenuta con successo.
  Si imposta la lunghezza dell’indirizzo del client possibile, con la funz accept() si estrae la prima richiesta di connessione dalla 
  code delle connessioni in attesa, crea la connessione, restituisce il descrittore per il socket e si controlla se l’accept() è 
  andata a buon fine.
  Dopo di che si assegna all’argomento “new_socket_fd” di “pthread_arg” il valore del “new_socket_fd”, si creano i vari 
  thread che eseguiranno la routine “esecuzione_server”.
  Nella routine “esecuzione_server” dopo diverse dichiarazioni ed assegnazioni si inizializza l’area di memoria che servirà per 
  contenere la stringa formattata ricevuta dal client, dopo di che iniziamo un ciclo while che non avrà fine se non per errori o 
  terminazione diretta, si chiamerà la funzione recv() che attenderà finchè non si riceve un messaggio dal client relativo alla 
  connessione, si sfrutta la funz clock_gettime() per ottenere il tempo attuale (usato come timestamp) che sfrutta la 
  struttura timespec e il tipo clockid_t, si salverà il timestamp come stringa nella variabile “timestamp_ricezione” attraverso 
  la funz sprintf(), andando a creare una stringa del tipo ‘secondi.nanosecondi’, si controlla se effettivamente il client non sia 
  terminato, se non è più attivo cancella il thread assegnato al client (il thread sarà se stesso), attraverso la funzione 
  pthread_cancel( con parametro la funzione pthread_self() che ritorna l’id del thread chiamante). In caso il client sia ancora
  attivo invece si suddivide la stringa attraverso la funz strtok() sul carattere ‘,’ e salva l’operatore e i due operandi nelle 
  relative variabili, convertendoli in double attraverso la funzione strtod(). Dopo di che attraverso uno switch sulla variabile 
  “op” si va ad effettuare l’operazione richiesta dal client e si salva il risultato nella variabile risultato. Unica nota da 
  aggiungere sul calcolo, nel caso di divisione ‘/’ si controllerà se si sta effettuando una divisione del tipo (operando/0), in 
  questo caso il risultato prenderà il valore di infinito per indicare l’errore al client. Convertiamo il risultato in stringa 
  “risultato_stringa”, si ottiene di nuovo attraverso la funz clock_gettime() il timestamp attuale e si converte come prima ma 
  nella variabile “timestap_fine”, si aspetta che sia concesso l’accesso al file, si applica un lock al file attraverso la variabile 
  “accesso_file” settandola a false, si ottiene l’indirizzo ip del client attraverso la funzione inet_ntop(), si crea la stringa finale 
  nel formato ["CLIENT IP: ip PORT: porta OP: op1 operatore op2 = risultato INIZIO:timestamp_inizio FINE:timestamp_fine], la 
  porta si ottiene attraverso la funzione htons(), salviamo la stringa appena creata sul file di log e si rilascia il lock sul file 
  (accesso_file = true). Come ultimi passi si va a creare la stringa che si deve inviare al client attraverso la funz sprintf() nella 
  variabile risultato_stringa, con il formato “timestamp_ricezione,timestamp_fine, risultato”, e lo si invia con la funz send(), 
  verificando se la trasmissione è andata a buon fine, in caso di errore si termina il thread relativo alla trasmissione.
  Alla fine del ciclo while si chiude il file di log aperto e la socket utilizzata.
## Come utilizzare il programma
  Su una o più altre shell avviare i programmi client, il programma chiederà di inserire un’operazione nel formato consentito e in caso 
  di operazione corretta e operazione effettuata con successo mostrerà il risultato e il tempo di esecuzione formattato nel modo 
  “Risultato: ris Tempo esecuzione: tempo s”. In caso di errore di connessione mostrerà il relativo errore, in caso di operazione non 
  consentita (e.s. numero/0) stamperà il suo errore e in caso di operazione non formattata nel modo corretto mostrerà l’errore e 
  spiegherà all’utente quali formattazioni sono considerate corrette.
  In una shell avvia il programma server che servirà tutti i futuri client che verranno attivati. Il server rimarrà in attesa e dopo aver 
  eseguito le operazioni mostrerà tutte le operazioni con i risultati richieste dai vari utenti.
## Prove effettuate
#### Formato operazione -> Risultato stampato dal client
  4 * 5 -> Risultato:20.000000 Tempo esecuzione:0.000015s<br />
  4* -6-> Risultato:-24.000000 Tempo esecuzione:0. 000018s<br />
  4*5 -> Risultato:20.000000 Tempo esecuzione:0.000026s<br />
  15+58.6 -> Risultato:73.600000 Tempo esecuzione:0.000015s<br />
  -125.301259*+3.232501 -> Risultato:-405.036445 Tempo esecuzione:0.000016s<br />
  18-96.3 -> Risultato:-78.300000 Tempo esecuzione:0.000015s<br />
  -65/+9 -> Risultato:-7.222222 Tempo esecuzione:0.000013s<br />
  -0.0123654789872/8512325.0022 -> Gli operandi devono essere di len massima 14<br />
  -745.025/0 -> L'operazione non e' consentita (Es: num/0)<br />
  -Se si esegue un’operazione, ma il server è già terminato -> Errore con il server. Riavviare il processo<br />
  -Se il client prova a connettersi ma il server non è attivo -> Errore nella connessione con il server: Connection refused<br />
  -8..3654-96.2 | -98*+-8 | ** | 1-. | -+-+ | *+1 | -1/. | 8.635*965..256 | 98.36*.9 | +9 | 13d.90/45 | a/b | a467.0/85 | 12i/9 |  567i/8 | 720A/8 | A/B | ab/c | 34*584/59 | 4+5*5 -> La stringa e' formattata male provare con uno dei seguenti formati:<br />
  - N1opN2<br />
  - SegnoN1opSegnoN2<br />
  - N1 op N2<br />
