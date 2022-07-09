#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#define LOGSIZE (2)
#define SIZE (4)
#define E(s, r, c) ((s)[((r) << (LOGSIZE)) + (c)])

int START_NUM = 0;

struct row_list_entry {
    int row[SIZE];
    struct row_list_entry *next;
    struct row_list_entry *next_seed;
};

struct square_list_entry {
    int square[SIZE*SIZE];
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
    int strict;
    int magic_square_count;
    struct row_list_entry *row_list;
    struct row_list_entry *seed_list;
    struct square_list *square_lists;
};

int increment_row(int *row, int magic_number) {
    row[0]++;
    if (row[0] <= magic_number)
        return 0;
    row[0] = START_NUM;
    row[1]++;
    if (row[1] <= magic_number)
        return 0;
    row[1] = START_NUM;
    row[2]++;
    if (row[2] <= magic_number)
        return 0;
    row[2] = START_NUM;
    row[3]++;
    if (row[3] <= magic_number)
        return 0;
    return 1;
}

void print_magic_square(int *square, FILE *outfile) {
    int i;
    for (i = 0; i < 4; i++)
        fprintf(outfile, "%d %d %d %d\n", E(square, i, 0), E(square, i, 1), E(square, i, 2), E(square, i, 3));
    fprintf(outfile, "\n");
}

int sum_of_row(int *row) {
    return row[0] + row[1] + row[2] + row[3];
}

int sum_of_col(struct row_list_entry **rows, int col) {
    return rows[0]->row[col] + rows[1]->row[col] + rows[2]->row[col] + rows[3]->row[col];
}

int columns_correct(struct row_list_entry **rows, int magic_number) {
    return sum_of_col(rows, 0) == magic_number
        && sum_of_col(rows, 1) == magic_number
        && sum_of_col(rows, 2) == magic_number
        && sum_of_col(rows, 3) == magic_number;
}

int diagonals_correct(struct row_list_entry **rows, int magic_number) {
    return (rows[0]->row[0] + rows[1]->row[1] + rows[2]->row[2] + rows[3]->row[3] == magic_number
            && rows[0]->row[3] + rows[1]->row[2] + rows[2]->row[1] + rows[3]->row[0] == magic_number);
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

int rows_duplicates_exist(struct row_list_entry **rows, int row_count, int size) {
    /* Assumes no duplicates within a given row */
    /* Assumes rows prior to final row have no duplicates */
    int i, j, k, value;
    for (i = 0; i < size; i++) {
        value = rows[row_count - 1]->row[i];
        for (j = 0; j < row_count - 1; j++) {
            for (k = 0; k < size; k++) {
                if (rows[j]->row[k] == value)
                    return 1;
            }
        }
    }
    return 0;
}

void reset_row(int *row) {
    row[0] = START_NUM;
    row[1] = START_NUM;
    row[2] = START_NUM;
    row[3] = START_NUM;
}

void compute_rows(int magic_number, struct row_list_entry **row_list, struct row_list_entry **seed_list, int *count, int *seed_count) {
    struct row_list_entry *tmp, *tail, *seed_tail;
    int row[SIZE] = {START_NUM, START_NUM, START_NUM, START_NUM};
    *row_list = NULL;
    *count = 0;
    *seed_count = 0;
    while (increment_row(row, magic_number) == 0) {
        if (sum_of_row(row) != magic_number)
            continue;
        if (duplicates_exist_in_row(row, SIZE))
            continue;
        tmp = malloc(sizeof(struct row_list_entry));
        tmp->row[0] = row[0];
        tmp->row[1] = row[1];
        tmp->row[2] = row[2];
        tmp->row[3] = row[3];
        tmp->next = NULL;
        if (*row_list == NULL)
            *row_list = tmp;
        else
            tail->next = tmp;
        tail = tmp;
        (*count)++;
        tmp->next_seed = NULL;
        if (row[0] < row[3]) {
            if (*seed_list == NULL)
                *seed_list = tmp;
            else
                seed_tail->next_seed = tmp;
            seed_tail = tmp;
            (*seed_count)++;
        }
    }
}

int find_magic_squares(int magic_number, int size, struct row_list_entry *row_list, struct row_list_entry *seed_row, struct square_list_entry **square_list, int strict) {
    int i, magic_square_count = 0;
    struct row_list_entry *rows[4];
    struct square_list_entry *tmp, *tail = NULL;
    *square_list = NULL;
    rows[0] = seed_row;
    for (rows[1] = row_list; rows[1] != NULL; rows[1] = rows[1]->next) {
        /* Third column */
        if (rows[0]->row[3] + rows[1]->row[3] > magic_number)
            break;
        if (strict & 2) {
            /* Taurus diagonals */
            if (rows[0]->row[2] + rows[1]->row[3] > magic_number
                    || rows[0]->row[0] + rows[1]->row[3] > magic_number)
                break;
            if (rows[0]->row[1] + rows[1]->row[2] > magic_number
                    /* || rows[0]->row[2] + rows[1]->row[3] > magic_number */
                    || rows[0]->row[3] + rows[1]->row[0] > magic_number
                    /* || rows[0]->row[0] + rows[1]->row[3] > magic_number */
                    || rows[0]->row[1] + rows[1]->row[0] > magic_number
                    || rows[0]->row[2] + rows[1]->row[1] > magic_number)
                continue;
        }
        if (strict & 1) {
            /* Quadrants */
            if (rows[0]->row[0] + rows[0]->row[1] + rows[1]->row[0] + rows[1]->row[1] != magic_number
                    || rows[0]->row[2] + rows[0]->row[3] + rows[1]->row[2] + rows[1]->row[3] != magic_number)
                continue;
        }
        /* Remaining columns */
        if (rows[0]->row[2] + rows[1]->row[2] > magic_number
                || rows[0]->row[1] + rows[1]->row[1] > magic_number
                || rows[0]->row[0] + rows[1]->row[0] > magic_number)
            continue;
        /* Diagonal from top left */
        if (rows[0]->row[0] + rows[1]->row[1] > magic_number)
            continue;
        /* Diagonal from top right */
        if (rows[0]->row[3] + rows[1]->row[2] > magic_number)
            continue;
        if (rows_duplicates_exist(rows, 2, size))
            continue;
        for (rows[2] = row_list; rows[2] != NULL; rows[2] = rows[2]->next) {
            /* Third column */
            if (rows[0]->row[3] + rows[1]->row[3] + rows[2]->row[3] > magic_number)
                break;
            if (strict & 2) {
                /* Taurus diagonals */
                if (rows[0]->row[1] + rows[1]->row[2] + rows[2]->row[3] > magic_number
                        || rows[0]->row[1] + rows[1]->row[0] + rows[2]->row[3] > magic_number)
                    break;
                if (/* rows[0]->row[1] + rows[1]->row[2] + rows[2]->row[3] > magic_number */
                           rows[0]->row[2] + rows[1]->row[3] + rows[2]->row[0] > magic_number
                        || rows[0]->row[3] + rows[1]->row[0] + rows[2]->row[1] > magic_number
                        || rows[0]->row[0] + rows[1]->row[3] + rows[2]->row[2] > magic_number
                        /* || rows[0]->row[1] + rows[1]->row[0] + rows[2]->row[3] > magic_number */
                        || rows[0]->row[2] + rows[1]->row[1] + rows[2]->row[0] > magic_number)
                    continue;
            }
            if (strict & 1) {
                /* Quadrants */
                if (rows[1]->row[1] + rows[1]->row[2] + rows[2]->row[1] + rows[2]->row[2] != magic_number)
                    continue;
            }
            /* Remaining columns */
            if (rows[0]->row[2] + rows[1]->row[2] + rows[2]->row[2] > magic_number
                    || rows[0]->row[1] + rows[1]->row[1] + rows[2]->row[1] > magic_number
                    || rows[0]->row[0] + rows[1]->row[0] + rows[2]->row[0] > magic_number)
                continue;
            /* Diagonal from top left */
            if (rows[0]->row[0] + rows[1]->row[1] + rows[2]->row[2] > magic_number)
                continue;
            /* Diagonal from top right */
            if (rows[0]->row[3] + rows[1]->row[2] + rows[2]->row[1] > magic_number)
                continue;
            if (rows_duplicates_exist(rows, 3, size))
                continue;
            for (rows[3] = row_list; rows[3] != NULL; rows[3] = rows[3]->next) {
                /* Third column */
                if (rows[0]->row[3] + rows[1]->row[3] + rows[2]->row[3] + rows[3]->row[3] > magic_number)
                    break;
                /* Breakable diagonal from top left */
                if (rows[0]->row[0] + rows[1]->row[1] + rows[2]->row[2] + rows[3]->row[3] > magic_number)
                    break;
                if (strict & 2) {
                    /* Breakable taurus diagonal */
                    if (rows[0]->row[2] + rows[1]->row[1] + rows[2]->row[0] + rows[3]->row[3] > magic_number)
                        break;
                    /* Taurus diagonals */
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
                    if (rows[2]->row[0] + rows[2]->row[1] + rows[3]->row[0] + rows[3]->row[1] != magic_number
                            || rows[2]->row[2] + rows[2]->row[3] + rows[3]->row[2] + rows[3]->row[3] != magic_number)
                        continue;
                }
                /* delete reflectional and rotational symmetry by forcing top */
                /* left corner smallest and top right < bottom left */
                if (rows[0]->row[0] > rows[3]->row[3]   /* diagonal and rotational */
                        || rows[0]->row[0] > rows[3]->row[0]    /* vertical */
                        || rows[0]->row[3] > rows[3]->row[0])   /* diagonal */
                    continue;
                /* Columns correct */
                if (rows[0]->row[3] + rows[1]->row[3] + rows[2]->row[3] + rows[3]->row[3] != magic_number
                        || rows[0]->row[2] + rows[1]->row[2] + rows[2]->row[2] + rows[3]->row[2] != magic_number
                        || rows[0]->row[1] + rows[1]->row[1] + rows[2]->row[1] + rows[3]->row[1] != magic_number
                        || rows[0]->row[0] + rows[1]->row[0] + rows[2]->row[0] + rows[3]->row[0] != magic_number)
                    continue;
                /* Diagonal from top left */
                if (rows[0]->row[0] + rows[1]->row[1] + rows[2]->row[2] + rows[3]->row[3] != magic_number)
                    continue;
                /* Diagonal from top right */
                if (rows[0]->row[3] + rows[1]->row[2] + rows[2]->row[1] + rows[3]->row[0] != magic_number)
                    continue;
                if (rows_duplicates_exist(rows, 4, size))
                    continue;
                /* If made it here, this is a valid magic square */
                tmp = malloc(sizeof(struct square_list_entry));
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
    }
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
        magic_square_count = find_magic_squares(info->magic_number, info->size, info->row_list, seed_row_entry, &tail->squares, info->strict);
        info->magic_square_count += magic_square_count;
        if (magic_square_count)
            fprintf(stderr, "Thread %d\tfound %lld additional magic squares from seed row [%d %d %d %d]\n", info->thread_id, magic_square_count, seed_row_entry->row[0], seed_row_entry->row[1], seed_row_entry->row[2], seed_row_entry->row[3]);
        else
            fprintf(stderr, "Thread %d\tfound no magic squares from seed row [%d %d %d %d]\n", info->thread_id, seed_row_entry->row[0], seed_row_entry->row[1], seed_row_entry->row[2], seed_row_entry->row[3]);
        for (i = 0; seed_row_entry != NULL && i < info->total_threads; i++)
            seed_row_entry = seed_row_entry->next_seed;
    }
    pthread_exit(0);
}

char *USAGE = "USAGE: %s [-h] [-m NUMBER] [-p] [-s]\n"
"   -h          display this help message\n"
"   -m NUMBER   use NUMBER as the magic number\n"
"   -o OUTFILE  write results to the output file given by OUTFILE -- default is stdout\n"
"   -p          only allow positive (non-zero) numbers -- default allows 0\n"
"   -s          strict, where quadrants and the center must also add to the magic number\n"
"   -S          very strict, where diagonals on the square as a taurus must also add to the magic number\n"
"   -t THREADS  use THREADS number of execution threads -- default is all available threads\n";
int main(int argc, char **argv) {
    int magic_number = 33;
    struct row_list_entry *row_list, *seed_list;
    int row_count, seed_count, i;
    int total_squares = 0;
    unsigned int num_cores = sysconf(_SC_NPROCESSORS_ONLN);
    struct thread_info *thread_info_array;
    struct square_list_entry *current, *tmp_entry;
    struct square_list *tmp_list;
    int opt;
    char *outfilename = NULL;
    FILE *outfile = NULL;
    int strict = 0;
    while ((opt = getopt(argc, argv, "hm:o:psSt:")) != -1) {
        switch (opt) {
            case 'h':
                fprintf(stderr, USAGE, argv[0]);
                exit(1);
            case 'm':
                magic_number = atoi(optarg);
                break;
            case 'o':
                outfilename = optarg;
                break;
            case 'p':
                START_NUM = 1;
                break;
            case 's':
                strict |= 1;
                break;
            case 'S':
                strict |= 2;
                break;
            case 't':
                num_cores = atoi(optarg);
                break;
            default:
                fprintf(stderr, USAGE, argv[0]);
                exit(1);
        }
    }
    thread_info_array = malloc(sizeof(struct thread_info) * num_cores);
    compute_rows(magic_number, &row_list, &seed_list, &row_count, &seed_count);
    fprintf(stderr, "Found %d viable rows and %d viable seed rows.\n", row_count, seed_count);
    fprintf(stderr, "Running with %d processing threads.\n\n", num_cores);
    pthread_t *thread_array = malloc(sizeof(pthread_t) * num_cores);
    for (i = 0; i < num_cores; i++) {
        thread_info_array[i].thread_id = i;
        thread_info_array[i].total_threads = num_cores;
        thread_info_array[i].magic_number = magic_number;
        thread_info_array[i].size = SIZE;
        thread_info_array[i].strict = strict;
        thread_info_array[i].row_list = row_list;
        thread_info_array[i].seed_list = seed_list;
        pthread_create(thread_array + i, NULL, &thread_find_magic_squares, thread_info_array + i);
    }
    for (i = 0; i < num_cores; i++) {
        pthread_join(thread_array[i], NULL);
        total_squares += thread_info_array[i].magic_square_count;
    }
    fprintf(stderr, "There were %lld distinct magic squares found.\n\n", total_squares);
    if (outfilename != NULL)
        outfile = fopen(outfilename, "w");
    else
        outfile = stdout;
    /* Destructively print magic squares _in order_ across threads */
    for (i = 0; thread_info_array[i].square_lists != NULL; i = (i + 1) % num_cores) {
        current = thread_info_array[i].square_lists->squares;
        while (current != NULL) {
            print_magic_square(current->square, outfile);
            tmp_entry = current;
            current = current->next;
            free(tmp_entry);
        }
        tmp_list = thread_info_array[i].square_lists;
        thread_info_array[i].square_lists = tmp_list->next;
        free(tmp_list);
    }
    if (outfile != stdout)
        fclose(outfile);
}
