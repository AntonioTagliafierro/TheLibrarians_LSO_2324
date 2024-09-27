# Usa l'immagine gcc come base
FROM ubuntu:latest

ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=Europe/Rome
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

# Installa le dipendenze necessarie per PostgreSQL
RUN apt-get update && apt-get install -y \
    gcc \
    make \
    libpq-dev \
    postgresql-all -y

WORKDIR /app

COPY server.c /app
COPY start.sh /app/start.sh
# Compila il server
RUN gcc -o server server.c -I/usr/include/postgresql -lpq

# Esponi la porta del server
EXPOSE 8080

# Avvia il server quando il container viene avviato
CMD ["./bin/bash"]

