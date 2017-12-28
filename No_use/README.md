# interdependent-sim
Código para correr simulaciones de ataques a redes interdependientes.

# Dependencias
* [Python 2.7](https://www.python.org)
* [numpy](http://www.numpy.org/)
* [python-igraph](http://igraph.org/python/)

# Modo de uso
Hay dos scripts para correr simulaciones en base a parámetros definidos por el usuario.

## Ejecución simple con run_script.py
Corre una instancia de simulación de ataques recibiendo todos los parámetros del experimento por consola de la siguiente forma:

```
usage: run_script.py [-h] [-ln LOGICNODES] [-pn PHYSICALNODES]
                     [-ia INTERDEPENDENCEAMOUNT] [-ls LOGICSUPPLIERS]
                     [-e EXPONENTPG] [-x XCOORDINATE] [-y YCOORDINATE]
                     [-v VERSION] [-r]

Run experiments with the given variables

optional arguments:
  -h, --help            show this help message and exit
  -ln LOGICNODES, --logicnodes LOGICNODES
                        amount of nodes in the logic network
  -pn PHYSICALNODES, --physicalnodes PHYSICALNODES
                        amount of nodes in the physical network
  -ia INTERDEPENDENCEAMOUNT, --interdependenceamount INTERDEPENDENCEAMOUNT
                        maximum amount of interconnections
  -ls LOGICSUPPLIERS, --logicsuppliers LOGICSUPPLIERS
                        amount of suppliers in the logic network
  -e EXPONENTPG, --exponentpg EXPONENTPG
                        lambda exponent for logic network Power-Law
  -x XCOORDINATE, --xcoordinate XCOORDINATE
                        width of the physical space for the physical network
  -y YCOORDINATE, --ycoordinate YCOORDINATE
                        length of the physical space for the physical network
  -v VERSION, --version VERSION
                        version for this kind of interdependent systems
  -r, --read            If this is specified will read the networks from file

```
El nombre de los archivos que contienen las redes generadas y los resultados de los ataques son definidos automáticamente a partir de los parámetros del experimento. Las redes se guardan en la carpeta `networks` y los resultados en la carpeta `test_results`, ambas carpetas serán creadas si no existen previamente.

Si se especifica la opción `-r` se intentará leer desde archivo las redes en vez de generarlas, el programa buscará en la carpeta `networks` los archivos con el nombre autogenerado correspondiente.

Para probar que el programa funciona correctamente se puede usar la siguiente configuración:
```
python run_script.py -ln 10 -pn 20 -ia 3 -ls 3 -e 2.5 -x 400 -y 400 -v 1
```
Al ejecutar de imprimirá en pantalla cuando el programa termine las distintas fases del experimento (generar redes, ataques, etc). Al terminar la ejecución se pueden ver los siguientes archivos en la carpeta `networks`:
```
dependence_400x400_exp_2.5_ndep_3_lprovnum_3_v1.csv
physic_400x400_exp_2.5_ndep_3_lprovnum_3_v1.csv
logic_400x400_exp_2.5_ndep_3_lprovnum_3_v1.csv
providers_400x400_exp_2.5_ndep_3_lprovnum_3_v1.csv
```
Y los siguientes archivos en la carpeta `test_results`:
```
result_400x400_exp_2.5_ndep_3_att_both_lprovnum_3_v1.csv
result_400x400_exp_2.5_ndep_3_att_logic_lprovnum_3_v1.csv
result_400x400_exp_2.5_ndep_3_att_physical_lprovnum_3_v1.csv
```
Ahora se puede correr nuevamente usando la opción `-r`:
 ```
 python run_script.py -ln 10 -pn 20 -ia 3 -ls 3 -e 2.5 -x 400 -y 400 -v 1 -r
 ```
 Al ejecutar el programa usará las redes pre-existentes en vez de crearlas de nuevo.
 ## Ejecución múltiple con concurrent_run.py
 Este script permite correr múltiples experimentos simultáneamente de la siguiente manera:
 ```
 usage: concurrent_run.py [-h] [-w WORKERS] [-f FROM_FILE]

 Run experiments concurrently

 optional arguments:
   -h, --help            show this help message and exit
   -w WORKERS, --workers WORKERS
                         amount of concurrent workers, if empty will default to
                         number of cpus
   -f FROM_FILE, --from_file FROM_FILE
                         filename containing the tasks parameters, if empty
                         will use static parameters
 ```
Se puede especificar la cantidad de threads concurrentes con la opción `-w`, si no se especifica se intentarán usar todos los cores de la máquina.

Para definir los parámetros de los experimentos a ejecutar se puede usar un archivo que utilice la misma sintaxis que en el script anterior, por ejemplo:
```
-ln 10 -pn 20 -ia 3 -ls 3 -e 2.5 -x 400 -y 400 -v 1
-ln 10 -pn 20 -ia 3 -ls 3 -e 2.5 -x 400 -y 400 -v 2
-ln 10 -pn 20 -ia 3 -ls 3 -e 2.5 -x 400 -y 400 -v 3
```
Cada línea debe contener los parámetros para un experimento. Si guardamos las líneas del ejemplo en un archivo `args.txt` y ejecutamos:
```
python concurrent_run.py -w 2 -f args.txt
```
Se ejecutarán los correrán los tres experimentos, primero se ejecutaran dos de manera simultánea y cuando alguno termine se ejecutará el último.

Cuando termine la ejecución de todos los experimentos se pueden ver sus resultados en las carpetas `networks` y `test_results`.

Al igual que en el caso anterior se pueden leer las redes desde archivo usando la opción `-r` en cada línea.
