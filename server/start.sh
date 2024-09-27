#!/bin/bash

# Aspetta che il database sia pronto a ricevere connessioni
wait_for_db() {
    while ! pg_isready -h database -p 5432 -q -U postgres; do
        echo "Il database non è ancora pronto, aspettando..."
        sleep 1
    done
    echo "Il database è pronto per le connessioni."
}

# Esegui la funzione di attesa del database
wait_for_db

# Avvia il server
./server
