DO $$
BEGIN
    IF NOT EXISTS (
        SELECT 1 FROM pg_database WHERE datname = 'library'
    ) THEN
        CREATE DATABASE library;
    END IF;
END $$;

\connect library;

-- Verifica l'utente e permessi
ALTER USER postgres WITH PASSWORD '12345';



-- Creazione del tipo genere
CREATE TYPE genere AS ENUM ('classico', 'fantasy', 'storico');



-- Creazione tabella utenti
CREATE TABLE IF NOT EXISTS users (
    id SERIAL PRIMARY KEY,
    username VARCHAR(255) UNIQUE NOT NULL,
    password VARCHAR(255) NOT NULL
);

-- Aggiungi un RAISE NOTICE per capire se lo script arriva a questo punto

-- Creazione tabella libri
CREATE TABLE IF NOT EXISTS books (
    isbn VARCHAR(20) PRIMARY KEY,
    titolo VARCHAR(50) NOT NULL,
    genere genere NOT NULL,  -- Usa il tipo ENUM appena creato
    imageUrl VARCHAR(100) NOT NULL,
    autore VARCHAR(50) NOT NULL,
    quantita INTEGER NOT NULL,
    copieprestate INTEGER NOT NULL
);


INSERT INTO users (username, password) VALUES ('admin', 'admin');

-- Inserimento di dati iniziali nella tabella Books

INSERT INTO books (isbn, titolo, genere, imageUrl, autore, quantita, copieprestate) VALUES 
    ('9788817154857', '1984', 'classico', 'https://covers.openlibrary.org/b/id/12054715-M.jpg', 'George Orwell', 5, 0), 
    ('9780547928227', 'The Hobbit', 'fantasy', 'https://covers.openlibrary.org/b/id/14625529-M.jpg', 'J.R.R. Tolkien', 4, 0), 
    ('9780140447934', 'War and Peace', 'classico', 'https://covers.openlibrary.org/b/id/14598041-M.jpg', 'Leo Tolstoy', 3, 0), 
    ('9780307277671', 'The Da Vinci Code', 'fantasy', 'https://covers.openlibrary.org/b/id/14553572-M.jpg', 'Dan Brown', 6, 0), 
    ('9788868360269', 'It', 'classico', 'https://covers.openlibrary.org/b/id/14656946-M.jpg', 'Stephen King', 4, 0), 
    ('9780143105428', 'Pride and Prejudice', 'classico', 'https://covers.openlibrary.org/b/id/14619629-M.jpg', 'Jane Austen', 5, 0), 
    ('9780307949486', 'The Girl with the Dragon Tattoo', 'fantasy', 'https://covers.openlibrary.org/b/id/6779579-M.jpg', 'Stieg Larsson', 5, 0), 
    ('9798392253876', 'The Great Gatsby', 'classico', 'https://covers.openlibrary.org/b/id/8248481-M.jpg', 'F. Scott Fitzgerald', 6, 0), 
    ('9788853019356', 'Treasure Island', 'storico', 'https://covers.openlibrary.org/b/id/12819540-M.jpg', 'Robert Louis Stevenson', 7, 0), 
    ('9788806228644', 'Frankenstein', 'classico', 'https://covers.openlibrary.org/b/id/12991957-M.jpg', 'Mary Shelley', 4, 0);
