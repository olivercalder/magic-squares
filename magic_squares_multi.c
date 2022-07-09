#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

#define LOGSIZE (2)
#define SIZE (4)
#define E(s, r, c) ((s)[((r) << (LOGSIZE)) + (c)])

int START_NUM = 0;

struct row_list_entry {
    int row[SIZE];
    struct row_list_entry *next;
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
    struct square_list *square_lists;
};

int increment_row(int *square, int row, int magic_number) {
    E(square, row, 0)++;
    if (E(square, row, 0) <= magic_number)
        return 0;
    E(square, row, 0) = START_NUM;
    E(square, row, 1)++;
    if (E(square, row, 1) <= magic_number)
        return 0;
    E(square, row, 1) = START_NUM;
    E(square, row, 2)++;
    if (E(square, row, 2) <= magic_number)
        return 0;
    E(square, row, 2) = START_NUM;
    E(square, row, 3)++;
    if (E(square, row, 3) <= magic_number)
        return 0;
    return 1;
}

void print_magic_square(int *square, FILE *outfile) {
    int i;
    for (i = 0; i < 4; i++)
        fprintf(outfile, "%d %d %d %d\n", E(square, i, 0), E(square, i, 1), E(square, i, 2), E(square, i, 3));
    fprintf(outfile, "\n");
}

int sum_of_row(int *square, int row) {
    return E(square, row, 0) + E(square, row, 1) + E(square, row, 2) + E(square, row, 3);
}

int sum_of_col(int *square, int col) {
    return E(square, 0, col) + E(square, 1, col) + E(square, 2, col) + E(square, 3, col);
}

int columns_correct(int *square, int magic_number) {
    return sum_of_col(square, 0) == magic_number
        && sum_of_col(square, 1) == magic_number
        && sum_of_col(square, 2) == magic_number
        && sum_of_col(square, 3) == magic_number;
}

int diagonals_correct(int *square, int magic_number) {
    return (E(square, 0, 0) + E(square, 1, 1) + E(square, 2, 2) + E(square, 3, 3) == magic_number
            && E(square, 0, 3) + E(square, 1, 2) + E(square, 2, 1) + E(square, 3, 0) == magic_number);
}

int duplicates_exist(int *square, int index) {
    int i = 0;
    int j = 1;
    for (i = 0; i < index - 1; i++)
        for (j = i + 1; j < index; j++)
            if (square[i] == square[j])
                return 1;
    return 0;
}

void reset_row(int *square, int row) {
    E(square, row, 0) = START_NUM;
    E(square, row, 1) = START_NUM;
    E(square, row, 2) = START_NUM;
    E(square, row, 3) = START_NUM;
}

void compute_rows(int magic_number, struct row_list_entry **row_list, int *count) {
    struct row_list_entry *tmp, *tail;
    int row[SIZE] = {START_NUM, START_NUM, START_NUM, START_NUM};
    *row_list = NULL;
    *count = 0;
    while (increment_row(row, 0, magic_number) == 0) {
        if (sum_of_row(row, 0) != magic_number)
            continue;
        if (duplicates_exist(row, SIZE))
            continue;
        if (row[0] > row[3])    /* eliminate symmetrically equivalent rows */
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
    }
}

int find_magic_squares(int magic_number, int *seed_row, struct square_list_entry **square_list, int strict) {
    int magic_square_count = 0, tmp_sum;
    struct square_list_entry *tmp, *tail = NULL;
    int square[SIZE*SIZE] = {START_NUM, START_NUM, START_NUM, START_NUM, START_NUM, START_NUM, START_NUM, START_NUM, START_NUM, START_NUM, START_NUM, START_NUM, START_NUM, START_NUM, START_NUM, START_NUM};
    *square_list = NULL;
    square[0] = seed_row[0];
    square[1] = seed_row[1];
    square[2] = seed_row[2];
    square[3] = seed_row[3];
    while (increment_row(square, 1, magic_number) == 0) {
        /* Third column */
        if (E(square, 0, 3) + E(square, 1, 3) + START_NUM + START_NUM > magic_number)
            break;
        if (strict & 2) {
            /* Breakable taurus diagonals */
            if (E(square, 0, 2) + E(square, 1, 3) + START_NUM + START_NUM > magic_number
                    || E(square, 0, 0) + E(square, 1, 3) + START_NUM + START_NUM > magic_number)
                break;
        }
        /* Remaining columns */
        if (E(square, 0, 2) + E(square, 1, 2) + START_NUM + START_NUM > magic_number) {
            E(square, 1, 0) = magic_number;
            E(square, 1, 1) = magic_number;
            E(square, 1, 2) = magic_number;
            continue;
        }
        if (E(square, 0, 1) + E(square, 1, 1) + START_NUM + START_NUM > magic_number) {
            E(square, 1, 0) = magic_number;
            E(square, 1, 1) = magic_number;
            continue;
        }
        if (E(square, 0, 0) + E(square, 1, 0) + START_NUM + START_NUM > magic_number) {
            E(square, 1, 0) = magic_number;
            continue;
        }
        /* Diagonal from top left */
        if (E(square, 0, 0) + E(square, 1, 1) + START_NUM + START_NUM > magic_number) {
            E(square, 1, 0) = magic_number;
            E(square, 1, 1) = magic_number;
            continue;
        }
        /* Diagonal from top right */
        if (E(square, 0, 3) + E(square, 1, 2) + START_NUM + START_NUM > magic_number) {
            E(square, 1, 0) = magic_number;
            E(square, 1, 1) = magic_number;
            E(square, 1, 2) = magic_number;
            continue;
        }
        if (sum_of_row(square, 1) > magic_number) {
            E(square, 1, 0) = magic_number;
            continue;
        } else if (sum_of_row(square, 1) < magic_number) {
            E(square, 1, 0) = magic_number - E(square, 1, 1) - E(square, 1, 2) - E(square, 1, 3) - 1;
            continue;
        }
        if (strict & 2) {
            /* Taurus diagonals */
            if (E(square, 0, 1) + E(square, 1, 2) + START_NUM + START_NUM > magic_number
                    /* || E(square, 0, 2) + E(square, 1, 3) + START_NUM + START_NUM > magic_number */
                    || E(square, 0, 3) + E(square, 1, 0) + START_NUM + START_NUM > magic_number
                    /* || E(square, 0, 0) + E(square, 1, 3) + START_NUM + START_NUM > magic_number */
                    || E(square, 0, 1) + E(square, 1, 0) + START_NUM + START_NUM > magic_number
                    || E(square, 0, 2) + E(square, 1, 1) + START_NUM + START_NUM > magic_number)
                continue;
        }
        if (strict & 1) {
            /* Quadrants */
            if (E(square, 0, 0) + E(square, 0, 1) + E(square, 1, 0) + E(square, 1, 1) != magic_number)
                continue;
            if (E(square, 0, 2) + E(square, 0, 3) + E(square, 1, 2) + E(square, 1, 3) != magic_number)
                continue;
        }
        if (duplicates_exist(square, 8))
            continue;
        reset_row(square, 2);
        while (increment_row(square, 2, magic_number) == 0) {
            /* Once third row is set and valid, fourth can be computed directly */
            /* Third column */
            if ((tmp_sum = E(square, 0, 3) + E(square, 1, 3) + E(square, 2, 3)) + START_NUM > magic_number)
                break;
            E(square, 3, 3) = magic_number - tmp_sum;
            if (strict & 2) {
                /* Breakable taurus diagonals */
                if (E(square, 0, 1) + E(square, 1, 2) + E(square, 2, 3) + START_NUM > magic_number
                        || E(square, 0, 1) + E(square, 1, 0) + E(square, 2, 3) + START_NUM > magic_number)
                    break;
            }
            /* Remaining columns */
            if ((tmp_sum = E(square, 0, 2) + E(square, 1, 2) + E(square, 2, 2)) + START_NUM > magic_number) {
                E(square, 2, 0) = magic_number;
                E(square, 2, 1) = magic_number;
                E(square, 2, 2) = magic_number;
                continue;
            }
            E(square, 3, 2) = magic_number - tmp_sum;
            if ((tmp_sum = E(square, 0, 1) + E(square, 1, 1) + E(square, 2, 1)) + START_NUM > magic_number) {
                E(square, 2, 0) = magic_number;
                E(square, 2, 1) = magic_number;
                continue;
            }
            E(square, 3, 1) = magic_number - tmp_sum;
            if ((tmp_sum = E(square, 0, 0) + E(square, 1, 0) + E(square, 2, 0)) + START_NUM > magic_number) {
                E(square, 2, 0) = magic_number;
                continue;
            }
            E(square, 3, 0) = magic_number - tmp_sum;
            if (sum_of_row(square, 2) > magic_number) {
                E(square, 2, 0) = magic_number;
                continue;
            } else if (sum_of_row(square, 2) < magic_number) {
                E(square, 2, 0) = magic_number - E(square, 2, 1) - E(square, 2, 2) - E(square, 2, 3) - 1;
                continue;
            }
            /* Delete vertical summetry by forcing top left < bottom left */
            if (E(square, 0, 0) > E(square, 3, 0))
                continue;
            /* Delete diagonal summetry by forcing top right < bottom left */
            if (E(square, 0, 3) > E(square, 3, 0))
                continue;
            /* Delete diagonal summetry by forcing top left < bottom right */
            if (E(square, 0, 0) > E(square, 3, 3))
                continue;
            if (sum_of_row(square, 3) != magic_number
                    || !diagonals_correct(square, magic_number))
                continue;
            if (strict & 2) {
                /* Taurus diagonals */
                if (E(square, 0, 1) + E(square, 1, 2) + E(square, 2, 3) + E(square, 3, 0) != magic_number
                        || E(square, 0, 2) + E(square, 1, 3) + E(square, 2, 0) + E(square, 3, 1) != magic_number
                        || E(square, 0, 3) + E(square, 1, 0) + E(square, 2, 1) + E(square, 3, 2) != magic_number
                        || E(square, 0, 0) + E(square, 1, 3) + E(square, 2, 2) + E(square, 3, 1) != magic_number
                        || E(square, 0, 1) + E(square, 1, 0) + E(square, 2, 3) + E(square, 3, 2) != magic_number
                        || E(square, 0, 2) + E(square, 1, 1) + E(square, 2, 0) + E(square, 3, 3) != magic_number)
                    continue;
            }
            if (strict & 1) {
                /* Quadrants */
                if (E(square, 1, 1) + E(square, 1, 2) + E(square, 2, 1) + E(square, 2, 2) != magic_number
                        || E(square, 2, 0) + E(square, 2, 1) + E(square, 3, 0) + E(square, 3, 1) != magic_number
                        || E(square, 2, 2) + E(square, 2, 3) + E(square, 3, 2) + E(square, 3, 3) != magic_number)
                    continue;
            }
            if (duplicates_exist(square, 16))
                continue;
            /* If made it here, everything is correct */
            tmp = malloc(sizeof(struct square_list_entry));
            memcpy(tmp->square, square, sizeof(int) * SIZE*SIZE);
            tmp->next = NULL;
            if (*square_list == NULL)
                *square_list = tmp;
            else
                tail->next = tmp;
            tail = tmp;
            magic_square_count++;
        }
    }
    return magic_square_count;
}

void *thread_find_magic_squares(void *arg) {
    struct thread_info *info = (struct thread_info *)arg;
    struct row_list_entry *seed_row_entry = info->row_list;
    struct square_list *tmp, *tail;
    int i, magic_square_count = 0;
    for (i = 0; i < info->thread_id; i++)
        seed_row_entry = seed_row_entry->next;
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
        magic_square_count = find_magic_squares(info->magic_number, seed_row_entry->row, &tail->squares, info->strict);
        info->magic_square_count += magic_square_count;
        if (magic_square_count)
            fprintf(stderr, "Thread %d\tfound %lld additional magic squares from seed row [%d %d %d %d]\n", info->thread_id, magic_square_count, seed_row_entry->row[0], seed_row_entry->row[1], seed_row_entry->row[2], seed_row_entry->row[3]);
        else
            fprintf(stderr, "Thread %d\tfound no magic squares from seed row [%d %d %d %d]\n", info->thread_id, seed_row_entry->row[0], seed_row_entry->row[1], seed_row_entry->row[2], seed_row_entry->row[3]);
        for (i = 0; seed_row_entry != NULL && i < info->total_threads; i++)
            seed_row_entry = seed_row_entry->next;
    }
    pthread_exit(0);
}

char *USAGE = "USAGE: %s [-h] [-m NUMBER] [-p] [-s]\n"
"   -h          display this help message\n"
"   -m NUMBER   use NUMBER as the magic number\n"
"   -o OUTFILE  write results to the following output file -- default is stdout\n"
"   -p          only allow positive (non-zero) numbers -- default allows 0\n"
"   -s          strict, where quadrants and the center must also add to the magic number\n"
"   -S          very strict, where diagonals on the square as a taurus must also add to the magic number\n"
"   -t THREADS  use THREADS number of execution threads -- default is all available threads\n";

int main(int argc, char **argv) {
    int magic_number = 33;
    struct row_list_entry *row_list;
    int row_count, i;
    void *square_count = 0;
    unsigned long long total_squares = 0;
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
    compute_rows(magic_number, &row_list, &row_count);
    fprintf(stderr, "Found %d viable seed rows.\n", row_count);
    fprintf(stderr, "Running with %d processing threads.\n\n", num_cores);
    pthread_t *thread_array = malloc(sizeof(pthread_t) * num_cores);
    for (i = 0; i < num_cores; i++) {
        thread_info_array[i].thread_id = i;
        thread_info_array[i].total_threads = num_cores;
        thread_info_array[i].magic_number = magic_number;
        thread_info_array[i].size = SIZE;
        thread_info_array[i].strict = strict;
        thread_info_array[i].row_list = row_list;
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
