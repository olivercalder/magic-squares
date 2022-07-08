#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

#define LOGSIZE (2)
#define SIZE (4)
#define E(m, r, c) ((m)[((r) << (LOGSIZE)) + (c)])

struct row_list_entry {
    int row[SIZE];
    struct row_list_entry *next;
};

struct matrix_list_entry {
    int matrix[SIZE*SIZE];
    struct matrix_list_entry *next;
};

struct thread_info {
    int thread_id;
    int total_threads;
    int magic_number;
    struct row_list_entry *row_list;
    struct matrix_list_entry *matrix_list;
};

int increment_row(int *matrix, int row, int magic_number) {
    E(matrix, row, 0)++;
    if (E(matrix, row, 0) <= magic_number)
        return 0;
    E(matrix, row, 0) = 1;
    E(matrix, row, 1)++;
    if (E(matrix, row, 1) <= magic_number)
        return 0;
    E(matrix, row, 1) = 1;
    E(matrix, row, 2)++;
    if (E(matrix, row, 2) <= magic_number)
        return 0;
    E(matrix, row, 2) = 1;
    E(matrix, row, 3)++;
    if (E(matrix, row, 3) <= magic_number)
        return 0;
    return 1;
}

void print_magic_square(int *matrix) {
    int i;
    for (i = 0; i < 4; i++)
        printf("%d %d %d %d\n", E(matrix, i, 0), E(matrix, i, 1), E(matrix, i, 2), E(matrix, i, 3));
    printf("\n");
}

int sum_of_row(int *matrix, int row) {
    return E(matrix, row, 0) + E(matrix, row, 1) + E(matrix, row, 2) + E(matrix, row, 3);
}

int sum_of_col(int *matrix, int col) {
    return E(matrix, 0, col) + E(matrix, 1, col) + E(matrix, 2, col) + E(matrix, 3, col);
}

int columns_correct(int *matrix, int magic_number) {
    return sum_of_col(matrix, 0) == magic_number
        && sum_of_col(matrix, 1) == magic_number
        && sum_of_col(matrix, 2) == magic_number
        && sum_of_col(matrix, 3) == magic_number;
}

int diagonals_correct(int *matrix, int magic_number) {
    return (E(matrix, 0, 0) + E(matrix, 1, 1) + E(matrix, 2, 2) + E(matrix, 3, 3) == magic_number
            && E(matrix, 0, 3) + E(matrix, 1, 2) + E(matrix, 2, 1) + E(matrix, 3, 0) == magic_number);
}

int duplicates_exist(int *matrix, int index) {
    int i = 0;
    int j = 1;
    for (i = 0; i < index - 1; i++)
        for (j = i + 1; j < index; j++)
            if (matrix[i] == matrix[j])
                return 1;
    return 0;
}

void one_row(int *matrix, int row) {
    E(matrix, row, 0) = 1;
    E(matrix, row, 1) = 1;
    E(matrix, row, 2) = 1;
    E(matrix, row, 3) = 1;
}

void compute_rows(int magic_number, struct row_list_entry **row_list, int *count) {
    struct row_list_entry *tmp;
    int row[SIZE] = {1, 1, 1, 1};
    *row_list = NULL;
    *count = 0;
    while (increment_row(row, 0, magic_number) == 0) {
        if (sum_of_row(row, 0) != magic_number)
            continue;
        if (duplicates_exist(row, 4))
            continue;
        if (row[0] > row[3])    /* eliminate symmetrically equivalent rows */
            continue;
        tmp = malloc(sizeof(struct row_list_entry));
        tmp->row[0] = row[0];
        tmp->row[1] = row[1];
        tmp->row[2] = row[2];
        tmp->row[3] = row[3];
        tmp->next = *row_list;
        *row_list = tmp;
        (*count)++;
    }
}

int find_magic_squares(int magic_number, int *seed_row, struct matrix_list_entry **matrix_list) {
    int magic_square_count = 0;
    struct matrix_list_entry *tmp;
    int matrix[SIZE*SIZE] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
    matrix[0] = seed_row[0];
    matrix[1] = seed_row[1];
    matrix[2] = seed_row[2];
    matrix[3] = seed_row[3];
    while (increment_row(matrix, 1, magic_number) == 0) {
        if (E(matrix, 0, 3) + E(matrix, 1, 3) > magic_number)
            break;
        if (E(matrix, 0, 2) + E(matrix, 1, 2) > magic_number) {
            E(matrix, 1, 0) = magic_number;
            E(matrix, 1, 1) = magic_number;
            E(matrix, 1, 2) = magic_number;
            continue;
        }
        if (E(matrix, 0, 1) + E(matrix, 1, 1) > magic_number) {
            E(matrix, 1, 0) = magic_number;
            E(matrix, 1, 1) = magic_number;
            continue;
        }
        if (E(matrix, 0, 0) + E(matrix, 1, 0) > magic_number) {
            E(matrix, 1, 0) = magic_number;
            continue;
        }
        /* Diagonal from top left */
        if (E(matrix, 0, 0) + E(matrix, 1, 1) > magic_number) {
            E(matrix, 1, 0) = magic_number;
            E(matrix, 1, 1) = magic_number;
            continue;
        }
        /* Diagonal from top right */
        if (E(matrix, 0, 3) + E(matrix, 1, 2) > magic_number) {
            E(matrix, 1, 0) = magic_number;
            E(matrix, 1, 1) = magic_number;
            E(matrix, 1, 2) = magic_number;
            continue;
        }
        if (sum_of_row(matrix, 1) != magic_number)
            continue;
        if (duplicates_exist(matrix, 8))
            continue;
        one_row(matrix, 2);
        while (increment_row(matrix, 2, magic_number) == 0) {
            if (E(matrix, 0, 3) + E(matrix, 1, 3) + E(matrix, 2, 3) > magic_number)
                break;
            if (E(matrix, 0, 2) + E(matrix, 1, 2) + E(matrix, 2, 2) > magic_number) {
                E(matrix, 2, 0) = magic_number;
                E(matrix, 2, 1) = magic_number;
                E(matrix, 2, 2) = magic_number;
                continue;
            }
            if (E(matrix, 0, 1) + E(matrix, 1, 1) + E(matrix, 2, 1) > magic_number) {
                E(matrix, 2, 0) = magic_number;
                E(matrix, 2, 1) = magic_number;
                continue;
            }
            if (E(matrix, 0, 0) + E(matrix, 1, 0) + E(matrix, 2, 0) > magic_number) {
                E(matrix, 2, 0) = magic_number;
                continue;
            }
            /* Diagonal from top left */
            if (E(matrix, 0, 0) + E(matrix, 1, 1) + E(matrix, 2, 2) > magic_number) {
                E(matrix, 2, 0) = magic_number;
                E(matrix, 2, 1) = magic_number;
                E(matrix, 2, 2) = magic_number;
                continue;
            }
            /* Diagonal from top right */
            if (E(matrix, 0, 3) + E(matrix, 1, 2) + E(matrix, 2, 1) > magic_number) {
                E(matrix, 2, 0) = magic_number;
                E(matrix, 2, 1) = magic_number;
                continue;
            }
            if (sum_of_row(matrix, 2) != magic_number)
                continue;
            if (duplicates_exist(matrix, 12))
                continue;
            one_row(matrix, 3);
            /* delete diagonal and rotational symmetry by forcing top left corner smallest */
            E(matrix, 3, 3) = E(matrix, 0, 0) + 1;
            while (increment_row(matrix, 3, magic_number) == 0) {
                /* delete vertical symmetry by forcing top left corner smallest */
                if (E(matrix, 0, 0) > E(matrix, 3, 0)) {
                    E(matrix, 3, 0) = E(matrix, 0, 0);
                    continue;
                }
                /* delete diagonal symmetry by forcing top right < bottom left */
                if (E(matrix, 0, 3) > E(matrix, 3, 0)) {
                    E(matrix, 3, 0) = E(matrix, 0, 3);
                    continue;
                }
                if (sum_of_row(matrix, 3) != magic_number)
                    continue;
                if (duplicates_exist(matrix, 16))
                    continue;
                if (columns_correct(matrix, magic_number) && diagonals_correct(matrix, magic_number)) {
                    tmp = malloc(sizeof(struct matrix_list_entry));
                    memcpy(tmp->matrix, matrix, sizeof(int) * SIZE*SIZE);
                    tmp->next = *matrix_list;
                    *matrix_list = tmp;
                    magic_square_count++;
                    /* print_magic_square(matrix); */
                }
            }
        }
    }
    return magic_square_count;
}

void *thread_find_magic_squares(void *arg) {
    struct thread_info *info = (struct thread_info *)arg;
    struct row_list_entry *seed_row_entry = info->row_list;
    int i;
    for (i = 0; i < info->thread_id; i++)
        seed_row_entry = seed_row_entry->next;
    unsigned long long magic_square_count = 0;
    unsigned long long magic_square_total = 0;
    while (seed_row_entry != NULL) {
        magic_square_count = find_magic_squares(info->magic_number, seed_row_entry->row, &info->matrix_list);
        magic_square_total += magic_square_count;
        if (magic_square_count)
            printf("Thread %d\tfound %lld additional magic squares from seed row [%d %d %d %d]\n", info->thread_id, magic_square_count, seed_row_entry->row[0], seed_row_entry->row[1], seed_row_entry->row[2], seed_row_entry->row[3]);
        else
            printf("Thread %d\tfound no magic squares from seed row [%d %d %d %d]\n", info->thread_id, seed_row_entry->row[0], seed_row_entry->row[1], seed_row_entry->row[2], seed_row_entry->row[3]);
        for (i = 0; i < info->total_threads && seed_row_entry != NULL; i++)
            seed_row_entry = seed_row_entry->next;
    }
    pthread_exit((void *)magic_square_total);
}

int main() {
    int magic_number = 34;
    struct row_list_entry *row_list;
    int row_count, i;
    void *square_count = 0;
    unsigned long long total_squares = 0;
    unsigned int num_cores = sysconf(_SC_NPROCESSORS_ONLN);
    struct thread_info *thread_info_array = malloc(sizeof(struct thread_info) * num_cores);
    struct matrix_list_entry *current;
    compute_rows(magic_number, &row_list, &row_count);
    printf("Found %d viable seed rows.\n", row_count);
    printf("Running with %d processing threads.\n\n", num_cores);
    pthread_t *thread_array = malloc(sizeof(pthread_t) * num_cores);
    for (i = 0; i < num_cores; i++) {
        thread_info_array[i].thread_id = i;
        thread_info_array[i].total_threads = num_cores;
        thread_info_array[i].row_list = row_list;
        thread_info_array[i].magic_number = magic_number;
        thread_info_array[i].matrix_list = NULL;
        pthread_create(thread_array + i, NULL, &thread_find_magic_squares, thread_info_array + i);
    }
    for (i = 0; i < num_cores; i++) {
        pthread_join(thread_array[i], &square_count);
        total_squares += (unsigned long long)square_count;
    }
    printf("There were %lld distinct magic squares found.\n\n", total_squares);
    for (i = 0; i < num_cores; i++) {
        current = thread_info_array[i].matrix_list;
        while (current != NULL) {
            print_magic_square(current->matrix);
            current = current->next;
        }
    }
}
