#include <stdio.h>

#define LOGSIZE (2)
#define SIZE (4)
#define E(m, r, c) (m[(r << LOGSIZE) + c])

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

void zero_row(int *matrix, int row) {
    E(matrix, row, 0) = 0;
    E(matrix, row, 1) = 0;
    E(matrix, row, 2) = 0;
    E(matrix, row, 3) = 0;
}

int increment_row(int *matrix, int row, int magic_number) {
    E(matrix, row, 0)++;
    if (E(matrix, row, 0) <= magic_number)
        return 0;
    E(matrix, row, 0) = 0;
    E(matrix, row, 1)++;
    if (E(matrix, row, 1) <= magic_number)
        return 0;
    E(matrix, row, 1) = 0;
    E(matrix, row, 2)++;
    if (E(matrix, row, 2) <= magic_number)
        return 0;
    E(matrix, row, 2) = 0;
    E(matrix, row, 3)++;
    if (E(matrix, row, 3) <= magic_number)
        return 0;
    return 1;
}

int main() {
    int magic_number = 33;
    int matrix[SIZE*SIZE] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    while (increment_row(matrix, 0, magic_number) == 0) {
        if (sum_of_row(matrix, 0) != magic_number)
            continue;
        if (duplicates_exist(matrix, 4))
            continue;
        zero_row(matrix, 1);
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
            zero_row(matrix, 2);
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
                zero_row(matrix, 3);
                while (increment_row(matrix, 3, magic_number) == 0) {
                    if (sum_of_row(matrix, 3) != magic_number)
                        continue;
                    if (duplicates_exist(matrix, 16))
                        continue;
                    if (columns_correct(matrix, magic_number) && diagonals_correct(matrix, magic_number))
                        print_magic_square(matrix);
                }
            }
        }
    }
}
