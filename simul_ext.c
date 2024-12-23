#include<stdio.h>
#include<string.h>
#include<ctype.h>
#include<stdlib.h>
#include "cabeceras.h"

#define LONGITUD_COMANDO 100

void Printbytemaps(EXT_BYTE_MAPS *ext_bytemaps);
char* leeLinea(int tam);

int ComprobarComando(char *strcomando, char *orden, char *argumento1, char *argumento2);

void LeeSuperBloque(EXT_SIMPLE_SUPERBLOCK *psup);


int BuscaFich(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, 
              char *nombre);
void Directorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos);

/*
int Renombrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, 
              char *nombreantiguo, char *nombrenuevo);
int Imprimir(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, 
             EXT_DATOS *memdatos, char *nombre);
int Borrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,
           EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock,
           char *nombre,  FILE *fich);
int Copiar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,
           EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock,
           EXT_DATOS *memdatos, char *nombreorigen, char *nombredestino,  FILE *fich);
void Grabarinodosydirectorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, FILE *fich);
void GrabarByteMaps(EXT_BYTE_MAPS *ext_bytemaps, FILE *fich);
void GrabarSuperBloque(EXT_SIMPLE_SUPERBLOCK *ext_superblock, FILE *fich);
void GrabarDatos(EXT_DATOS *memdatos, FILE *fich);
*/
int main()
{
	 char *comando;
	 char orden[LONGITUD_COMANDO];
	 char argumento1[LONGITUD_COMANDO];
	 char argumento2[LONGITUD_COMANDO];
	 
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
		 } while (ComprobarComando(comando, orden, argumento1, argumento2) != 0);
		 
		 if (strcmp(comando, "dir") == 0) {
			 Directorio(directorio, &ext_blq_inodos);
		 }
		 if (strcmp(comando, "info") == 0) {
			 LeeSuperBloque(&ext_superblock);
		 }
		
		 if (strcmp(comando, "bytemaps") == 0) {
			 Printbytemaps(&ext_bytemaps);
		 }
		return 0;
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

int ComprobarComando(char *strcomando, char *orden, char *argumento1, char *argumento2){
	printf("si");
	return 0;
}

char *leeLinea(int tam)
{
   int i = 0;
   char c;
   char *res = (char *)malloc(tam);
   do
   {
      c = getchar();
      if(c != '\n')
      {
         res[i++] = c;
      }
      

   }
   while(c != '\n' && i < tam);

   res[i] = '\0';

   return res;

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
