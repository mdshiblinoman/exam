/*
Problem_9: Divide an array among MPI processes. Each process sorts its local portion and exchanges data with neighboring processes to produce a globally sorted array.

Sample Input:
Number of processes: 4
Array: 45 12 78 3 56 23 89 10
Distribution:
P0: 45 12
P1: 78 3
P2: 56 23
P3: 89 10

Sample Output:
Local sorting:
P0: 12 45
P1: 3 78
P2: 23 56
P3: 10 89
After data exchange and merge/exchange steps:
Sorted array: 3 10 12 23 45 56 78 89

run project = /usr/bin/mpicc problem_9.c -o problem_9
then /usr/bin/mpirun -np 4 ./problem_9
*/

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define N 8

void keep_lower(int *local, int *received, int n)
{
    int *temp = (int *)malloc(2 * n * sizeof(int));

    int i = 0;
    int j = 0;
    int k = 0;

    while (i < n && j < n)
    {
        if (local[i] < received[j])
            temp[k++] = local[i++];
        else
            temp[k++] = received[j++];
    }

    while (i < n)
        temp[k++] = local[i++];

    while (j < n)
        temp[k++] = received[j++];

    for (i = 0; i < n; i++)
        local[i] = temp[i];

    free(temp);
}

void keep_upper(int *local, int *received, int n)
{
    int *temp = (int *)malloc(2 * n * sizeof(int));

    int i = 0;
    int j = 0;
    int k = 0;

    while (i < n && j < n)
    {
        if (local[i] < received[j])
            temp[k++] = local[i++];
        else
            temp[k++] = received[j++];
    }

    while (i < n)
        temp[k++] = local[i++];

    while (j < n)
        temp[k++] = received[j++];

    for (i = 0; i < n; i++)
        local[i] = temp[n + i];

    free(temp);
}

int main(int argc, char *argv[])
{
    int rank, size;
    int array[N] = {
        45, 12, 78, 3,
        56, 23, 89, 10};

    int local_size;
    int *local_array;
    int *received_array;

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (N % size != 0)
    {
        if (rank == 0)
        {
            printf("Array size must be divisible by number of processes.\n");
        }

        MPI_Finalize();
        return 0;
    }

    local_size = N / size;
    local_array = (int *)malloc(local_size * sizeof(int));
    received_array = (int *)malloc(local_size * sizeof(int));

    MPI_Scatter(
        array,
        local_size,
        MPI_INT,
        local_array,
        local_size,
        MPI_INT,
        0,
        MPI_COMM_WORLD);

    for (int i = 0; i < local_size - 1; i++)
    {
        for (int j = 0; j < local_size - i - 1; j++)
        {
            if (local_array[j] > local_array[j + 1])
            {
                int temp = local_array[j];

                local_array[j] = local_array[j + 1];
                local_array[j + 1] = temp;
            }
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);
    if (rank == 0)
        printf("\nLocal sorting:\n");

    MPI_Barrier(MPI_COMM_WORLD);

    for (int r = 0; r < size; r++)
    {
        if (rank == r)
        {
            printf("P%d: ", rank);

            for (int i = 0; i < local_size; i++)
                printf("%d ", local_array[i]);

            printf("\n");
        }

        MPI_Barrier(MPI_COMM_WORLD);
    }

    for (int phase = 0; phase < size; phase++)
    {
        int partner = -1;

        if (phase % 2 == 0)
        {
            if (rank % 2 == 0)
                partner = rank + 1;
            else
                partner = rank - 1;
        }
        else
        {
            if (rank % 2 == 0)
                partner = rank - 1;
            else
                partner = rank + 1;
        }

        if (partner >= 0 && partner < size)
        {
            MPI_Sendrecv(
                local_array,
                local_size,
                MPI_INT,
                partner,
                100,
                received_array,
                local_size,
                MPI_INT,
                partner,
                100,
                MPI_COMM_WORLD,
                MPI_STATUS_IGNORE);

            if (rank < partner)
            {
                keep_lower(
                    local_array,
                    received_array,
                    local_size);
            }
            else
            {
                keep_upper(
                    local_array,
                    received_array,
                    local_size);
            }
        }
    }

    MPI_Gather(
        local_array,
        local_size,
        MPI_INT,
        array,
        local_size,
        MPI_INT,
        0,
        MPI_COMM_WORLD);

    if (rank == 0)
    {
        printf("\nAfter data exchange and merge/exchange steps:\n");

        printf("Sorted array: ");

        for (int i = 0; i < N; i++)
            printf("%d ", array[i]);

        printf("\n");
    }

    free(local_array);
    free(received_array);

    MPI_Finalize();

    return 0;
}
