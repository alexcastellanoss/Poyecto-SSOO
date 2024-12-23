#include<stdio.h>
#include<string.h>
#include<ctype.h>
#include<stdlib.h>
#include "cabeceras.h"

#define LONGITUD_COMANDO 100
#define NUM_COMANDOS 8

char *listaComandos[NUM_COMANDOS] = {"dir","info","bytemaps","rename","imprimir","remove","copy","salir"};
void Printbytemaps(EXT_BYTE_MAPS *ext_bytemaps);
int ComprobarComando(char *strcomando, char *orden, char *argumento1, char *argumento2);
int palabraEnLista(char *palabra, char **lista, int num_elementos);
char* leeLinea(int tam);
void LeeSuperBloque(EXT_SIMPLE_SUPERBLOCK *psup);
int BuscaFich(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombre);
void Directorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos);
int Renombrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombreantiguo, char *nombrenuevo);
int Imprimir(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_DATOS *memdatos, char *nombre);
int Borrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock, char *nombre,  FILE *fich);
int Copiar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock, EXT_DATOS *memdatos, char *nombreorigen, char *nombredestino,  FILE *fich);

/*
void Grabarinodosydirectorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, FILE *fich);
void GrabarByteMaps(EXT_BYTE_MAPS *ext_bytemaps, FILE *fich);
void GrabarSuperBloque(EXT_SIMPLE_SUPERBLOCK *ext_superblock, FILE *fich);
void GrabarDatos(EXT_DATOS *memdatos, FILE *fich);
*/

int main(){
	char *comando;
	char *orden;
	char *argumento1;
	char *argumento2;
	 
	int grabardatos = 0;
	EXT_SIMPLE_SUPERBLOCK ext_superblock;
	EXT_BYTE_MAPS ext_bytemaps;
	EXT_BLQ_INODOS ext_blq_inodos;
	EXT_ENTRADA_DIR directorio[MAX_FICHEROS];
	EXT_DATOS memdatos[MAX_BLOQUES_DATOS];
	EXT_DATOS datosfich[MAX_BLOQUES_PARTICION];
	FILE *fent;
	 
	 // Lectura del fichero completo de una sola vez
	fent = fopen("particion.bin","r+b");
	if (fent == NULL) {
		printf("Error: No se pudo abrir el archivo particion.bin\n");
		return 1;
	}
	 fread(&datosfich, SIZE_BLOQUE, MAX_BLOQUES_PARTICION, fent);
	 
	memcpy(&ext_superblock, (EXT_SIMPLE_SUPERBLOCK *)&datosfich[0], SIZE_BLOQUE);
	memcpy(&directorio, (EXT_ENTRADA_DIR *)&datosfich[3], SIZE_BLOQUE);
	memcpy(&ext_bytemaps, (EXT_BYTE_MAPS *)&datosfich[1], SIZE_BLOQUE);
	memcpy(&ext_blq_inodos, (EXT_BLQ_INODOS *)&datosfich[2], SIZE_BLOQUE);
	memcpy(&memdatos, (EXT_DATOS *)&datosfich[4], MAX_BLOQUES_DATOS * SIZE_BLOQUE);
	 
	 // Bucle de tratamiento de comandos
	 for (;;) {
		 do {
			 printf(">> ");
			 fflush(stdin);
			 comando = leeLinea(LONGITUD_COMANDO);
			 orden = strtok(comando, " ");
			argumento1 = strtok(NULL, " ");
			argumento2 = strtok(NULL, " ");
		 } while (ComprobarComando(comando, orden, argumento1, argumento2) != 0);
		 
		 if (strcmp(orden, "dir") == 0) {
			 Directorio(directorio, &ext_blq_inodos);
		 }
		 else if(strcmp(orden, "info") == 0) {
			 LeeSuperBloque(&ext_superblock);
		 }
		 else if(strcmp(orden, "bytemaps") == 0) {
			 Printbytemaps(&ext_bytemaps);
		 }
		 else if(strcmp(orden, "rename") == 0) {
			 if (Renombrar(directorio, &ext_blq_inodos, argumento1, argumento2) == -1) {
				 printf("Error: no se pudo renombrar el fichero\n");
			 }
		
		 }
		 else if(strcmp(orden, "imprimir") == 0) {
			 if (Imprimir(directorio, &ext_blq_inodos, memdatos, argumento1) == -1) {
				 printf("Error: no se pudo imprimir el contenido del fichero\n");
			 }
		
		 }
		 else if(strcmp(orden, "remove") == 0) {
			 if (Borrar(directorio, &ext_blq_inodos, &ext_bytemaps, &ext_superblock, argumento1, fent) == -1) {
				 printf("Error: no se pudo eliminar el fichero\n");
			 }
			 grabardatos = 1;
		 } 
		 else if(strcmp(orden, "copy") == 0) {
			 if (Copiar(directorio, &ext_blq_inodos, &ext_bytemaps, &ext_superblock, memdatos, argumento1, argumento2, fent) == -1) {
				 printf("Error: no se pudo copiar el fichero\n");
			 }
			 grabardatos = 1;
		 } 
		 else if(strcmp(orden, "salir") == 0) {
			 fclose(fent);
			 return 0;
		}
	}
}

void Printbytemaps(EXT_BYTE_MAPS *ext_bytemaps) {
    int i;

    // Mostrar bytemap de inodos
    printf("Inodos :");
    for (i = 0; i < MAX_INODOS; i++) {
        printf(" %d", ext_bytemaps->bmap_inodos[i]);
    }
    printf("\n");

    // Mostrar los primeros 25 elementos del bytemap de bloques
    printf("Bloques [0-25]:");
    for (i = 0; i < 25; i++) {
        printf(" %d", ext_bytemaps->bmap_bloques[i]);
    }
    printf("\n");
}

int ComprobarComando(char *strcomando, char *orden, char *argumento1, char *argumento2) {
    int resultado = 1; 
    // Verifica si la orden está en la lista de comandos válidos.
    if (palabraEnLista(orden, listaComandos, NUM_COMANDOS) == 1) {
        resultado = 0;  // Comando válido.
    } else {
        // Mensaje de error para comandos ilegales.
        printf("ERROR: Comando ilegal [bytemaps, copy, dir, info, imprimir, rename, remove, salir]\n");
    }
    return resultado;
}

// Definición de la función
int palabraEnLista(char *palabra, char **lista, int num_elementos) {
	int resultado = 0; 
    for (int i = 0; i < num_elementos; i++) {
        if (strcmp(palabra, lista[i]) == 0) {
            resultado = 1;  // La palabra está en la lista
        }
    }
    return resultado;  //si continua siendo 0 la palabra no está en la lista
}

char *leeLinea(int tam)
{
   int i = 0;
   char c;
   char *resultado = (char *)malloc(tam);
   do
   {
      c = getchar();
      if(c != '\n')
      {
        resultado[i++] = c;
      }
   }
   while(c != '\n' && i < tam);
   resultado[i] = '\0';
   return resultado;
}

void LeeSuperBloque(EXT_SIMPLE_SUPERBLOCK *psup) {
    printf("Informacion del superbloque:\n");
    printf("Bloques libres: %d\n", psup->s_free_blocks_count);
    printf("Inodos libres: %d\n", psup->s_free_inodes_count);
    printf("Tamano de la particion: %d bloques\n", psup->s_blocks_count);
    printf("Primer bloque de datos: %d\n", psup->s_first_data_block);
}

int BuscaFich(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombre) {
	int resultado = -1; // Devuelve -1 si no se encuentra el fichero
    for (int i = 0; i < MAX_FICHEROS; i++) {
        // Verificamos si el fichero no está vacío y si el nombre coincide con el fichero que estamos buscando
        if (directorio[i].dir_inodo != NULL_INODO && strcmp(directorio[i].dir_nfich, nombre) == 0) {
            resultado = i; // Devuelve el índice del fichero encontrado
        }
    }
    return resultado; 
}

void Directorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *bloque_inodos) {

    printf("Listado de ficheros en el directorio:\n");

    for (int i = 1; i < MAX_FICHEROS; i++) {
        // Si el inodo es diferente de BloqueNullo y el nombre no está vacío accedemos al inodo correspondiente dentro del bloque
        if (directorio[i].dir_inodo != NULL_BLOQUE && strlen(directorio[i].dir_nfich) > 0) {
            EXT_SIMPLE_INODE *inodo = &bloque_inodos->blq_inodos[directorio[i].dir_inodo];

            printf("Nombre: %s,\t Inodo: %d,\t Tamano: %d bytes,\t Bloques: ", 
                   directorio[i].dir_nfich, directorio[i].dir_inodo, 
                   inodo->size_fichero);

            // Imprimimos los bloques que ocupa el archivo
            for (int j = 0; j < MAX_NUMS_BLOQUE_INODO; j++) {
                if (inodo->i_nbloque[j] != NULL_BLOQUE) {
                    printf("%d ", inodo->i_nbloque[j]);
                }
            }
            printf("\n");
        }
    }
}

int Renombrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombreantiguo, char *nombrenuevo) {
    int resultado = -1;

    // Verificamos que el nuevo nombre no excede la longitud máxima del nombre de archivo
    if (strlen(nombrenuevo) >= LEN_NFICH) {
        printf("Error: El nombre nuevo es demasiado largo.\n");
    } else {
        // Verificamos que el archivo con el nombre antiguo existe
        int indiceAntiguo = BuscaFich(directorio, inodos, nombreantiguo);
        if (indiceAntiguo == -1) {
            printf("Error: El archivo '%s' no existe en el directorio.\n", nombreantiguo);
        } else {
            // Verificamos que no exista un archivo con el nuevo nombre
            int indiceNuevo = BuscaFich(directorio, inodos, nombrenuevo);
            if (indiceNuevo != -1) {
                printf("Error: Ya existe un archivo con el nombre '%s'.\n", nombrenuevo);
            } else {
                // Si no existe un archivo con el nuevo nombre, renombramos
                strcpy(directorio[indiceAntiguo].dir_nfich, nombrenuevo);
                printf("Archivo renombrado de '%s' a '%s'.\n", nombreantiguo, nombrenuevo);
                resultado = 0; // Renombrado exitoso
            }
        }
    }
    return resultado;
}

int Imprimir(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, 
             EXT_DATOS *memdatos, char *nombre) {
    int resultado = -1;  // Valor de retorno, -1 si no se encuentra el archivo

    // Buscamos el archivo por su nombre en el directorio
    int indiceFichero = BuscaFich(directorio, inodos, nombre);

    // Verificamos si el archivo fue encontrado
    if (indiceFichero == -1) {
        printf("Error: El archivo '%s' no se encuentra en el directorio.\n", nombre);
    } else {
        // Obtenemos el inodo asociado al archivo
        unsigned short int inodoArchivo = directorio[indiceFichero].dir_inodo;

        // Accedemos a la estructura del inodo
        EXT_SIMPLE_INODE *inodo = &inodos->blq_inodos[inodoArchivo];

        printf("Contenido del archivo '%s':\n", nombre);

        // Imprimimos el contenido de los bloques de datos del archivo
        for (int i = 0; i < MAX_NUMS_BLOQUE_INODO; i++) {
            if (inodo->i_nbloque[i] != NULL_BLOQUE) {
                // Cada bloque de datos tiene un índice que corresponde al array de bloques de datos
                EXT_DATOS *bloqueDatos = &memdatos[inodo->i_nbloque[i]];

                // Imprimimos los datos del bloque
                printf("Bloque %d: ", i);
                for (int j = 0; j < SIZE_BLOQUE; j++) {
                    // Imprimimos el contenido del bloque (puedes formatearlo según lo que necesites)
                    printf("%c", bloqueDatos->dato[j]);
                }
                printf("\n");
            }
        }

        // Si no se imprimió ningún bloque, el archivo está vacío
        if (inodo->size_fichero == 0) {
            printf("El archivo está vacio.\n");
        }
        resultado = 0;  // El archivo se encontró y se imprimieron los datos correctamente
    }
    return resultado;
}

int Borrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,
           EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock,
           char *nombre, FILE *fich) {
    int resultado = -1;  // Valor de retorno, -1 si no se puede borrar el archivo

    // Buscar el archivo en el directorio
    int indiceFichero = BuscaFich(directorio, inodos, nombre);
    if (indiceFichero == -1) {
        printf("Error: El archivo '%s' no se encuentra en el directorio.\n", nombre);
        return resultado;
    }

    // Obtener el inodo asociado al archivo
    unsigned short int inodoArchivo = directorio[indiceFichero].dir_inodo;
    EXT_SIMPLE_INODE *inodo = &inodos->blq_inodos[inodoArchivo];

    // Liberar los bloques de datos
    for (int i = 0; i < MAX_NUMS_BLOQUE_INODO; i++) {
        if (inodo->i_nbloque[i] != NULL_BLOQUE) {
            // Marcar el bloque como libre en el mapa de bloques
            ext_bytemaps->bmap_bloques[inodo->i_nbloque[i]] = 0;  // 0 indica bloque libre
            printf("Bloque %d liberado.\n", inodo->i_nbloque[i]);
        }
    }

    // Liberar el inodo
    ext_bytemaps->bmap_inodos[inodoArchivo] = 0;  // 0 indica inodo libre
    printf("Inodo %d liberado.\n", inodoArchivo);

    // Marcar el archivo como eliminado en el directorio (limpiar el nombre y el inodo)
    memset(directorio[indiceFichero].dir_nfich, 0, LEN_NFICH);
    directorio[indiceFichero].dir_inodo = NULL_INODO;
    printf("Archivo '%s' eliminado del directorio.\n", nombre);

    // Actualizar el superbloque
    ext_superblock->s_free_blocks_count += inodo->size_fichero / SIZE_BLOQUE;
    ext_superblock->s_free_inodes_count += 1;
    printf("Superbloque actualizado: Bloques libres: %d, Inodos libres: %d.\n", 
           ext_superblock->s_free_blocks_count, ext_superblock->s_free_inodes_count);

    // Guardar los cambios en el archivo
    fseek(fich, 0, SEEK_SET);  // Volver al principio del archivo
    fwrite(ext_bytemaps, SIZE_BLOQUE, 1, fich);  // Guardar los mapas de bits
    fwrite(ext_superblock, SIZE_BLOQUE, 1, fich);  // Guardar el superbloque
    fwrite(directorio, SIZE_BLOQUE, MAX_FICHEROS, fich);  // Guardar el directorio

    resultado = 0;  // El archivo se borró correctamente
    return resultado;
}

int Copiar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,
           EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock,
           EXT_DATOS *memdatos, char *nombreorigen, char *nombredestino,  FILE *fich) {
    int resultado = 0;  // Valor de retorno, -1 si no se puede copiar el archivo

    // Buscar el archivo de origen
    int indiceOrigen = BuscaFich(directorio, inodos, nombreorigen);
    if (indiceOrigen == -1) {
        printf("Error: El archivo de origen '%s' no se encuentra en el directorio.\n", nombreorigen);
        resultado = -1;
    }

    // Obtener el inodo del archivo de origen
    unsigned short int inodoOrigen = directorio[indiceOrigen].dir_inodo;
    EXT_SIMPLE_INODE *inodoArchivoOrigen = &inodos->blq_inodos[inodoOrigen];

    // Verificar si hay espacio para un nuevo archivo (nuevo inodo)
    int indiceDestino = -1;
    for (int i = 0; i < MAX_FICHEROS; i++) {
        if (directorio[i].dir_inodo == NULL_INODO) {
            indiceDestino = i;
            break;
        }
    }
    if (indiceDestino == -1) {
        printf("Error: No hay espacio disponible en el directorio para el nuevo archivo.\n");
        resultado = -1;
    }

    // Asignar un nuevo inodo para el archivo de destino
    unsigned short int nuevoInodo = -1;
    for (int i = 0; i < MAX_INODOS; i++) {
        if (ext_bytemaps->bmap_inodos[i] == 0) {  // Inodo libre
            nuevoInodo = i;
            break;
        }
    }
    if (nuevoInodo == -1) {
        printf("Error: No hay inodos libres para el archivo de destino.\n");
        resultado = -1;
    }

    // Marcar el nuevo inodo como utilizado
    ext_bytemaps->bmap_inodos[nuevoInodo] = 1;

    // Crear el nuevo inodo para el archivo de destino
    EXT_SIMPLE_INODE *inodoDestino = &inodos->blq_inodos[nuevoInodo];
    memcpy(inodoDestino, inodoArchivoOrigen, sizeof(EXT_SIMPLE_INODE));

    // Copiar los bloques de datos del archivo de origen al archivo de destino
    int numBloquesUsados = 0;
    for (int i = 0; i < MAX_NUMS_BLOQUE_INODO; i++) {
        if (inodoArchivoOrigen->i_nbloque[i] != NULL_BLOQUE) {
            // Buscar un bloque libre
            int bloqueLibre = -1;
            for (int j = 0; j < MAX_BLOQUES_DATOS; j++) {
                if (ext_bytemaps->bmap_bloques[j] == 0) {
                    bloqueLibre = j;
                    break;
                }
            }
            if (bloqueLibre == -1) {
                printf("Error: No hay bloques libres para copiar los datos.\n");
                resultado = -1;
            }

            // Marcar el bloque como utilizado
            ext_bytemaps->bmap_bloques[bloqueLibre] = 1;
            // Copiar los datos del bloque de origen al bloque de destino
            EXT_DATOS *bloqueOrigen = &memdatos[inodoArchivoOrigen->i_nbloque[i]];
            EXT_DATOS *bloqueDestino = &memdatos[bloqueLibre];
            memcpy(bloqueDestino, bloqueOrigen, sizeof(EXT_DATOS));

            // Asignar el bloque copiado al inodo del archivo de destino
            inodoDestino->i_nbloque[numBloquesUsados] = bloqueLibre;
            numBloquesUsados++;
        }
    }

    // Actualizar el tamaño del archivo de destino
    inodoDestino->size_fichero = inodoArchivoOrigen->size_fichero;

    // Actualizar el directorio con el nuevo archivo de destino
    strcpy(directorio[indiceDestino].dir_nfich, nombredestino);
    directorio[indiceDestino].dir_inodo = nuevoInodo;

    // Actualizar el superbloque
    ext_superblock->s_free_blocks_count -= numBloquesUsados;
    ext_superblock->s_free_inodes_count -= 1;

    // Guardar los cambios en el archivo de partición
    fseek(fich, 0, SEEK_SET);  // Volver al principio del archivo
    fwrite(ext_bytemaps, SIZE_BLOQUE, 1, fich);  // Guardar los mapas de bits
    fwrite(ext_superblock, SIZE_BLOQUE, 1, fich);  // Guardar el superbloque
    fwrite(directorio, SIZE_BLOQUE, MAX_FICHEROS, fich);  // Guardar el directorio

    printf("Archivo '%s' copiado correctamente a '%s'.\n", nombreorigen, nombredestino);
    // El archivo se copió correctamente si sigue valiendo 0;
    return resultado;
}
