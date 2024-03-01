#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

#define FILENAME "practica1.txt"
#define MAX_STRING_LENGTH 9 // 8 caracteres + '\n'

char generate_random_char() {
    return 'A' + rand() % 26; // Genera un caracter aleatorio entre 'A' y 'Z'
}

void write_to_file() {
    // Abrir el archivo en modo de escritura, creándolo si no existe
    int fd = open(FILENAME, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    // Generar una línea de texto con caracteres aleatorios
    char line[MAX_STRING_LENGTH];
    for (int i = 0; i < MAX_STRING_LENGTH - 1; i++) {
        line[i] = generate_random_char();
    }
    line[MAX_STRING_LENGTH - 1] = '\n'; // Agregar un salto de línea al final

    // Escribir la línea en el archivo
    if (write(fd, line, MAX_STRING_LENGTH) == -1) {
        perror("write");
        exit(EXIT_FAILURE);
    }

    // Cerrar el archivo
    if (close(fd) == -1) {
        perror("close");
        exit(EXIT_FAILURE);
    }
}

int main() {
    srand(time(NULL));

    while (1) {
        int random = rand() % 3;
        switch (random) {
            case 0:
                // Write
                write_to_file();
                break;
            case 1:
                // Read (no se implementa para mantenerlo simple)
                break;
            case 2:
                // Seek (no se implementa para mantenerlo simple)
                break;
            default:
                break;
        }

        // Esperar un tiempo aleatorio entre 1 y 3 segundos antes de realizar la próxima llamada
        unsigned int sleep_time = 1 + rand() % 3;
        sleep(sleep_time);
    }

    return 0;
}
