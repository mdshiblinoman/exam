/*
Problem_2: The root process distributes different rows of a matrix to worker processes using MPI_Send. Workers calculate the sum of their assigned rows and return the results using MPI_Recv.

Sample Input:
Number of processes: 4
Matrix:
1 2 3 4
5 6 7 8
9 10 11 12
13 14 15 16
Each process receives one row.

Sample Output:
P0: Row = 1 2 3 4
P1: Row = 5 6 7 8
P2: Row = 9 10 11 12
P3: Row = 13 14 15 16
Sum = 10
Sum = 26
Sum = 42
Sum = 58
Total matrix sum = 136

run project = /usr/bin/mpicc problem_2.c -o problem_2
then /usr/bin/mpirun -np 4 ./problem_2
*/

#include <stdio.h>
#include <mpi.h>

#define ROWS 4
#define COLS 4

int main(int argc, char *argv[])
{
    int rank, size;

    int matrix[ROWS][COLS] = {
        {1, 2, 3, 4},
        {5, 6, 7, 8},
        {9, 10, 11, 12},
        {13, 14, 15, 16}};

    int row[COLS];
    int row_sum;
    int total_sum = 0;

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    /* We need exactly 4 processes */
    if (size != 4)
    {
        if (rank == 0)
        {
            printf("Please run the program with 4 processes.\n");
        }

        MPI_Finalize();
        return 0;
    }

    if (rank == 0)
    {
        /* Root gets its own row */
        for (int j = 0; j < COLS; j++)
        {
            row[j] = matrix[0][j];
        }

        for (int i = 1; i < 4; i++)
        {
            MPI_Send(
                matrix[i],
                COLS,
                MPI_INT,
                i,
                100,
                MPI_COMM_WORLD);
        }
    }
    else
    {
        MPI_Recv(
            row,
            COLS,
            MPI_INT,
            0,
            100,
            MPI_COMM_WORLD,
            MPI_STATUS_IGNORE);
    }

    row_sum = 0;

    for (int j = 0; j < COLS; j++)
    {
        row_sum += row[j];
    }

    for (int p = 0; p < size; p++)
    {
        MPI_Barrier(MPI_COMM_WORLD);

        if (rank == p)
        {
            printf("P%d: Row = ", rank);

            for (int j = 0; j < COLS; j++)
            {
                printf("%d ", row[j]);
            }

            printf("\n");
            printf("Sum = %d\n", row_sum);

            fflush(stdout);
        }
    }

    if (rank == 0)
    {
        total_sum = row_sum;

        int received_sum;

        for (int i = 1; i < 4; i++)
        {
            MPI_Recv(
                &received_sum,
                1,
                MPI_INT,
                i,
                200,
                MPI_COMM_WORLD,
                MPI_STATUS_IGNORE);

            total_sum += received_sum;
        }

        total_sum += received_sum;

        printf("Total matrix sum = %d\n", total_sum);
    }
    else
    {
        MPI_Send(
            &row_sum,
            1,
            MPI_INT,
            0,
            200,
            MPI_COMM_WORLD);
    }

    MPI_Finalize();

    return 0;
}