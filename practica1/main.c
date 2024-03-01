#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h> // Inclusión de fcntl.h
#include <time.h>

#define LOG_FILE "syscalls.log"
#define FILE_NAME "practica1.txt"
#define CHILD_PIDS_FILE "child_pids.txt"

// Variables globales para contar las llamadas al sistema por tipo
int total_calls = 0;
int read_calls = 0;
int write_calls = 0;

// Función para manejar la señal SIGINT (Ctrl + C)
void sigint_handler(int signum) {
    printf("\nSeñal SIGINT recibida. Terminando...\n");
    printf("Número total de llamadas al sistema: %d\n", total_calls);
    printf("Número de llamadas al sistema por tipo (Read: %d, Write: %d)\n", read_calls, write_calls);
    exit(0);
}

int main() {
    // Instalar el manejador de señal para SIGINT
    signal(SIGINT, sigint_handler);

    // Crear o vaciar el archivo de registro
    FILE *log_file = fopen(LOG_FILE, "w");
    fclose(log_file);

    // Crear o vaciar el archivo de los PID de los procesos hijos
    FILE *child_pids_file = fopen(CHILD_PIDS_FILE, "w");
    fclose(child_pids_file);

    // Crear dos procesos hijos
    for (int i = 0; i < 2; i++) {
        pid_t pid = fork();

        if (pid == -1) {
            // Error al crear el proceso hijo
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            // Código para el proceso hijo
            // Crear o abrir el archivo para escritura
            int file_desc = open(FILE_NAME, O_WRONLY | O_CREAT | O_TRUNC, 0666);
            if (file_desc == -1) {
                perror("open");
                exit(EXIT_FAILURE);
            }

            // Escribir el PID del proceso hijo en el archivo
            FILE *child_pids_file = fopen(CHILD_PIDS_FILE, "a");
            fprintf(child_pids_file, "%d\n", getpid());
            fclose(child_pids_file);

            // Semilla para generar números aleatorios
            srand(time(NULL) ^ getpid());

            while (1) {
                // Realizar llamadas al sistema de manera aleatoria
                int syscall_choice = rand() % 3;
                switch (syscall_choice) {
                    case 0: {
                        // Escribir una línea de texto aleatoria con 8 caracteres alfanuméricos
                        char buffer[9];
                        for (int i = 0; i < 8; i++) {
                            buffer[i] = 'A' + rand() % 26; // Caracteres alfabéticos aleatorios
                        }
                        buffer[8] = '\n'; // Agregar un salto de línea al final
                        write(file_desc, buffer, 9);
                        printf("Proceso %d: Write (fecha y hora)\n", getpid());
                        write_calls++;
                        break;
                    }
                    case 1: {
                        // Leer 8 caracteres
                        char buffer[9];
                        read(file_desc, buffer, 8);
                        buffer[8] = '\0'; // Añadir el carácter nulo al final para imprimir como cadena
                        printf("Proceso %d: Read (fecha y hora)\n", getpid());
                        read_calls++;
                        break;
                    }
                    case 2:
                        // Reposicionar el puntero de lectura/escritura al inicio del archivo
                        lseek(file_desc, 0, SEEK_SET);
                        printf("Proceso %d: Seek (fecha y hora)\n", getpid());
                        break;
                }

                // Incrementar el contador de llamadas totales
                total_calls++;

                // Esperar un tiempo aleatorio entre 1 y 3 segundos
                sleep(rand() % 3 + 1);
            }

            // Cerrar el archivo
            close(file_desc);
        }
    }

    // Esperar la señal SIGINT
    while (1) {
        pause();
    }

    return 0;
}
