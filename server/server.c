#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "/usr/include/postgresql/libpq-fe.h"

#define PORT 8080
#define BUFFER_SIZE 1024

// Funzione per gestire il client (registrazione o login)
void *handle_client(void *socket_desc);
void handle_register(PGconn *conn, const char *username, const char *password);
int handle_login(PGconn *conn, const char *username, const char *password);
void send_books_from_db(PGconn *conn, int sock);
void send_books_from_db_key_genre(PGconn *conn, int sock, const char *field1, const char *field2);
void send_books_from_db_key(PGconn *conn, int sock, const char *field1);
void send_books_from_db_genre(PGconn *conn, int sock, const char *field1);


// Funzione per gestire la registrazione
void handle_register(PGconn *conn,const char *username, const char *password) {
    printf("Richiesta di registrazione ricevuta per l'utente: %s\n", username);
    if (PQstatus(conn) == CONNECTION_BAD) {
        fprintf(stderr, "Errore connessione al database: %s\n", PQerrorMessage(conn));
        PQfinish(conn);
        return;
    }

    char query[BUFFER_SIZE];
    snprintf(query, sizeof(query), "INSERT INTO users (username, password) VALUES ('%s', '%s');", username, password);
    PGresult *res = PQexec(conn, query);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stderr, "Errore durante l'inserimento nel database: %s\n", PQerrorMessage(conn));
    } else {
        printf("Registrazione completata per l'utente: %s\n", username);
    }

    PQclear(res);

}

// Funzione per gestire il login
int handle_login(PGconn *conn,const char *username, const char *password) {

    if (PQstatus(conn) == CONNECTION_BAD) {
        fprintf(stderr, "Errore connessione al database: %s\n", PQerrorMessage(conn));
        PQfinish(conn);
        return 0;
    }

    char query[BUFFER_SIZE];
    snprintf(query, sizeof(query), "SELECT * FROM users WHERE username = '%s' AND password = '%s';", username, password);
    PGresult *res = PQexec(conn, query);

    int login_success = (PQntuples(res) == 1);  // Se c'è una riga, il login è valido

    if (login_success) {
        printf("Login riuscito per l'utente: %s\n", username);
    } else {
        printf("Login fallito per l'utente: %s\n", username);
    }

    PQclear(res);


    return login_success;
}

//Funzione per richiedere tutti i libri
void send_books_from_db(PGconn *conn, int sock) {
    // Connessione a PostgreSQL
    if (PQstatus(conn) == CONNECTION_BAD) {
        fprintf(stderr, "Connection to database failed: %s", PQerrorMessage(conn));

        return;
    }

    // Esegui la query per prendere i dettagli dei libri
    PGresult *res = PQexec(conn, "SELECT isbn, titolo, genere, imageUrl, autore, quantita, copieprestate FROM books");
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "SELECT failed: %s", PQerrorMessage(conn));
        PQclear(res);

        return;
    }

    int rows = PQntuples(res);
    char buffer[1024];
    
    // Itera sui risultati e invia i dati di ogni libro
    for (int i = 0; i < rows; i++) {
        snprintf(buffer, sizeof(buffer), "%s,%s,%s,%s,%s,%s,%s\n", 
                 PQgetvalue(res, i, 0),  // isbn
                 PQgetvalue(res, i, 1),  // titolo
                 PQgetvalue(res, i, 2),  // genere
                 PQgetvalue(res, i, 3),  // imageUrl
                 PQgetvalue(res, i, 4),  // autore
                 PQgetvalue(res, i, 5),  // quantita
                 PQgetvalue(res, i, 6)); // copieprestate

        send(sock, buffer, strlen(buffer), 0);
    }

    // Invia segnale di fine ("END")
    send(sock, "END\n", 4, 0);

    PQclear(res);

}

//Funzione per richiedere libri doppio filtro
void send_books_from_db_key_genre(PGconn *conn, int sock, const char *field1, const char *field2) {
    // Connessione a PostgreSQL
    if (PQstatus(conn) == CONNECTION_BAD) {
        fprintf(stderr, "Connection to database failed: %s", PQerrorMessage(conn));

        return;
    }

    // Esegui la query per prendere i dettagli dei libri
    char query[BUFFER_SIZE];
    snprintf(query, sizeof(query), 
    "SELECT isbn, titolo, genere, imageUrl, autore, quantita, copieprestate FROM books WHERE titolo LIKE '%%%s%%' AND genere = '%s'", 
    field1, field2);
    PGresult *res = PQexec(conn, "SELECT isbn, titolo, genere, imageUrl, autore, quantita, copieprestate FROM books");
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "SELECT failed: %s", PQerrorMessage(conn));
        PQclear(res);

        return;
    }

    int rows = PQntuples(res);
    char buffer[1024];
    
    // Itera sui risultati e invia i dati di ogni libro
    for (int i = 0; i < rows; i++) {
        snprintf(buffer, sizeof(buffer), "%s,%s,%s,%s,%s,%s,%s\n", 
                 PQgetvalue(res, i, 0),  // isbn
                 PQgetvalue(res, i, 1),  // titolo
                 PQgetvalue(res, i, 2),  // genere
                 PQgetvalue(res, i, 3),  // imageUrl
                 PQgetvalue(res, i, 4),  // autore
                 PQgetvalue(res, i, 5),  // quantita
                 PQgetvalue(res, i, 6)); // copieprestate

        send(sock, buffer, strlen(buffer), 0);
    }

    // Invia segnale di fine ("END")
    send(sock, "END\n", 4, 0);

    PQclear(res);

}

//Funzione per richiedere libri con keyword
void send_books_from_db_key(PGconn *conn, int sock, const char *field1) {
    // Connessione a PostgreSQL
    if (PQstatus(conn) == CONNECTION_BAD) {
        fprintf(stderr, "Connection to database failed: %s", PQerrorMessage(conn));

        return;
    }

    // Esegui la query per prendere i dettagli dei libri
    char query[BUFFER_SIZE];
    snprintf(query, sizeof(query), "SELECT isbn, titolo, genere, imageUrl, autore, quantita, copieprestate FROM books WHERE titolo LIKE '%%%s%%'", field1);
    PGresult *res = PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "SELECT failed: %s", PQerrorMessage(conn));
        PQclear(res);

        return;
    }

    int rows = PQntuples(res);
    char buffer[1024];
    
    // Itera sui risultati e invia i dati di ogni libro
    for (int i = 0; i < rows; i++) {
        snprintf(buffer, sizeof(buffer), "%s,%s,%s,%s,%s,%s,%s\n", 
                 PQgetvalue(res, i, 0),  // isbn
                 PQgetvalue(res, i, 1),  // titolo
                 PQgetvalue(res, i, 2),  // genere
                 PQgetvalue(res, i, 3),  // imageUrl
                 PQgetvalue(res, i, 4),  // autore
                 PQgetvalue(res, i, 5),  // quantita
                 PQgetvalue(res, i, 6)); // copieprestate

        send(sock, buffer, strlen(buffer), 0);
    }

    // Invia segnale di fine ("END")
    send(sock, "END\n", 4, 0);

    PQclear(res);

}

//Funzione per richiedere libri con genere
void send_books_from_db_genre(PGconn *conn, int sock, const char *field1) {
    // Connessione a PostgreSQL
    if (PQstatus(conn) == CONNECTION_BAD) {
        fprintf(stderr, "Connection to database failed: %s", PQerrorMessage(conn));

        return;
    }

    // Esegui la query per prendere i dettagli dei libri
    char query[BUFFER_SIZE];
    snprintf(query, sizeof(query), "SELECT isbn, titolo, genere, imageUrl, autore, quantita, copieprestate FROM books WHERE genere = '%s'", field1);
    PGresult *res = PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "SELECT failed: %s", PQerrorMessage(conn));
        PQclear(res);

        return;
    }

    int rows = PQntuples(res);
    char buffer[1024];
    
    // Itera sui risultati e invia i dati di ogni libro
    for (int i = 0; i < rows; i++) {
        snprintf(buffer, sizeof(buffer), "%s,%s,%s,%s,%s,%s,%s\n", 
                 PQgetvalue(res, i, 0),  // isbn
                 PQgetvalue(res, i, 1),  // titolo
                 PQgetvalue(res, i, 2),  // genere
                 PQgetvalue(res, i, 3),  // imageUrl
                 PQgetvalue(res, i, 4),  // autore
                 PQgetvalue(res, i, 5),  // quantita
                 PQgetvalue(res, i, 6)); // copieprestate

        send(sock, buffer, strlen(buffer), 0);
    }

    // Invia segnale di fine ("END")
    send(sock, "END\n", 4, 0);

    PQclear(res);

}


// Funzione eseguita da ogni thread per gestire il client
void *handle_client(void *socket_desc) {

    PGconn *conn = PQconnectdb("host=database port=5432 dbname=library user=postgres password=12345");

    int sock = *(int*)socket_desc;
    char buffer[BUFFER_SIZE] = {0};
    free(socket_desc);  // Liberiamo la memoria allocata per il socket

    // Leggi i dati inviati dal client
    int read_size = read(sock, buffer, BUFFER_SIZE);
    if (read_size <= 0) {
        printf("Errore durante la lettura dal client\n");
        close(sock);
        return NULL;
    }

    // Rimuovi il carattere di fine linea '\n' se presente
    char *newline_pos = strchr(buffer, '\n');
    if (newline_pos != NULL) {
        *newline_pos = '\0'; // Sostituisci '\n' con il terminatore di stringa '\0'
    }
    
    printf("Dati ricevuti: %s\n", buffer);

    // Estrai il tipo di richiesta (login o registrazione) e le credenziali
    char *token = strtok(buffer, ":");
    if (token != NULL) {
        char *request_type = token;
        char *field1 = strtok(NULL, ":");
        char *field2 = strtok(NULL, ":");

        //Blocco registrazione e login
        
            if (strcmp(request_type, "register") == 0) {
                // Gestione registrazione
                handle_register(conn, field1, field2);
                char *response = "Registrazione completata con successo!";
                send(sock, response, strlen(response), 0);
            } else if (strcmp(request_type, "login") == 0) {
                // Gestione login
                if (handle_login(conn, field1, field2)) {
                    char *response = "Login avvenuto con successo!";
                    send(sock, response, strlen(response), 0);
                } else {
                    char *response = "Login fallito: credenziali errate.";
                    send(sock, response, strlen(response), 0);
                }
                //blocco richieste libri
            } else if (strcmp(request_type, "allbooks") == 0){
                //gestione richiesta tutti i libri 
                send_books_from_db(conn, sock);
            } else if (strcmp(request_type, "totalfilter") == 0){
                //libri doppio filtro
                send_books_from_db_key_genre(conn, sock, field1, field2);
            } else if (strcmp(request_type, "onlytitle") == 0){
                //libri solo keyword
                send_books_from_db_key(conn, sock, field1);
            } else if (strcmp(request_type, "onlygenre") == 0){
                //libri sono genere
                send_books_from_db_genre(conn, sock, field1);
            } else {

                char *response = "Tipo di richiesta non valido.";
                send(sock, response, strlen(response), 0);
            }

    } else {
        char *response = "Richiesta non valida.";
        send(sock, response, strlen(response), 0);
    }

    // Chiudi la connessione
    close(sock);
    printf("Connessione chiusa per questo client\n");
    PQfinish(conn);
    pthread_exit(NULL);
    return NULL;
}


int start() {


    printf("Avvio del server...\n");
    printf("Server avviato e in attesa di connessioni...\n");

    //connessione al DB
    PGconn *conn = PQconnectdb("host=database port=5432 dbname=library user=postgres password=12345");
    
    if (PQstatus(conn) == CONNECTION_BAD) {
        fprintf(stderr, "Connessione al database fallita: %s\n", PQerrorMessage(conn));
        PQfinish(conn);
    } else {
        printf("Connesso\n");
    }
    //popolamento del DB
    const char *checkTablesQuery = "SELECT table_name FROM information_schema.tables WHERE table_schema='public';";
    PGresult *checkTablesResult = PQexec(conn, checkTablesQuery);
    if (PQresultStatus(checkTablesResult) != PGRES_TUPLES_OK || PQntuples(checkTablesResult) == 0) {
        printf("Le tabelle non esistono, creazione in corso...\n");

        const char *createTablesQuery = "CREATE TYPE genere AS ENUM ('classico', 'fantasy', 'storico'); CREATE TABLE users(id_utente SERIAL PRIMARY KEY, username VARCHAR(255) NOT NULL, password VARCHAR(255) NOT NULL); CREATE TABLE books (isbn VARCHAR(20) PRIMARY KEY, titolo VARCHAR(50) NOT NULL, genere GENERE NOT NULL, imageUrl VARCHAR(100) NOT NULL, autore VARCHAR(50) NOT NULL, quantita INTEGER NOT NULL, copieprestate INTEGER NOT NULL);";
        PGresult *createTablesResult = PQexec(conn, createTablesQuery);

        if (PQresultStatus(createTablesResult) != PGRES_COMMAND_OK) {
            fprintf(stderr, "Errore durante la creazione delle tabelle: %s", PQerrorMessage(conn));
            PQclear(createTablesResult);
            PQfinish(conn);
            exit(1);
        }
        PQclear(createTablesResult);

        printf("Popolazione in corso...\n");
/*const char *populateDBQuery1 = "INSERT INTO users (username, password) VALUES ('admin', 'admin');";
const char *populateDBQuery2 = "INSERT INTO books (isbn, titolo, genere, imageUrl, autore, quantita, copieprestate) VALUES "
    "('9788817154857', '1984', 'classico', 'https://covers.openlibrary.org/b/id/12054715-M.jpg', 'George Orwell', 5, 0), "
    "('9780547928227', 'The Hobbit', 'fantasy', 'https://covers.openlibrary.org/b/id/14625529-M.jpg', 'J.R.R. Tolkien', 4, 0), "
    "('9780140447934', 'War and Peace', 'classico', 'https://covers.openlibrary.org/b/id/14598041-M.jpg', 'Leo Tolstoy', 3, 0), "
    "('9780307277671', 'The Da Vinci Code', 'fantasy', 'https://covers.openlibrary.org/b/id/14553572-M.jpg', 'Dan Brown', 6, 0), "
    "('9788868360269', 'It', 'classico', 'https://covers.openlibrary.org/b/id/14656946-M.jpg', 'Stephen King', 4, 0), "
    "('9780143105428', 'Pride and Prejudice', 'classico', 'https://covers.openlibrary.org/b/id/14619629-M.jpg', 'Jane Austen', 5, 0), "
    "('9780307949486', 'The Girl with the Dragon Tattoo', 'fantasy', 'https://covers.openlibrary.org/b/id/6779579-M.jpg', 'Stieg Larsson', 5, 0), "
    "('9798392253876', 'The Great Gatsby', 'classico', 'https://covers.openlibrary.org/b/id/8248481-M.jpg', 'F. Scott Fitzgerald', 6, 0), "
    "('9788853019356', 'Treasure Island', 'storico', 'https://covers.openlibrary.org/b/id/12819540-M.jpg', 'Robert Louis Stevenson', 7, 0), "
    "('9788806228644', 'Frankenstein', 'classico', 'https://covers.openlibrary.org/b/id/12991957-M.jpg', 'Mary Shelley', 4, 0);";

                                           
    PGresult *populateDBResult1 = PQexec(conn, populateDBQuery1);
    if (PQresultStatus(populateDBResult1) != PGRES_COMMAND_OK) {
        fprintf(stderr, "Errore durante l'inserimento utenti: %s\n", PQerrorMessage(conn));
    }
    PQclear(populateDBResult1);

    PGresult *populateDBResult2 = PQexec(conn, populateDBQuery2);
    if (PQresultStatus(populateDBResult2) != PGRES_COMMAND_OK) {
        fprintf(stderr, "Errore durante l'inserimento libri: %s\n", PQerrorMessage(conn));
    }
    PQclear(populateDBResult2);

    printf("Operazioni completate con successo.\n");*/


}

    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // Creazione della socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket fallita");
        exit(EXIT_FAILURE);
    }

    printf("Socket creata con successo.\n");

    // Configurazione indirizzo e porta
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Binding della socket
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("Bind fallito");
        exit(EXIT_FAILURE);
    }

    printf("Bind eseguito sulla porta %d\n", PORT);

    // Ascolto delle connessioni
    if (listen(server_fd, 10) < 0) {
        perror("Listen fallito");
        exit(EXIT_FAILURE);
    }

    printf("Server in ascolto sulla porta %d\n", PORT);


    // Ciclo per accettare più connessioni
    while (1) {
        printf("In attesa di connessioni...\n");
        if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
            perror("Accept fallito");
            exit(EXIT_FAILURE);
        }

        printf("Nuova connessione accettata\n");

        // Creazione di un nuovo thread per gestire il client
        pthread_t client_thread;
        int *new_sock = malloc(sizeof(int));  // Risorsa per il nuovo socket
        *new_sock = new_socket;

        if (pthread_create(&client_thread, NULL, handle_client, (void*)new_sock) < 0) {
            perror("Impossibile creare il thread");
            free(new_sock);
            return 1;
        }

        printf("Handler assegnato al client\n");
    }
  
    close(server_fd);
    PQfinish(conn);

    return 0;
}









int main() {
    start();
    return 0;
} 