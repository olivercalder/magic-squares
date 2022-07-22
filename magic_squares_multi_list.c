#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

struct row_list_entry {
    int *row;
    struct row_list_entry *next;
    struct row_list_entry *next_seed;
};

struct square_list_entry {
    int *square;
    struct square_list_entry *next;
};

struct square_list {
    struct square_list_entry *squares;
    struct square_list *next;
};

struct thread_info {
    int thread_id;
    int total_threads;
    int magic_number;
    int size;
    int no_zero;
    int strict;
    int magic_square_count;
    int silent;
    struct row_list_entry *row_list;
    struct row_list_entry *seed_list;
    struct square_list *square_lists;
};

int increment_row(int *row, int magic_number, int no_zero) {
    row[0]++;
    if (row[0] <= magic_number)
        return 0;
    row[0] = no_zero;
    row[1]++;
    if (row[1] <= magic_number)
        return 0;
    row[1] = no_zero;
    row[2]++;
    if (row[2] <= magic_number)
        return 0;
    row[2] = no_zero;
    row[3]++;
    if (row[3] <= magic_number)
        return 0;
    return 1;
}

void print_magic_square(int *square, int size, FILE *outfile) {
    int i, j;
    for (i = 0; i < size; i++) {
        fprintf(outfile, "%d", square[i * size]);
        for (j = 1; j < size; j++)
            fprintf(outfile, " %d", square[i * size + j]);
        fprintf(outfile, "\n");
    }
    fprintf(outfile, "\n");
}

int sum_of_row(int *row) {
    return row[0] + row[1] + row[2] + row[3];
}

int duplicates_exist_in_row(int *row, int size) {
    int i = 0;
    int j = 1;
    for (i = 0; i < size - 1; i++)
        for (j = i + 1; j < size; j++)
            if (row[i] == row[j])
                return 1;
    return 0;
}

int rows_duplicates_exist(struct row_list_entry **rows, int row_index, int size) {
    /* Assumes no duplicates within a given row */
    /* Assumes rows prior to final row have no duplicates */
    int i, j, k, value;
    for (i = 0; i < size; i++) {
        value = rows[row_index]->row[i];
        for (j = 0; j < row_index; j++) {
            for (k = 0; k < size; k++) {
                if (rows[j]->row[k] == value)
                    return 1;
            }
        }
    }
    return 0;
}

void compute_rows(int magic_number, int size, int no_zero, struct row_list_entry **row_list, struct row_list_entry **seed_list, int *count, int *seed_count) {
    struct row_list_entry *tmp, *tail, *seed_tail;
    int i, *row;
    *row_list = NULL;
    *seed_list = NULL;
    *count = 0;
    *seed_count = 0;
    row = malloc(sizeof(int) * size);
    for (i = 0; i < size; i++)
        row[i] = no_zero;
    while (increment_row(row, magic_number, no_zero) == 0) {
        if (sum_of_row(row) != magic_number)
            continue;
        if (duplicates_exist_in_row(row, size))
            continue;
        tmp = malloc(sizeof(struct row_list_entry));
        tmp->row = malloc(sizeof(int) * size);
        memcpy(tmp->row, row, sizeof(int) * size);
        tmp->next = NULL;
        if (*row_list == NULL)
            *row_list = tmp;
        else
            tail->next = tmp;
        tail = tmp;
        (*count)++;
        tmp->next_seed = NULL;
        if (row[0] < row[size - 1]) {
            if (*seed_list == NULL)
                *seed_list = tmp;
            else
                seed_tail->next_seed = tmp;
            seed_tail = tmp;
            (*seed_count)++;
        }
    }
    free(row);
}

int find_magic_squares(int magic_number, int size, int no_zero, struct row_list_entry *row_list, struct row_list_entry *seed_row, struct square_list_entry **square_list, int strict) {
    int i, magic_square_count = 0, tmp_sum;
    struct row_list_entry **rows;
    struct row_list_entry *final_row;
    struct square_list_entry *tmp, *tail = NULL;
    *square_list = NULL;
    rows = malloc(sizeof(struct row_list_entry *) * size);
    final_row = malloc(sizeof(struct row_list_entry));
    final_row->row = malloc(sizeof(int) * size);
    rows[0] = seed_row;
    rows[size - 1] = final_row;
    for (rows[1] = row_list; rows[1] != NULL; rows[1] = rows[1]->next) {
        /* Third column */
        if (rows[0]->row[3] + rows[1]->row[3] + no_zero + no_zero > magic_number)
            break;
        if (strict & 2) {
            /* Taurus diagonals */
            if (rows[0]->row[2] + rows[1]->row[3] + no_zero + no_zero > magic_number
                    || rows[0]->row[0] + rows[1]->row[3] + no_zero + no_zero > magic_number)
                break;
            if (rows[0]->row[1] + rows[1]->row[2] + no_zero + no_zero > magic_number
                    /* || rows[0]->row[2] + rows[1]->row[3] + no_zero + no_zero > magic_number */
                    || rows[0]->row[3] + rows[1]->row[0] + no_zero + no_zero > magic_number
                    /* || rows[0]->row[0] + rows[1]->row[3] + no_zero + no_zero > magic_number */
                    || rows[0]->row[1] + rows[1]->row[0] + no_zero + no_zero > magic_number
                    || rows[0]->row[2] + rows[1]->row[1] + no_zero + no_zero > magic_number)
                continue;
        }
        if (strict & 1) {
            /* Quadrants */
            if (rows[0]->row[0] + rows[0]->row[1] + rows[1]->row[0] + rows[1]->row[1] != magic_number
                    || rows[0]->row[2] + rows[0]->row[3] + rows[1]->row[2] + rows[1]->row[3] != magic_number)
                continue;
        }
        /* Remaining columns */
        if (rows[0]->row[2] + rows[1]->row[2] + no_zero + no_zero > magic_number
                || rows[0]->row[1] + rows[1]->row[1] + no_zero + no_zero > magic_number
                || rows[0]->row[0] + rows[1]->row[0] + no_zero + no_zero > magic_number)
            continue;
        /* Diagonal from top left */
        if (rows[0]->row[0] + rows[1]->row[1] + no_zero + no_zero > magic_number)
            continue;
        /* Diagonal from top right */
        if (rows[0]->row[3] + rows[1]->row[2] + no_zero + no_zero > magic_number)
            continue;
        if (rows_duplicates_exist(rows, 1, size))
            continue;
        for (rows[2] = row_list; rows[2] != NULL; rows[2] = rows[2]->next) {
            /* Once third row is set and valid, fourth can be computed directly */
            /* Third column */
            if ((tmp_sum = rows[0]->row[3] + rows[1]->row[3] + rows[2]->row[3]) + no_zero > magic_number)
                break;
            rows[3]->row[3] = magic_number - tmp_sum;
            if (strict & 2) {
                /* Taurus diagonals */
                if (rows[0]->row[1] + rows[1]->row[2] + rows[2]->row[3] + no_zero > magic_number
                        || rows[0]->row[1] + rows[1]->row[0] + rows[2]->row[3] + no_zero > magic_number)
                    break;
            }
            /* Remaining columns */
            if ((tmp_sum = rows[0]->row[2] + rows[1]->row[2] + rows[2]->row[2]) + no_zero > magic_number)
                continue;
            rows[3]->row[2] = magic_number - tmp_sum;
            if ((tmp_sum = rows[0]->row[1] + rows[1]->row[1] + rows[2]->row[1]) + no_zero > magic_number)
                continue;
            rows[3]->row[1] = magic_number - tmp_sum;
            if ((tmp_sum = rows[0]->row[0] + rows[1]->row[0] + rows[2]->row[0]) + no_zero > magic_number)
                continue;
            rows[3]->row[0] = magic_number - tmp_sum;
            /* Delete vertical symmetry by forcing top left < bottom left */
            if (rows[0]->row[0] > rows[3]->row[0])
                continue;
            /* Delete diagonal symmetry by forcing top right < bottom left */
            if (rows[0]->row[3] > rows[3]->row[0])
                continue;
            /* Delete diagonal symmetry by forcing top left < bottom right */
            if (rows[0]->row[0] > rows[3]->row[3])
                continue;
            /* Check final row sums to magic number and no duplicates exist */
            if (sum_of_row(rows[3]->row) != magic_number
                    || duplicates_exist_in_row(rows[3]->row, size))
                continue;
            /* Diagonal from top left */
            if (rows[0]->row[0] + rows[1]->row[1] + rows[2]->row[2] + rows[3]->row[3] != magic_number)
                continue;
            /* Diagonal from top right */
            if (rows[0]->row[3] + rows[1]->row[2] + rows[2]->row[1] + rows[3]->row[0] != magic_number)
                continue;
            if (strict & 2) {
                if (rows[0]->row[1] + rows[1]->row[2] + rows[2]->row[3] + rows[3]->row[0] != magic_number
                        || rows[0]->row[2] + rows[1]->row[3] + rows[2]->row[0] + rows[3]->row[1] != magic_number
                        || rows[0]->row[3] + rows[1]->row[0] + rows[2]->row[1] + rows[3]->row[2] != magic_number
                        || rows[0]->row[0] + rows[1]->row[3] + rows[2]->row[2] + rows[3]->row[1] != magic_number
                        || rows[0]->row[1] + rows[1]->row[0] + rows[2]->row[3] + rows[3]->row[2] != magic_number
                        || rows[0]->row[2] + rows[1]->row[1] + rows[2]->row[0] + rows[3]->row[3] != magic_number)
                    continue;
            }
            if (strict & 1) {
                /* Quadrants */
                if (rows[1]->row[1] + rows[1]->row[2] + rows[2]->row[1] + rows[2]->row[2] != magic_number
                        || rows[2]->row[0] + rows[2]->row[1] + rows[3]->row[0] + rows[3]->row[1] != magic_number
                        || rows[2]->row[2] + rows[2]->row[3] + rows[3]->row[2] + rows[3]->row[3] != magic_number)
                    continue;
            }
            if (rows_duplicates_exist(rows, 2, size) || rows_duplicates_exist(rows, 3, size))
                continue;
            /* If made it here, this is a valid magic square */
            tmp = malloc(sizeof(struct square_list_entry));
            tmp->square = malloc(sizeof(int) * size * size);
            for (i = 0; i < size; i++)
                memcpy(&(tmp->square[i * size]), rows[i]->row, sizeof(int) * size);
            tmp->next = NULL;
            if (*square_list == NULL)
                *square_list = tmp;
            else
                tail->next = tmp;
            tail = tmp;
            magic_square_count++;
        }
    }
    free(final_row->row);
    free(final_row);
    free(rows);
    return magic_square_count;
}

void *thread_find_magic_squares(void *arg) {
    struct thread_info *info = (struct thread_info *)arg;
    struct row_list_entry *seed_row_entry = info->seed_list;
    struct square_list *tmp, *tail;
    int i, magic_square_count = 0;
    for (i = 0; i < info->thread_id; i++)
        seed_row_entry = seed_row_entry->next_seed;
    info->magic_square_count = 0;
    info->square_lists = NULL;
    while (seed_row_entry != NULL) {
        tmp = malloc(sizeof(struct square_list));
        tmp->next = NULL;
        if (info->square_lists == NULL)
            info->square_lists = tmp;
        else
            tail->next = tmp;
        tail = tmp;
        magic_square_count = find_magic_squares(info->magic_number, info->size, info->no_zero, info->row_list, seed_row_entry, &tail->squares, info->strict);
        info->magic_square_count += magic_square_count;
        if (!info->silent) {
            if (magic_square_count)
                fprintf(stderr, "Thread %d\tfound %d additional magic squares from seed row [%d %d %d %d]\n", info->thread_id, magic_square_count, seed_row_entry->row[0], seed_row_entry->row[1], seed_row_entry->row[2], seed_row_entry->row[3]);
            else
                fprintf(stderr, "Thread %d\tfound no magic squares from seed row [%d %d %d %d]\n", info->thread_id, seed_row_entry->row[0], seed_row_entry->row[1], seed_row_entry->row[2], seed_row_entry->row[3]);
        }
        for (i = 0; seed_row_entry != NULL && i < info->total_threads; i++)
            seed_row_entry = seed_row_entry->next_seed;
    }
    pthread_exit(0);
}

int magic_squares(int magic_number, int size, int no_zero, int strict, int num_threads, char *outfilename, int silent) {
    struct row_list_entry *row_list, *seed_list, *tmp;
    int row_count, seed_count, i;
    int total_squares = 0;
    struct thread_info *thread_info_array;
    struct square_list_entry *current, *tmp_entry;
    struct square_list *tmp_list;
    FILE *outfile = NULL;
    thread_info_array = malloc(sizeof(struct thread_info) * num_threads);
    compute_rows(magic_number, size, no_zero, &row_list, &seed_list, &row_count, &seed_count);
    if (!silent) {
        fprintf(stderr, "Found %d viable rows and %d viable seed rows.\n", row_count, seed_count);
        fprintf(stderr, "Running with %d processing threads.\n\n", num_threads);
    }
    pthread_t *thread_array = malloc(sizeof(pthread_t) * num_threads);
    for (i = 0; i < num_threads; i++) {
        thread_info_array[i].thread_id = i;
        thread_info_array[i].total_threads = num_threads;
        thread_info_array[i].magic_number = magic_number;
        thread_info_array[i].size = size;
        thread_info_array[i].no_zero = no_zero;
        thread_info_array[i].strict = strict;
        thread_info_array[i].silent = silent;
        thread_info_array[i].row_list = row_list;
        thread_info_array[i].seed_list = seed_list;
        pthread_create(thread_array + i, NULL, &thread_find_magic_squares, thread_info_array + i);
    }
    for (i = 0; i < num_threads; i++) {
        pthread_join(thread_array[i], NULL);
        total_squares += thread_info_array[i].magic_square_count;
    }
    free(thread_array);
    while (row_list != NULL) {
        tmp = row_list;
        row_list = row_list->next;
        free(tmp->row);
        free(tmp);
    }
    if (!silent)
        fprintf(stderr, "There were %d distinct magic squares found.\n\n", total_squares);
    if (outfilename != NULL)
        outfile = fopen(outfilename, "w");
    else
        outfile = stdout;
    /* Destructively print magic squares _in order_ across threads */
    for (i = 0; thread_info_array[i].square_lists != NULL; i = (i + 1) % num_threads) {
        current = thread_info_array[i].square_lists->squares;
        while (current != NULL) {
            print_magic_square(current->square, size, outfile);
            tmp_entry = current;
            current = current->next;
            free(tmp_entry->square);
            free(tmp_entry);
        }
        tmp_list = thread_info_array[i].square_lists;
        thread_info_array[i].square_lists = tmp_list->next;
        free(tmp_list);
    }
    free(thread_info_array);
    if (outfile != stdout)
        fclose(outfile);
}

char *USAGE = "USAGE: %s [-h] [-d SIZE] [-m NUMBER] [-o OUTFILE] [-p] [-s] [-S] [-t THREADS]\n"
"   -h          display this help message\n"
"   -d SIZE     compute magic squares with dimension SIZExSIZE\n"
"   -m NUMBER   use NUMBER as the magic number\n"
"   -o OUTFILE  write results to the output file given by OUTFILE -- default is stdout\n"
"   -p          only allow positive (non-zero) numbers -- default allows 0\n"
"   -s          strict, where quadrants and the center must also add to the magic number\n"
"   -S          very strict, where diagonals on the square as a taurus must also add to the magic number\n"
"   -t THREADS  use THREADS number of execution threads -- default is all available threads\n";
int main(int argc, char **argv) {
    int magic_number = 33, size = 4, no_zero = 0, strict = 0, silent = 0, total_squares, opt;
    unsigned int num_threads = sysconf(_SC_NPROCESSORS_ONLN);
    char *outfilename = NULL;
    while ((opt = getopt(argc, argv, "hd:m:o:psSt:")) != -1) {
        switch (opt) {
            case 'h':
                fprintf(stderr, USAGE, argv[0]);
                exit(1);
            case 'd':
                size = atoi(optarg);
                break;
            case 'm':
                magic_number = atoi(optarg);
                break;
            case 'o':
                outfilename = optarg;
                break;
            case 'p':
                no_zero = 1;
                break;
            case 's':
                strict |= 1;
                break;
            case 'S':
                strict |= 2;
                break;
            case 't':
                num_threads = atoi(optarg);
                break;
            default:
                fprintf(stderr, USAGE, argv[0]);
                exit(1);
        }
    }
    total_squares = magic_squares(magic_number, size, no_zero, strict, num_threads, outfilename, silent);
}
