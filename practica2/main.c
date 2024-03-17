#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <time.h>

#define MAX_LINE_LENGTH 500
#define MAX_USERS 1000
#define MAX_OPERATIONS 1000
#define NUM_HILOS 3
#define NUM_THREADS 4
#define MAX_ERRORES_CARGA_USUARIOS 1000
#define MAX_ERRORES_CARGA_OPERACIONES 1000

// Estructura para representar un usuario
typedef struct
{
    int no_cuenta;
    char nombre[50];
    double saldo;
} Usuario;

// Estructura para representar una operación bancaria
typedef struct
{
    int tipo_operacion; // 1: Depósito, 2: Retiro, 3: Transferencia
    int cuenta_origen;
    int cuenta_destino;
    double monto;
} Operacion;

// Función para obtener la fecha actual y formatearla
void obtenerFechaHora(char *fecha)
{
    time_t rawtime;
    struct tm *info;
    time(&rawtime);
    info = localtime(&rawtime);
    strftime(fecha, 20, "%Y_%m_%d-%H_%M_%S", info);
}

typedef struct
{
    int linea;
    char mensaje[MAX_LINE_LENGTH];
} ErrorCargaUsuario;

typedef struct
{
    int linea;
    char mensaje[MAX_LINE_LENGTH];
} ErrorCargaOperacion;

Usuario usuarios[MAX_USERS];           // Arreglo para almacenar usuarios
Operacion operaciones[MAX_OPERATIONS]; // Arreglo para almacenar operaciones
int total_usuarios = 0;                // Total de usuarios cargados
int total_operaciones = 0;             // Total de operaciones cargadas
int errores_carga_usuarios = 0;
int errores_carga_operaciones = 0;
int errores_cuenta_duplicada = 0;
int errores_cuenta_no_entero = 0;
int errores_saldo_no_real = 0;

int usuarios_procesados[NUM_HILOS] = {0}; // Usuarios procesados por cada hilo

int total_retiros = 0;
int total_depositos = 0;
int total_transferencias;
int total_operaciones_realizadas = 0;
int operaciones_realizadas_por_hilo[NUM_THREADS] = {0};

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // Mutex para sincronizar acceso a datos compartidos

ErrorCargaUsuario errores_carga_usuarios_arr[MAX_ERRORES_CARGA_USUARIOS];
ErrorCargaOperacion errores_carga_operaciones_arr[MAX_ERRORES_CARGA_OPERACIONES];
int total_errores_carga_usuarios = 0;
int total_errores_carga_operaciones = 0;

// Función para escribir un error en el archivo de errores
void escribirError(const char *mensaje, int linea)
{
    // Abrir el archivo de errores
    FILE *error_file = fopen("errores.log", "a"); // Abre el archivo en modo de añadir al final
    if (error_file == NULL)
    {
        perror("Error al abrir el archivo de errores");
        return;
    }
    fprintf(error_file, "%s\n", mensaje);
    fclose(error_file);

    // Guardar el error en una estructura ErrorCargaUsuario
    errores_carga_operaciones_arr[total_errores_carga_operaciones].linea = linea;
    snprintf(errores_carga_operaciones_arr[total_errores_carga_operaciones].mensaje, MAX_LINE_LENGTH, "%s\n", mensaje);

    // Incrementar el contador de errores
    total_errores_carga_operaciones++;
}

// Función que será ejecutada por cada hilo
void *procesarUsuarios(void *args)
{
    int hilo_id = *((int *)args);
    int start_index = hilo_id * (total_usuarios / NUM_HILOS);
    int end_index = (hilo_id + 1) * (total_usuarios / NUM_HILOS);
    if (hilo_id == NUM_HILOS - 1)
        end_index = total_usuarios;

    for (int i = start_index; i < end_index; i++)
    {
        Usuario usuario = usuarios[i];

        // Validar los datos
        int error = 0;
        char causa_error[MAX_LINE_LENGTH] = "";

        if (usuario.saldo <= 0)
        {
            error = 1;
            errores_saldo_no_real++;
            strcat(causa_error, "Saldo no positivo; ");
        }

        for (int j = 0; j < total_usuarios; j++)
        {
            if (i != j && usuarios[j].no_cuenta == usuario.no_cuenta)
            {
                error = 1;
                errores_cuenta_duplicada++;
                strcat(causa_error, "Cuenta duplicada; ");
                break;
            }
        }

        if (usuario.no_cuenta <= 0)
        {
            error = 1;
            errores_cuenta_no_entero++;
            strcat(causa_error, "Número de cuenta no entero positivo; ");
        }

        if (error)
        {
            // Si hay un error, escribir el registro en el archivo de errores
            pthread_mutex_lock(&mutex);
            FILE *error_file = fopen("errores.log", "a");
            if (error_file == NULL)
            {
                perror("Error al abrir el archivo de errores");
                exit(EXIT_FAILURE);
            }
            fprintf(error_file, "Error en la línea %d: Usuario con cuenta %d: %s\n", i + 1, usuario.no_cuenta, causa_error);
            fclose(error_file);
            pthread_mutex_unlock(&mutex);

            // Guardar el error en una estructura ErrorCargaUsuario
            errores_carga_usuarios_arr[total_errores_carga_usuarios].linea = i + 1;
            snprintf(errores_carga_usuarios_arr[total_errores_carga_usuarios].mensaje, MAX_LINE_LENGTH, "Usuario con cuenta %d: %s", usuario.no_cuenta, causa_error);

            // Incrementar el contador de errores
            total_errores_carga_usuarios++;
        }
        else
        {
            pthread_mutex_lock(&mutex);                           // Bloquear el mutex antes de modificar datos compartidos
            printf("Procesando usuario %d\n", usuario.no_cuenta); // Simulando procesamiento
            pthread_mutex_unlock(&mutex);                         // Desbloquear el mutex
            usuarios_procesados[hilo_id]++;
        }
    }

    return NULL;
}

// Función para generar el reporte de carga
void generarReporte()
{
    char nombre_archivo[50];
    char fecha[20];
    obtenerFechaHora(fecha);
    sprintf(nombre_archivo, "carga_%s.log", fecha);
    FILE *archivo_registro = fopen(nombre_archivo, "w");

    if (archivo_registro == NULL)
    {
        perror("Error al crear el archivo de registro");
        pthread_exit(NULL);
    }
    fprintf(archivo_registro, "-----------------Carga de usuarios-----------------\n");
    fprintf(archivo_registro, "\n");
    fprintf(archivo_registro, "Fecha:%s\n", fecha);
    fprintf(archivo_registro, "\n");
    for (int i = 0; i < NUM_HILOS; i++)
    {
        fprintf(archivo_registro, "Hilo %d: %d usuarios cargados\n", i + 1, usuarios_procesados[i]);
    }
    fprintf(archivo_registro, "Total de usuarios cargados: %d\n", total_usuarios);

    fprintf(archivo_registro, "\nErrores en la carga:\n");
    fprintf(archivo_registro, "Cuentas duplicadas: %d\n", errores_cuenta_duplicada);
    fprintf(archivo_registro, "Cuentas no enteras positivas: %d\n", errores_cuenta_no_entero);
    fprintf(archivo_registro, "Saldos no reales positivos: %d\n", errores_saldo_no_real);

    // Escribir los errores en la carga de usuarios
    fprintf(archivo_registro, "\nErrores en la carga de usuarios:\n");
    for (int i = 0; i < total_errores_carga_usuarios; i++)
    {
        fprintf(archivo_registro, "Error en la línea %d: %s\n", errores_carga_usuarios_arr[i].linea, errores_carga_usuarios_arr[i].mensaje);
    }

    fclose(archivo_registro);
}

// Función para realizar un depósito
void realizarDeposito(int cuenta_destino, double monto, int linea)
{
    // Buscar la cuenta destino
    int index_destino = -1;
    for (int i = 0; i < total_usuarios; ++i)
    {
        if (usuarios[i].no_cuenta == cuenta_destino)
        {
            index_destino = i;
            break;
        }
    }

    // Validar si la cuenta destino existe
    if (index_destino == -1)
    {
        escribirError("Error: El número de cuenta no existe.", linea);
        return;
    }

    // Validar si el monto es válido
    if (monto <= 0)
    {
        escribirError("Error: El monto debe ser un número positivo.", linea);
        return;
    }

    // Realizar el depósito
    usuarios[index_destino].saldo += monto;
    printf("Depósito de %.2f realizado en la cuenta %d.\n", monto, cuenta_destino);
}

// Función para realizar un retiro
void realizarRetiro(int cuenta_origen, double monto, int linea)
{
    // Buscar la cuenta origen
    int index_origen = -1;
    for (int i = 0; i < total_usuarios; ++i)
    {
        if (usuarios[i].no_cuenta == cuenta_origen)
        {
            index_origen = i;
            break;
        }
    }

    // Validar si la cuenta origen existe
    if (index_origen == -1)
    {
        escribirError("Error: El número de cuenta no existe.", linea);
        return;
    }

    // Validar si el monto es válido
    if (monto <= 0)
    {
        escribirError("Error: El monto debe ser un número positivo.", linea);
        return;
    }

    // Validar si la cuenta tiene saldo suficiente
    if (usuarios[index_origen].saldo < monto)
    {
        escribirError("Error: La cuenta no tiene saldo suficiente para realizar el retiro.", linea);
        return;
    }

    // Realizar el retiro
    usuarios[index_origen].saldo -= monto;
    printf("Retiro de %.2f realizado en la cuenta %d.\n", monto, cuenta_origen);
}

// Función para realizar una transferencia
void realizarTransferencia(int num_cuenta_origen, int num_cuenta_destino, double monto, int linea)
{
    // Buscar la cuenta origen
    int index_origen = -1;
    for (int i = 0; i < total_usuarios; ++i)
    {
        if (usuarios[i].no_cuenta == num_cuenta_origen)
        {
            index_origen = i;
            break;
        }
    }

    // Validar si la cuenta origen existe
    if (index_origen == -1)
    {
        escribirError("Error: El número de cuenta no existe.", linea);
        return;
    }

    // Buscar la cuenta destino
    int index_destino = -1;
    for (int i = 0; i < total_usuarios; ++i)
    {
        if (usuarios[i].no_cuenta == num_cuenta_destino)
        {
            index_destino = i;
            break;
        }
    }

    // Validar si la cuenta destino existe
    if (index_destino == -1)
    {
        escribirError("Error: El número de cuenta no existe.", linea);
        return;
    }

    // Validar si el monto es válido
    if (monto <= 0)
    {
        escribirError("Error: El monto debe ser un número positivo.", linea);
        return;
    }

    // Validar si la cuenta origen tiene saldo suficiente
    if (usuarios[index_origen].saldo < monto)
    {
        escribirError("Error: La cuenta no tiene saldo suficiente para realizar la transferencia.", linea);
        return;
    }

    // Realizar la transferencia
    usuarios[index_origen].saldo -= monto;
    usuarios[index_destino].saldo += monto;
    printf("Transferencia de %.2f realizada de la cuenta %d a la cuenta %d.\n", monto, num_cuenta_origen, num_cuenta_destino);
}

// Función para procesar las operaciones bancarias
void *procesarOperaciones(void *args)
{
     int hilo_id = *((int *)args);
    int start_index = hilo_id * (total_operaciones / NUM_THREADS);
    int end_index = (hilo_id == NUM_THREADS - 1) ? total_operaciones : (hilo_id + 1) * (total_operaciones / NUM_THREADS);

    int errores_identificador_operacion_no_existe = 0;

    for (int i = start_index; i < end_index; i++)
    {
        Operacion operacion = operaciones[i];

        // Procesar la operación
        pthread_mutex_lock(&mutex); // Bloquear el mutex antes de modificar datos compartidos
        switch (operacion.tipo_operacion)
        {
        case 1: // Depósito
            // Aquí iría la lógica para realizar el depósito
            realizarDeposito(operacion.cuenta_destino, operacion.monto, i);
            operaciones_realizadas_por_hilo[hilo_id]++;
            total_depositos++;
            total_operaciones_realizadas++;
            break;
        case 2: // Retiro
            // Aquí iría la lógica para realizar el retiro
            realizarRetiro(operacion.cuenta_origen, operacion.monto, i);
            operaciones_realizadas_por_hilo[hilo_id]++;
            total_retiros++;
            total_operaciones_realizadas++;
            break;
        case 3: // Transferencia
            // Aquí iría la lógica para realizar la transferencia
            realizarTransferencia(operacion.cuenta_origen, operacion.cuenta_destino, operacion.monto, i);
            operaciones_realizadas_por_hilo[hilo_id]++;
            total_transferencias++;
            total_operaciones_realizadas++;
            break;
        default:
            printf("Error: Operación desconocida en la línea %d\n", i + 1);
            errores_identificador_operacion_no_existe++;
        }
        pthread_mutex_unlock(&mutex); // Desbloquear el mutex después de modificar datos compartidos
    }
    pthread_exit(NULL);
}

void crearRegistroCargaOperaciones()
{
    char nombre_archivo[50];
    char fecha[20];
    obtenerFechaHora(fecha);
    sprintf(nombre_archivo, "operaciones_%s.log", fecha);
    FILE *archivo_registro = fopen(nombre_archivo, "w");

    if (archivo_registro == NULL)
    {
        perror("Error al crear el archivo de registro operaciones");
        pthread_exit(NULL);
    }
    fprintf(archivo_registro, "-----------------Carga de operaciones-----------------\n");
    fprintf(archivo_registro, "\n");
    fprintf(archivo_registro, "Fecha:%s\n", fecha);
    fprintf(archivo_registro, "\n");
    for (int i = 0; i < NUM_THREADS; i++)
    {
        fprintf(archivo_registro, "Hilo %d: %d operaciones procesadas\n", i + 1, operaciones_realizadas_por_hilo[i]);
    }
    fprintf(archivo_registro, "\nOperaciones realizadas:\n");
    fprintf(archivo_registro, "Retiros: %d\n", total_retiros);
    fprintf(archivo_registro, "Depositos: %d\n", total_depositos);
    fprintf(archivo_registro, "Transferencias: %d\n", total_transferencias);
    fprintf(archivo_registro, "Total de operaciones cargadas: %d\n", total_operaciones_realizadas);


    // Escribir los errores en la carga de usuarios
    fprintf(archivo_registro, "\nErrores en las operaciones:\n");
    for (int i = 0; i < total_errores_carga_operaciones; i++)
    {
        fprintf(archivo_registro, "Error en la línea %d: %s\n", errores_carga_operaciones_arr[i].linea, errores_carga_operaciones_arr[i].mensaje);
    }

    fclose(archivo_registro);
}

// Función para cargar operaciones desde un archivo CSV
void cargarOperacionesDesdeArchivo()
{
    char filename[MAX_LINE_LENGTH];
    printf("Ingrese la ruta del archivo CSV de operaciones: ");
    scanf("%s", filename);

    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        perror("Error al abrir el archivo de operaciones");
        return;
    }

    // Leer el archivo CSV y cargar las operaciones en memoria
    char line[MAX_LINE_LENGTH];
    int line_number = 0;
    while (fgets(line, sizeof(line), file))
    {
        line_number++;
        int tipo_operacion, cuenta_origen, cuenta_destino;
        double monto;

        if (sscanf(line, "%d,%d,%d,%lf", &tipo_operacion, &cuenta_origen, &cuenta_destino, &monto) != 4)
        {
            printf("Error al leer la línea %d del archivo de operaciones: Formato incorrecto\n", line_number);
            errores_carga_operaciones++;
            continue;
        }

        // Almacenar la operación en el arreglo
        operaciones[total_operaciones].tipo_operacion = tipo_operacion;
        operaciones[total_operaciones].cuenta_origen = cuenta_origen;
        operaciones[total_operaciones].cuenta_destino = cuenta_destino;
        operaciones[total_operaciones].monto = monto;
        total_operaciones++;
    }

    fclose(file);

    // Crear los hilos para procesar las operaciones
    pthread_t thread_ids_operaciones[NUM_THREADS];
    int hilo_ids_operaciones[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; i++)
    {
        hilo_ids_operaciones[i] = i;
        if (pthread_create(&thread_ids_operaciones[i], NULL, procesarOperaciones, &hilo_ids_operaciones[i]) != 0)
        {
            perror("Error al crear el hilo de operaciones");
            exit(EXIT_FAILURE);
        }
    }

    // Esperar a que todos los hilos de operaciones terminen
    for (int i = 0; i < NUM_THREADS; i++)
    {
        if (pthread_join(thread_ids_operaciones[i], NULL) != 0)
        {
            perror("Error al esperar el hilo de operaciones");
            exit(EXIT_FAILURE);
        }
    }

    // Generar el reporte de carga de operaciones
    printf("\nReporte de carga de operaciones:\n");
    printf("Total de operaciones cargadas: %d\n", total_operaciones);
    printf("Errores en la carga de operaciones: %d\n", errores_carga_operaciones);

    // Crear el archivo de registro de carga de operaciones
    crearRegistroCargaOperaciones();
}

// Función para realizar un depósito en una cuenta
void deposito(int num_cuenta, double monto)
{
    for (int i = 0; i < total_usuarios; i++)
    {
        if (usuarios[i].no_cuenta == num_cuenta)
        {
            pthread_mutex_lock(&mutex);
            usuarios[i].saldo += monto;
            printf("Depósito de %.2f realizado en la cuenta %d\n", monto, num_cuenta);
            pthread_mutex_unlock(&mutex);
            return;
        }
    }
    printf("Error: No existe la cuenta %d\n", num_cuenta);
}

// Función para realizar un retiro en una cuenta
void retiro(int num_cuenta, double monto)
{
    for (int i = 0; i < total_usuarios; i++)
    {
        if (usuarios[i].no_cuenta == num_cuenta)
        {
            pthread_mutex_lock(&mutex);
            if (usuarios[i].saldo >= monto)
            {
                usuarios[i].saldo -= monto;
                printf("Retiro de %.2f realizado en la cuenta %d\n", monto, num_cuenta);
            }
            else
            {
                printf("Error: Saldo insuficiente en la cuenta %d\n", num_cuenta);
            }
            pthread_mutex_unlock(&mutex);
            return;
        }
    }
    printf("Error: No existe la cuenta %d\n", num_cuenta);
}

// Función para realizar una transacción entre cuentas
void transaccion(int num_cuenta_origen, int num_cuenta_destino, double monto)
{
    int i, j;
    for (i = 0; i < total_usuarios; i++)
    {
        if (usuarios[i].no_cuenta == num_cuenta_origen)
            break;
    }
    for (j = 0; j < total_usuarios; j++)
    {
        if (usuarios[j].no_cuenta == num_cuenta_destino)
            break;
    }

    if (i == total_usuarios)
    {
        printf("Error: No existe la cuenta de origen %d\n", num_cuenta_origen);
        return;
    }
    if (j == total_usuarios)
    {
        printf("Error: No existe la cuenta de destino %d\n", num_cuenta_destino);
        return;
    }

    pthread_mutex_lock(&mutex);
    if (usuarios[i].saldo >= monto)
    {
        usuarios[i].saldo -= monto;
        usuarios[j].saldo += monto;
        printf("Transacción de %.2f realizada de la cuenta %d a la cuenta %d\n", monto, num_cuenta_origen, num_cuenta_destino);
    }
    else
    {
        printf("Error: Saldo insuficiente en la cuenta %d\n", num_cuenta_origen);
    }
    pthread_mutex_unlock(&mutex);
}

// Función para consultar el saldo de una cuenta
void consultarCuenta(int num_cuenta)
{
    for (int i = 0; i < total_usuarios; i++)
    {
        if (usuarios[i].no_cuenta == num_cuenta)
        {
            printf("Información de la cuenta %d:\n", num_cuenta);
            printf("Nombre: %s\n", usuarios[i].nombre);
            printf("Saldo: %.2f\n", usuarios[i].saldo);
            return;
        }
    }
    printf("Error: No existe la cuenta %d\n", num_cuenta);
}

int main()
{
    char filename[MAX_LINE_LENGTH];
    printf("Ingrese la ruta del archivo CSV: ");
    fgets(filename, sizeof(filename), stdin);
    // Eliminar el salto de línea del final
    filename[strcspn(filename, "\n")] = 0;

    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        perror("Error al abrir el archivo");
        return EXIT_FAILURE;
    }

    // Leer el archivo y cargar los datos en memoria
    char line[MAX_LINE_LENGTH];
    int line_number = 0;
    while (fgets(line, sizeof(line), file))
    {
        line_number++;
        int no_cuenta;
        char nombre[50];
        double saldo;

        if (sscanf(line, "%d,%49[^,],%lf", &no_cuenta, nombre, &saldo) != 3)
        {
            printf("Error al leer la línea %d: %s\n", line_number, line);
            continue;
        }

        usuarios[total_usuarios].no_cuenta = no_cuenta;
        strcpy(usuarios[total_usuarios].nombre, nombre);
        usuarios[total_usuarios].saldo = saldo;
        total_usuarios++;
    }

    fclose(file);

    // Crear tres hilos para procesar los usuarios
    pthread_t thread_ids[NUM_HILOS];
    int hilo_ids[NUM_HILOS];
    for (int i = 0; i < NUM_HILOS; i++)
    {
        hilo_ids[i] = i;
        if (pthread_create(&thread_ids[i], NULL, procesarUsuarios, &hilo_ids[i]) != 0)
        {
            perror("Error al crear el hilo");
            exit(EXIT_FAILURE);
        }
    }

    // Esperar a que todos los hilos terminen
    for (int i = 0; i < NUM_HILOS; i++)
    {
        if (pthread_join(thread_ids[i], NULL) != 0)
        {
            perror("Error al esperar el hilo");
            exit(EXIT_FAILURE);
        }
    };

    generarReporte();

    // Menú de operaciones
    int opcion;
    int num_cuenta;
    double monto;
    int num_cuenta_destino;
    do
    {
        printf("\nMenu de Operaciones:\n");
        printf("1. Deposito\n");
        printf("2. Retiro\n");
        printf("3. Transacción\n");
        printf("4. Consultar cuenta\n");
        printf("5. Carga de operacion\n");
        printf("6. Salir\n");
        printf("Seleccione una opción: ");
        scanf("%d", &opcion);

        switch (opcion)
        {
        case 1:
            printf("Ingrese el numero de cuenta: ");
            scanf("%d", &num_cuenta);
            printf("Ingrese el monto a depositar: ");
            scanf("%lf", &monto);
            deposito(num_cuenta, monto);
            break;
        case 2:
            printf("Ingrese el numero de cuenta: ");
            scanf("%d", &num_cuenta);
            printf("Ingrese el monto a retirar: ");
            scanf("%lf", &monto);
            retiro(num_cuenta, monto);
            break;
        case 3:
            printf("Ingrese el numero de cuenta de origen: ");
            scanf("%d", &num_cuenta);
            printf("Ingrese el numero de cuenta de destino: ");
            scanf("%d", &num_cuenta_destino);
            printf("Ingrese el monto a transferir: ");
            scanf("%lf", &monto);
            transaccion(num_cuenta, num_cuenta_destino, monto);
            break;
        case 4:
            printf("Ingrese el numero de cuenta a consultar: ");
            scanf("%d", &num_cuenta);
            consultarCuenta(num_cuenta);
            break;
        case 5:
            // Cargar operaciones desde archivo
            cargarOperacionesDesdeArchivo();
            break;
        case 6:
            printf("Saliendo...\n");
            break;
        default:
            printf("Opción inválida\n");
        }
    } while (opcion != 6);

    return 0;
}
