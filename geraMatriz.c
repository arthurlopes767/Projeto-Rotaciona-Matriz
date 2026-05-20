#include <stdio.h>
#include <stdlib.h>
#include <time.h>
 
int main(int argc, char *argv[])
{
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <N> <arquivo_saida>\n", argv[0]);
        fprintf(stderr, "Exemplo: %s 1000 matriz1000.dat\n", argv[0]);
        return 1;
    }
 
    int N = atoi(argv[1]);
    if (N <= 0) {
        fprintf(stderr, "Erro: N deve ser um inteiro positivo.\n");
        return 1;
    }
 
    FILE *fp = fopen(argv[2], "w");
    if (fp == NULL) {
        fprintf(stderr, "Erro: nao foi possivel criar o arquivo '%s'.\n", argv[2]);
        return 1;
    }
 
    srand((unsigned int) time(NULL));
 
    int i, j;
    for (i = 0; i < N; i++) {
        for (j = 0; j < N; j++) {
            /* Gera inteiro entre -1000 e 1000 */
            int valor = (rand() % 2001) - 1000;
            if (j > 0) fprintf(fp, " ");
            fprintf(fp, "%d", valor);
        }
        fprintf(fp, "\n");
    }
 
    fclose(fp);
    printf("Matriz %dx%d gravada em '%s'.\n", N, N, argv[2]);
    return 0;
}
