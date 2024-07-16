#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define PAGE_TABLE_SIZE 1048576 // 2^20 para 1M de páginas possíveis

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

    // Verificar se o tamanho da página é uma potência de 2
    if ((page_size_kb & (page_size_kb - 1)) != 0) {
        printf("Erro: O tamanho da página deve ser uma potência de 2.\n");
        return 1;
    }

    int frame_count = memory_size_kb / page_size_kb;
    Frame *frames = (Frame *) malloc(frame_count * sizeof(Frame));
    memset(frames, 0, frame_count * sizeof(Frame));

    // Inicialização de uma tabela de páginas densa (descomente apenas uma das opções abaixo)

   int *page_table = (int *) malloc(PAGE_TABLE_SIZE * sizeof(int));
   for (int i = 0; i < PAGE_TABLE_SIZE; i++) {
       page_table[i] = -1; // Inicializa a tabela de páginas com -1 (indicando páginas não presentes)
   }

    // Inicialização de uma tabela de páginas hierárquica com 2 níveis
//    int *page_table_l1 = (int *) malloc(PAGE_TABLE_SIZE / 1024 * sizeof(int));
//    for (int i = 0; i < PAGE_TABLE_SIZE / 1024; i++) {
//        page_table_l1[i] = -1;
//    }

    // Inicialização de uma tabela de páginas hierárquica com 3 níveis
//    int **page_table_l2 = (int **) malloc(PAGE_TABLE_SIZE / 1024 * sizeof(int *));
//    for (int i = 0; i < PAGE_TABLE_SIZE / 1024; i++) {
//        page_table_l2[i] = NULL;
//    }

    // Inicialização de uma tabela de páginas invertida
    // int *invert_page_table = (int *) malloc(frame_count * sizeof(int));
    // for (int i = 0; i < frame_count; i++) {
    //     invert_page_table[i] = -1;
    // }

    FILE *log_file = fopen(log_file_name, "r");
    if (!log_file) {
        perror("Erro ao abrir o arquivo de log");
        free(frames);
         free(page_table);
        //free(page_table_l1);
        //free(page_table_l2);
        //free(invert_page_table);
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

    // Derivar o valor de s com base no tamanho da página
    unsigned s = 0;
    unsigned tmp = page_size_kb * 1024; // Converter de KB para bytes
    while (tmp > 1) {
        tmp = tmp >> 1;
        s++;
    }

    while (fscanf(log_file, "%x %c", &addr, &rw) != EOF) {
        current_time++;
        // Determinar a página a partir do endereço
        unsigned int page_number = addr >> s;
        int found = 0;

        // Verificar na tabela de páginas descomentar uma das opções abaixo para verificação
        // Tabela de páginas densa
        if (page_table[page_number] != -1) {
            // Página encontrada na tabela de páginas
            int frame_index = page_table[page_number];
            found = 1;
            frames[frame_index].last_access_time = current_time;
            frames[frame_index].reference_bit = 1; // Atualiza o bit de referência
            if (rw == 'W') {
                frames[frame_index].modified = 1;
            }
        }

        // Tabela de páginas hierárquica de 2 níveis
        // int l1_index = page_number >> 10;
        // int l2_index = page_number & 0x3FF;
        // if (page_table_l1[l1_index] != -1) {
        //     int frame_index = page_table_l1[l1_index] + l2_index;
        //     found = 1;
        //     frames[frame_index].last_access_time = current_time;
        //     frames[frame_index].reference_bit = 1;
        //     if (rw == 'W') {
        //         frames[frame_index].modified = 1;
        //     }
        // }

        // Tabela de páginas hierárquica de 3 níveis
        // l1_index = page_number >> 20;
        // l2_index = (page_number >> 10) & 0x3FF;
        // int l3_index = page_number & 0x3FF;
        // if (page_table_l2[l1_index] != NULL && page_table_l2[l1_index][l2_index] != -1) {
        //     int frame_index = page_table_l2[l1_index][l2_index] + l3_index;
        //     found = 1;
        //     frames[frame_index].last_access_time = current_time;
        //     frames[frame_index].reference_bit = 1;
        //     if (rw == 'W') {
        //         frames[frame_index].modified = 1;
        //     }
        // }

        // Tabela de páginas invertida
        // for (int i = 0; i < frame_count; i++) {
        //     if (invert_page_table[i] == page_number) {
        //         int frame_index = i;
        //         found = 1;
        //         frames[frame_index].last_access_time = current_time;
        //         frames[frame_index].reference_bit = 1; // Atualiza o bit de referência
        //         if (rw == 'W') {
        //             frames[frame_index].modified = 1;
        //         }
        //         break;
        //     }
        // }

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
                     free(page_table);
                    //free(page_table_l1);
                     //free(page_table_l2);
                    //free(invert_page_table);
                    return 1;
                }
            }

            if (frames[replace_index].present) {
                // Página sendo substituída
                // Tabela de páginas descomentar uma das opções abaixo para atualização
                 page_table[frames[replace_index].page_number] = -1; // Invalida a entrada da tabela de páginas
                //page_table_l1[frames[replace_index].page_number >> 10] = -1;
                 //page_table_l2[frames[replace_index].page_number >> 20][(frames[replace_index].page_number >> 10) & 0x3FF] = -1;
                //invert_page_table[replace_index] = -1; // Invalida a entrada da tabela de páginas invertida
                
                if (frames[replace_index].modified) {
                    pages_written++;
                }
            }

            // Carregar a nova página no quadro físico
            frames[replace_index].page_number = page_number;
            frames[replace_index].last_access_time = current_time;
            frames[replace_index].modified = (rw == 'W');
            frames[replace_index].present = 1;
            frames[replace_index].reference_bit = 1; // Define o bit de referência

            // Atualizar a tabela de páginas descomentar uma das opções abaixo para atualização
            page_table[page_number] = replace_index;
             //page_table_l1[page_number >> 10] = replace_index - (page_number & 0x3FF);
            //page_table_l2[page_number >> 20][(page_number >> 10) & 0x3FF] = replace_index - (page_number & 0x3FF);
            //invert_page_table[replace_index] = page_number;
        }
    }

    fclose(log_file);
    free(frames);
    free(page_table);
    //free(page_table_l1);
     //free(page_table_l2);
    //free(invert_page_table);

    printf("Arquivo de entrada: %s\n", log_file_name);
    printf("Tamanho da memoria: %d KB\n", memory_size_kb);
    printf("Tamanho das paginas: %d KB\n", page_size_kb);
    printf("Tecnica de reposicao: %s\n", algorithm);
    printf("Paginas lidas: %d\n", page_faults); 
    printf("Paginas escritas: %d\n", pages_written);

    return 0;
}
