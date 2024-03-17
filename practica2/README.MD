## Manual Técnico del Código

### Introducción
El código proporcionado es un programa en C que simula el funcionamiento de un sistema bancario básico. Permite cargar información de usuarios desde un archivo CSV, procesar dicha información, realizar operaciones bancarias como depósitos, retiros y transferencias, generar reportes y guardar registros en archivos CSV.

### Estructuras de Datos
1. **Usuario**: Representa la información de un usuario bancario, incluyendo el número de cuenta, nombre y saldo.
    - Atributos:
        - `no_cuenta` (int): Número de cuenta del usuario.
        - `nombre` (char[50]): Nombre del usuario.
        - `saldo` (double): Saldo del usuario.
2. **Operacion**: Representa una operación bancaria, incluyendo el tipo de operación, cuentas involucradas y monto.
    - Atributos:
        - `tipo_operacion` (int): Tipo de operación (depósito, retiro o transferencia).
        - `cuenta_origen` (int): Número de cuenta origen de la operación.
        - `cuenta_destino` (int): Número de cuenta destino de la operación (en caso de transferencia).
        - `monto` (double): Monto de la operación.
3. **ErrorCargaUsuario**: Estructura para almacenar errores durante la carga de usuarios.
    - Atributos:
        - `linea` (int): Número de línea donde ocurrió el error.
        - `mensaje` (char[MAX_LINE_LENGTH]): Mensaje de error.
4. **ErrorCargaOperacion**: Estructura para almacenar errores durante la carga de operaciones.
    - Atributos:
        - `linea` (int): Número de línea donde ocurrió el error.
        - `mensaje` (char[MAX_LINE_LENGTH]): Mensaje de error.

### Variables Globales
- **Arreglos de Usuarios y Operaciones**: Almacenan la información de los usuarios y operaciones bancarias respectivamente.
- **Contadores y Marcadores de Errores**: Mantienen un seguimiento del número total de usuarios, operaciones, y diversos tipos de errores durante la carga y procesamiento de datos.
- **Arreglos para Almacenar Errores**: Guardan información detallada sobre los errores encontrados durante la carga de usuarios y operaciones.
- **Variables para Seguimiento de Operaciones Realizadas**: Mantienen un seguimiento del número total de depósitos, retiros, transferencias y operaciones realizadas por cada hilo.
- **Mutex**: Proporciona sincronización para el acceso a datos compartidos entre hilos.

### Funciones Principales
1. **obtenerFechaHora**: Obtiene la fecha y hora actual y la formatea.
2. **procesarUsuarios**: Función ejecutada por cada hilo para procesar usuarios cargados.
3. **generarReporte**: Genera un reporte de carga de usuarios.
4. **procesarOperaciones**: Función ejecutada por cada hilo para procesar operaciones bancarias.
5. **crearRegistroCargaOperaciones**: Crea un registro de la carga de operaciones.
6. **cargarOperacionesDesdeArchivo**: Carga operaciones desde un archivo CSV y procesa dichas operaciones utilizando múltiples hilos.
7. **deposito, retiro, transaccion, consultarCuenta**: Funciones para realizar operaciones bancarias y consultar saldos.
8. **guardarRegistrosCSV**: Guarda los registros de las cuentas en un archivo CSV.

### Main Function
La función `main` es la función principal del programa que controla el flujo de ejecución. Realiza las siguientes acciones:
- Carga la información de usuarios desde un archivo CSV.
- Procesa la información de usuarios utilizando múltiples hilos.
- Genera un reporte de carga de usuarios.
- Presenta un menú de operaciones que permite al usuario realizar diversas operaciones bancarias.
- Permite cargar operaciones adicionales desde un archivo CSV y procesarlas utilizando múltiples hilos.
- Permite guardar registros de cuentas en un archivo CSV.
- El programa se ejecuta hasta que el usuario decide salir del menú.

### Conclusiones
El programa implementa un sistema básico de gestión bancaria utilizando el lenguaje de programación C y conceptos de concurrencia mediante el uso de hilos. Ofrece funcionalidades para cargar usuarios y operaciones desde archivos CSV, realizar operaciones bancarias y generar reportes. Además, utiliza técnicas de sincronización para garantizar la integridad de los datos compartidos entre hilos.
