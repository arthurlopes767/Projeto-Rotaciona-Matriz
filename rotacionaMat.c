#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define ELEM(mat, i, j, cols) ((mat)[(i) * (cols) + (j)])

/*Estrutura de argumentos passados a cada thread*/                    
typedef struct {
    int    id;            
    int    N;             
    int    numThreads;    
    int   *original;     
    int   *rotacionada;   
    double tempoExecucao; 
} ArgThread;

void *rotacionarFatia(void *arg)
{
    ArgThread *dados = (ArgThread *) arg;
 
    int id         = dados->id;
    int N          = dados->N;
    int numThreads = dados->numThreads;
    int *orig      = dados->original;
    int *rot       = dados->rotacionada;
 
    int colPorThread = N / numThreads;
    int colInicio    = id * colPorThread;
    int colFim       = (id == numThreads - 1) ? N : colInicio + colPorThread;
 
    struct timespec tInicio, tFim;
 
    clock_gettime(CLOCK_MONOTONIC, &tInicio);
 
    int i, j;
    for (j = colInicio; j < colFim; j++) {
        for (i = 0; i < N; i++) {
            ELEM(rot, j, N - 1 - i, N) = ELEM(orig, i, j, N);
        }
    }
 
    clock_gettime(CLOCK_MONOTONIC, &tFim);
 
    dados->tempoExecucao  = (double)(tFim.tv_sec  - tInicio.tv_sec);
    dados->tempoExecucao += (double)(tFim.tv_nsec - tInicio.tv_nsec) / 1.0e9;
 
    pthread_exit(NULL);
}
 
int lerMatriz(const char *nomeArquivo, int *matriz, int N)
{
    FILE *fp = fopen(nomeArquivo, "r");
    if (fp == NULL) {
        fprintf(stderr, "Erro: nao foi possivel abrir o arquivo '%s'.\n", nomeArquivo);
        return 0;
    }
 
    int i, j;
    for (i = 0; i < N; i++) {
        for (j = 0; j < N; j++) {
            if (fscanf(fp, "%d", &ELEM(matriz, i, j, N)) != 1) {
                fprintf(stderr,
                        "Erro: falha ao ler elemento [%d][%d] do arquivo.\n",
                        i, j);
                fclose(fp);
                return 0;
            }
        }
    }
 
    fclose(fp);
    return 1;
}
 
int gravarMatriz(const char *nomeArquivo, int *matriz, int N)
{
    FILE *fp = fopen(nomeArquivo, "w");
    if (fp == NULL) {
        fprintf(stderr, "Erro: nao foi possivel criar o arquivo '%s'.\n",
                nomeArquivo);
        return 0;
    }
 
    int i, j;
    for (i = 0; i < N; i++) {
        for (j = 0; j < N; j++) {
            if (j > 0) fprintf(fp, " ");
            fprintf(fp, "%d", ELEM(matriz, i, j, N));
        }
        fprintf(fp, "\n");
    }
 
    fclose(fp);
    return 1;
}
 
void exibirUso(const char *nomePrograma)
{
    fprintf(stderr, "Uso:\n");
    fprintf(stderr, "  %s <N> <T> <arquivo_entrada> <arquivo_saida>\n\n", nomePrograma);
    fprintf(stderr, "  N - dimensao da matriz (N x N)\n");
    fprintf(stderr, "  T - numero de threads\n");
    fprintf(stderr, "  arquivo_entrada - arquivo com a matriz original\n");
    fprintf(stderr, "  arquivo_saida - arquivo de saida (.rot)\n\n");
    fprintf(stderr, "Exemplo:\n");
    fprintf(stderr, "  %s 1000 4 matriz.dat matriz.rot\n", nomePrograma);
}

int main(int argc, char *argv[])
{
    /*Validação dos argumentos de linha de comando*/
    if (argc != 5) {
        fprintf(stderr, "Erro: numero incorreto de argumentos.\n\n");
        exibirUso(argv[0]);
        return 1;
    }
 
    int N = atoi(argv[1]);
    int numThreads = atoi(argv[2]);
    char *arquivoEntrada = argv[3];
    char *arquivoSaida = argv[4];
 
    if (N <= 0) {
        fprintf(stderr, "Erro: N deve ser um inteiro positivo.\n");
        return 1;
    }
    if (numThreads <= 0) {
        fprintf(stderr, "Erro: T deve ser um inteiro positivo.\n");
        return 1;
    }
    if (numThreads > N) {
        fprintf(stderr,"Aviso: numero de threads (%d) maior que N (%d). ""Ajustando para %d thread(s).\n", numThreads, N, N);
        numThreads = N;
    }
 
    /*Alocação das matrizes em uma única etapa*/
    int *matrizOriginal = (int *) malloc((long)N * N * sizeof(int));
    if (matrizOriginal == NULL) {
        fprintf(stderr, "Erro: falha ao alocar memoria para a matriz original.\n");
        return 1;
    }
 
    int *matrizRotacionada = (int *) malloc((long)N * N * sizeof(int));
    if (matrizRotacionada == NULL) {
        fprintf(stderr, "Erro: falha ao alocar memoria para a matriz rotacionada.\n");
        free(matrizOriginal);
        return 1;
    }
 
    if (!lerMatriz(arquivoEntrada, matrizOriginal, N)) {
        free(matrizOriginal);
        free(matrizRotacionada);
        return 1;
    }
 
    /*Alocação dos arrays de threads e seus argumentos*/
    pthread_t *threads = (pthread_t *) malloc(numThreads * sizeof(pthread_t));
    ArgThread *args = (ArgThread *) malloc(numThreads * sizeof(ArgThread));
 
    if (threads == NULL || args == NULL) {
        fprintf(stderr, "Erro: falha ao alocar memoria para as threads.\n");
        free(matrizOriginal);
        free(matrizRotacionada);
        free(threads);
        free(args);
        return 1;
    }
 
    /*Marca o início do tempo total de processamento*/
    struct timespec tTotalInicio, tTotalFim;
    clock_gettime(CLOCK_MONOTONIC, &tTotalInicio);
 
    /*Criação das threads de processamento*/
    int t;
    for (t = 0; t < numThreads; t++) {
        args[t].id = t;
        args[t].N = N;
        args[t].numThreads = numThreads;
        args[t].original = matrizOriginal;
        args[t].rotacionada = matrizRotacionada;
        args[t].tempoExecucao = 0.0;
 
        if (pthread_create(&threads[t], NULL, rotacionarFatia, &args[t]) != 0) {
            fprintf(stderr, "Erro: falha ao criar thread %d.\n", t);
            free(matrizOriginal);
            free(matrizRotacionada);
            free(threads);
            free(args);
            return 1;
        }
    }
 
    /*Aguarda o término de todas as threads*/
    for (t = 0; t < numThreads; t++) {
        pthread_join(threads[t], NULL);
    }
 
    /*Marca o fim do tempo total de processamento*/
    clock_gettime(CLOCK_MONOTONIC, &tTotalFim);
 
    double tempoTotal = (double)(tTotalFim.tv_sec - tTotalInicio.tv_sec);
    tempoTotal += (double)(tTotalFim.tv_nsec - tTotalInicio.tv_nsec)/1.0e9;
 
    /*Exibe tempos de execução das threads e total*/
    for (t = 0; t < numThreads; t++) {
        printf("Tempo de execucao do Thread %d: %.3f segundos.\n", t, args[t].tempoExecucao);
    }
    printf("Tempo total de execucao: %.3f segundos.\n", tempoTotal);
 
    /*Grava a matriz rotacionada no arquivo de saída*/
    if (!gravarMatriz(arquivoSaida, matrizRotacionada, N)) {
        free(matrizOriginal);
        free(matrizRotacionada);
        free(threads);
        free(args);
        return 1;
    }
 
    printf("Matriz rotacionada gravada em '%s'.\n", arquivoSaida);
 
    free(matrizOriginal);
    free(matrizRotacionada);
    free(threads);
    free(args);
 
    return 0;
}
 
