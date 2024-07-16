#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Estrutura para representar um quadro físico
typedef struct {
    unsigned int page_number;
    int last_access_time;
    int modified;
    int present;
    int reference_bit;  // Bit de referência para o algoritmo de segunda chance
} Frame;

// Função para encontrar a página com LRU (Least Recently Used)
int findLRU(Frame *frames, int frame_count) {
    int lru_index = 0;
    for (int i = 1; i < frame_count; i++) {
        if (frames[i].last_access_time < frames[lru_index].last_access_time) {
            lru_index = i;
        }
    }
    return lru_index;
}

// Função para selecionar uma página aleatória (RAND)
int findRAND(int frame_count) {
    return rand() % frame_count;
}

// Função para o algoritmo de segunda chance
int findSecondChance(Frame *frames, int frame_count, int *second_chance_index) {
    while (1) {
        if (frames[*second_chance_index].reference_bit == 0) {
            int replace_index = *second_chance_index;
            *second_chance_index = (*second_chance_index + 1) % frame_count;
            return replace_index;
        }
        frames[*second_chance_index].reference_bit = 0;
        *second_chance_index = (*second_chance_index + 1) % frame_count;
    }
}

// Função principal
int main(int argc, char *argv[]) {
    if (argc < 5) {
        printf("Uso: %s <algoritmo> <arquivo.log> <tamanho_pagina_kb> <tamanho_memoria_kb>\n", argv[0]);
        return 1;
    }

    char *algorithm = argv[1];
    char *log_file_name = argv[2];
    int page_size_kb = atoi(argv[3]);
    int memory_size_kb = atoi(argv[4]);

    int frame_count = memory_size_kb / page_size_kb;
    Frame *frames = (Frame *) malloc(frame_count * sizeof(Frame));
    memset(frames, 0, frame_count * sizeof(Frame));

    FILE *log_file = fopen(log_file_name, "r");
    if (!log_file) {
        perror("Erro ao abrir o arquivo de log");
        return 1;
    }

    unsigned int addr;
    char rw;
    int current_time = 0;
    int page_faults = 0;
    int pages_written = 0;
    int fifo_index = 0; // Índice para o algoritmo FIFO
    int second_chance_index = 0; // Índice para o algoritmo de segunda chance

    srand(time(NULL)); // Inicializa a semente do gerador de números aleatórios

    while (fscanf(log_file, "%x %c", &addr, &rw) != EOF) {
        current_time++;
        unsigned int page_number = addr / (page_size_kb * 1024);
        int found = 0;

        for (int i = 0; i < frame_count; i++) {
            if (frames[i].present && frames[i].page_number == page_number) {
                found = 1;
                frames[i].last_access_time = current_time;
                frames[i].reference_bit = 1; // Atualiza o bit de referência
                if (rw == 'W') {
                    frames[i].modified = 1;
                }
                break;
            }
        }

        if (!found) {
            page_faults++;
            int replace_index = -1;
            for (int i = 0; i < frame_count; i++) {
                if (!frames[i].present) {
                    replace_index = i;
                    break;
                }
            }

            if (replace_index == -1) {
                if (strcmp(algorithm, "lru") == 0) {
                    replace_index = findLRU(frames, frame_count);
                } else if (strcmp(algorithm, "fifo") == 0) {
                    replace_index = fifo_index;
                    fifo_index = (fifo_index + 1) % frame_count;
                } else if (strcmp(algorithm, "rand") == 0) {
                    replace_index = findRAND(frame_count);
                } else if (strcmp(algorithm, "2a") == 0) {
                    replace_index = findSecondChance(frames, frame_count, &second_chance_index);
                } else {
                    printf("Algoritmo desconhecido: %s\n", algorithm);
                    fclose(log_file);
                    free(frames);
                    return 1;
                }
            }

            if (frames[replace_index].present && frames[replace_index].modified) {
                pages_written++;
            }

            frames[replace_index].page_number = page_number;
            frames[replace_index].last_access_time = current_time;
            frames[replace_index].modified = (rw == 'W');
            frames[replace_index].present = 1;
            frames[replace_index].reference_bit = 1; // Define o bit de referência
        }
    }

    fclose(log_file);
    free(frames);

    printf("Arquivo de entrada: %s\n", log_file_name);
    printf("Tamanho da memoria: %d KB\n", memory_size_kb);
    printf("Tamanho das paginas: %d KB\n", page_size_kb);
    printf("Tecnica de reposicao: %s\n", algorithm);
    printf("Paginas lidas: %d\n", page_faults);
    printf("Paginas escritas: %d\n", pages_written);

    return 0;
}
