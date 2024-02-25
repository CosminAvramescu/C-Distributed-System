#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    int numtasks, rank, workersNo, coord, N;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    FILE *fp_in;

    if (rank == 0)
    {
        N = atoi(argv[1]);
        int v[N];
        for (int i = 0; i < N; i++)
        {
            v[i] = N - i - 1;
        }
        MPI_Status status;

        fp_in = fopen("cluster0.txt", "r");
        fscanf(fp_in, "%d", &workersNo);

        //topology-matrice; fiecare cluster are un vector de workeri
        //prima linie din fiecare vector din matrice reprezinta numarul sau de workeri
        int topology[4][10];
        topology[0][0] = workersNo;
        for (int i = 1; i <= workersNo; i++)
        {
            fscanf(fp_in, "%d", &topology[0][i]);
        }
        for (int i = 1; i <= workersNo; i++)
        {
            printf("M(%d,%d)\n", rank, topology[0][i]);
            MPI_Send(&rank, 1, MPI_INT, topology[0][i], 0, MPI_COMM_WORLD);
        }

        printf("M(%d,%d)\n", rank, 3);
        MPI_Send(topology, 4 * 10, MPI_INT, 3, 0, MPI_COMM_WORLD);
        MPI_Recv(topology, 4 * 10, MPI_INT, 3, 0, MPI_COMM_WORLD, &status);

        for (int i = 1; i <= workersNo; i++)
        {
            printf("M(%d,%d)\n", rank, topology[0][i]);
            MPI_Send(topology, 4 * 10, MPI_INT, topology[0][i], 0, MPI_COMM_WORLD);
        }

        //se afiseaza topologia
        printf("%d ->", rank);
        for (int i = 0; i < 4; i++)
        {
            printf(" %d:", i);
            for (int j = 1; j <= topology[i][0]; j++)
            {
                if (j == topology[i][0])
                {
                    printf("%d", topology[i][j]);
                }
                else
                {
                    printf("%d,", topology[i][j]);
                }
            }
        }
        printf("\n");

        // COMPUTE VECTOR --------------------------------------------------------------------

        int totalWorkers = 0, loadPerWorker, rest;
        for (int i = 0; i < 4; i++)
        {
            totalWorkers += topology[i][0];
        }
        loadPerWorker = N / totalWorkers;
        rest = N % totalWorkers;

        //se trimit mesajele pentru procesul care preia excesul de operatii (rest)
        //in caz ca N nu se imparte exact la totalWorkers
        printf("M(%d,%d)\n", rank, topology[0][1]);
        MPI_Send(&N, 1, MPI_INT, topology[0][1], 0, MPI_COMM_WORLD);
        MPI_Send(&totalWorkers, 1, MPI_INT, topology[0][1], 0, MPI_COMM_WORLD);
        MPI_Send(v, N, MPI_INT, topology[0][1], 0, MPI_COMM_WORLD);
        MPI_Recv(v, N, MPI_INT, topology[0][1], 0, MPI_COMM_WORLD, &status);

        int offset = (loadPerWorker + rest) * sizeof(int);
        for (int i = 2; i <= workersNo; i++)
        {
            //celelalte procese din cluster 0 isi calculeaza bucata de vector
            printf("M(%d,%d)\n", rank, topology[0][i]);
            MPI_Send(&N, 1, MPI_INT, topology[0][i], 0, MPI_COMM_WORLD);
            MPI_Send(&totalWorkers, 1, MPI_INT, topology[0][i], 0, MPI_COMM_WORLD);
            MPI_Send(&offset, 1, MPI_INT, topology[0][i], 0, MPI_COMM_WORLD);
            MPI_Send(v, N, MPI_INT, topology[0][i], 0, MPI_COMM_WORLD);
            MPI_Recv(v, N, MPI_INT, topology[0][i], 0, MPI_COMM_WORLD, &status);
            offset += (loadPerWorker * sizeof(int));
        }

        //se da mai departe informatia la coordonatorul 3
        printf("M(%d,%d)\n", rank, 3);
        MPI_Send(&N, 1, MPI_INT, 3, 0, MPI_COMM_WORLD);
        MPI_Send(&totalWorkers, 1, MPI_INT, 3, 0, MPI_COMM_WORLD);
        MPI_Send(&offset, 1, MPI_INT, 3, 0, MPI_COMM_WORLD);

        MPI_Send(v, N, MPI_INT, 3, 0, MPI_COMM_WORLD);
        MPI_Recv(v, N, MPI_INT, 3, 0, MPI_COMM_WORLD, &status);

        //se afiseaza vectorul dupa revenirea acestuia de la coordonatorul 3
        printf("Rezultat: ");
        for (int i = 0; i < N; i++)
        {
            if (i == N - 1)
            {
                printf("%d", v[i]);
            }
            else
            {
                printf("%d ", v[i]);
            }
        }
        fclose(fp_in);
    }
    else if (rank == 3)
    {
        MPI_Status status;

        fp_in = fopen("cluster3.txt", "r");
        fscanf(fp_in, "%d", &workersNo);

        int topology[4][10];
        
        //citire workeri pentru cluster 3
        MPI_Recv(topology, 4 * 10, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        topology[3][0] = workersNo;
        for (int i = 1; i <= workersNo; i++)
        {
            fscanf(fp_in, "%d", &topology[3][i]);
        }
        for (int i = 1; i <= workersNo; i++)
        {
            printf("M(%d,%d)\n", rank, topology[3][i]);
            MPI_Send(&rank, 1, MPI_INT, topology[3][i], 0, MPI_COMM_WORLD);
        }
        printf("M(%d,%d)\n", rank, 2);
        MPI_Send(topology, 4 * 10, MPI_INT, 2, 0, MPI_COMM_WORLD);
        MPI_Recv(topology, 4 * 10, MPI_INT, 2, 0, MPI_COMM_WORLD, &status);
        //pe revenirea de la cluster 2, se trimite topologia completa
        //catre toti workerii de la cluster 3
        for (int i = 1; i <= workersNo; i++)
        {
            printf("M(%d,%d)\n", rank, topology[3][i]);
            MPI_Send(topology, 4 * 10, MPI_INT, topology[3][i], 0, MPI_COMM_WORLD);
        }
        printf("M(%d,%d)\n", rank, 0);
        MPI_Send(topology, 4 * 10, MPI_INT, 0, 0, MPI_COMM_WORLD);

        //se afiseaza topologia
        printf("%d ->", rank);
        for (int i = 0; i < 4; i++)
        {
            printf(" %d:", i);
            for (int j = 1; j <= topology[i][0]; j++)
            {
                if (j == topology[i][0])
                {
                    printf("%d", topology[i][j]);
                }
                else
                {
                    printf("%d,", topology[i][j]);
                }
            }
        }
        printf("\n");

        // COMPUTE VECTOR --------------------------------------------------------------------

        int totalWorkers, loadPerWorker, offset;
        MPI_Recv(&N, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        int v[N];
        MPI_Recv(&totalWorkers, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        MPI_Recv(&offset, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);

        loadPerWorker = N / totalWorkers;

        MPI_Recv(v, N, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        // se trimit vectorul si offset-ul catre fiecare worker din cluster 3
        for (int i = 1; i <= workersNo; i++)
        {
            printf("M(%d,%d)\n", rank, topology[3][i]);
            MPI_Send(&N, 1, MPI_INT, topology[3][i], 0, MPI_COMM_WORLD);
            MPI_Send(&totalWorkers, 1, MPI_INT, topology[3][i], 0, MPI_COMM_WORLD);
            MPI_Send(&offset, 1, MPI_INT, topology[3][i], 0, MPI_COMM_WORLD);
            MPI_Send(v, N, MPI_INT, topology[3][i], 0, MPI_COMM_WORLD);
            MPI_Recv(v, N, MPI_INT, topology[3][i], 0, MPI_COMM_WORLD, &status);
            offset += (sizeof(int) * loadPerWorker);
        }

        //se trimit calculele ramase mai departe catre cluster 2
        printf("M(%d,%d)\n", rank, 2);
        MPI_Send(&N, 1, MPI_INT, 2, 0, MPI_COMM_WORLD);
        MPI_Send(&totalWorkers, 1, MPI_INT, 2, 0, MPI_COMM_WORLD);
        MPI_Send(&offset, 1, MPI_INT, 2, 0, MPI_COMM_WORLD);
        MPI_Send(v, N, MPI_INT, 2, 0, MPI_COMM_WORLD);

        MPI_Recv(v, N, MPI_INT, 2, 0, MPI_COMM_WORLD, &status);
        //revine informatia de la cluster 2 (workerii de la 1 si de la 2 si-au terminat treaba)
        printf("M(%d,%d)\n", rank, 0);
        //se trimite inapoi catre 0
        MPI_Send(v, N, MPI_INT, 0, 0, MPI_COMM_WORLD);

        fclose(fp_in);
    }
    else if (rank == 2)
    {
        MPI_Status status;

        fp_in = fopen("cluster2.txt", "r");
        fscanf(fp_in, "%d", &workersNo);

        int topology[4][10];

        //aceleasi principii ca la cluster 3
        MPI_Recv(topology, 4 * 10, MPI_INT, 3, 0, MPI_COMM_WORLD, &status);
        topology[2][0] = workersNo;
        for (int i = 1; i <= workersNo; i++)
        {
            fscanf(fp_in, "%d", &topology[2][i]);
        }
        for (int i = 1; i <= workersNo; i++)
        {
            printf("M(%d,%d)\n", rank, topology[2][i]);
            MPI_Send(&rank, 1, MPI_INT, topology[2][i], 0, MPI_COMM_WORLD);
        }
        printf("M(%d,%d)\n", rank, 1);
        MPI_Send(topology, 4 * 10, MPI_INT, 1, 0, MPI_COMM_WORLD);
        MPI_Recv(topology, 4 * 10, MPI_INT, 1, 0, MPI_COMM_WORLD, &status);
        for (int i = 1; i <= workersNo; i++)
        {
            printf("M(%d,%d)\n", rank, topology[2][i]);
            MPI_Send(topology, 4 * 10, MPI_INT, topology[2][i], 0, MPI_COMM_WORLD);
        }
        printf("M(%d,%d)\n", rank, 3);
        MPI_Send(topology, 4 * 10, MPI_INT, 3, 0, MPI_COMM_WORLD);

        //se afiseaza topologia
        printf("%d ->", rank);
        for (int i = 0; i < 4; i++)
        {
            printf(" %d:", i);
            for (int j = 1; j <= topology[i][0]; j++)
            {
                if (j == topology[i][0])
                {
                    printf("%d", topology[i][j]);
                }
                else
                {
                    printf("%d,", topology[i][j]);
                }
            }
        }
        printf("\n");

        // COMPUTE VECTOR --------------------------------------------------------------------

        int totalWorkers, loadPerWorker, offset;
        MPI_Recv(&N, 1, MPI_INT, 3, 0, MPI_COMM_WORLD, &status);
        int v[N];
        MPI_Recv(&totalWorkers, 1, MPI_INT, 3, 0, MPI_COMM_WORLD, &status);
        MPI_Recv(&offset, 1, MPI_INT, 3, 0, MPI_COMM_WORLD, &status);

        loadPerWorker = N / totalWorkers;

        MPI_Recv(v, N, MPI_INT, 3, 0, MPI_COMM_WORLD, &status);
        for (int i = 1; i <= workersNo; i++)
        {
            printf("M(%d,%d)\n", rank, topology[2][i]);
            MPI_Send(&N, 1, MPI_INT, topology[2][i], 0, MPI_COMM_WORLD);
            MPI_Send(&totalWorkers, 1, MPI_INT, topology[2][i], 0, MPI_COMM_WORLD);
            MPI_Send(&offset, 1, MPI_INT, topology[2][i], 0, MPI_COMM_WORLD);
            MPI_Send(v, N, MPI_INT, topology[2][i], 0, MPI_COMM_WORLD);
            MPI_Recv(v, N, MPI_INT, topology[2][i], 0, MPI_COMM_WORLD, &status);
            offset += (sizeof(int) * loadPerWorker);
        }

        printf("M(%d,%d)\n", rank, 1);
        MPI_Send(&N, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
        MPI_Send(&totalWorkers, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
        MPI_Send(&offset, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
        MPI_Send(v, N, MPI_INT, 1, 0, MPI_COMM_WORLD);

        MPI_Recv(v, N, MPI_INT, 1, 0, MPI_COMM_WORLD, &status);

        printf("M(%d,%d)\n", rank, 3);
        MPI_Send(v, N, MPI_INT, 3, 0, MPI_COMM_WORLD);

        fclose(fp_in);
    }
    else if (rank == 1)
    {
        MPI_Status status;

        fp_in = fopen("cluster1.txt", "r");
        fscanf(fp_in, "%d", &workersNo);

        int topology[4][10];

        MPI_Recv(topology, 4 * 10, MPI_INT, 2, 0, MPI_COMM_WORLD, &status);
        topology[1][0] = workersNo;
        for (int i = 1; i <= workersNo; i++)
        {
            fscanf(fp_in, "%d", &topology[1][i]);
        }
        for (int i = 1; i <= workersNo; i++)
        {
            printf("M(%d,%d)\n", rank, topology[1][i]);
            MPI_Send(&rank, 1, MPI_INT, topology[1][i], 0, MPI_COMM_WORLD);
        }
        for (int i = 1; i <= workersNo; i++)
        {
            printf("M(%d,%d)\n", rank, topology[1][i]);
            MPI_Send(topology, 4 * 10, MPI_INT, topology[1][i], 0, MPI_COMM_WORLD);
        }
        printf("M(%d,%d)\n", rank, 2);
        MPI_Send(topology, 4 * 10, MPI_INT, 2, 0, MPI_COMM_WORLD);

        //se afiseaza topologia
        printf("%d ->", rank);
        for (int i = 0; i < 4; i++)
        {
            printf(" %d:", i);
            for (int j = 1; j <= topology[i][0]; j++)
            {
                if (j == topology[i][0])
                {
                    printf("%d", topology[i][j]);
                }
                else
                {
                    printf("%d,", topology[i][j]);
                }
            }
        }
        printf("\n");

        // COMPUTE VECTOR --------------------------------------------------------------------
        int totalWorkers, loadPerWorker, offset;
        MPI_Recv(&N, 1, MPI_INT, 2, 0, MPI_COMM_WORLD, &status);
        int v[N];
        MPI_Recv(&totalWorkers, 1, MPI_INT, 2, 0, MPI_COMM_WORLD, &status);
        MPI_Recv(&offset, 1, MPI_INT, 2, 0, MPI_COMM_WORLD, &status);

        loadPerWorker = N / totalWorkers;

        MPI_Recv(v, N, MPI_INT, 2, 0, MPI_COMM_WORLD, &status);
        for (int i = 1; i <= workersNo; i++)
        {
            printf("M(%d,%d)\n", rank, topology[1][i]);
            MPI_Send(&N, 1, MPI_INT, topology[1][i], 0, MPI_COMM_WORLD);
            MPI_Send(&totalWorkers, 1, MPI_INT, topology[1][i], 0, MPI_COMM_WORLD);
            MPI_Send(&offset, 1, MPI_INT, topology[1][i], 0, MPI_COMM_WORLD);
            MPI_Send(v, N, MPI_INT, topology[1][i], 0, MPI_COMM_WORLD);
            MPI_Recv(v, N, MPI_INT, topology[1][i], 0, MPI_COMM_WORLD, &status);
            offset += (sizeof(int) * loadPerWorker);
        }

        printf("M(%d,%d)\n", rank, 2);
        MPI_Send(v, N, MPI_INT, 2, 0, MPI_COMM_WORLD);

        fclose(fp_in);
    }
    else
    {
        int topology[4][10];
        MPI_Status status;
        MPI_Recv(&coord, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
        MPI_Recv(topology, 4 * 10, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);

        //se afiseaza topologia
        printf("%d ->", rank);
        for (int i = 0; i < 4; i++)
        {
            printf(" %d:", i);
            for (int j = 1; j <= topology[i][0]; j++)
            {
                if (j == topology[i][0])
                {
                    printf("%d", topology[i][j]);
                }
                else
                {
                    printf("%d,", topology[i][j]);
                }
            }
        }
        printf("\n");

        int N, totalWorkers, offset;

        MPI_Recv(&N, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
        int v[N];
        MPI_Recv(&totalWorkers, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);

        //doar primul proces din cluster-ul 0 va prelua excesul de operatii de executat (rest)
        //in caz ca N nu se imparte exact la totalWorkers
        int loadPerWorker = N / totalWorkers;
        int rest = N % totalWorkers;
        if (rank == topology[0][1])
        {
            MPI_Recv(v, N, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
            for (int i = 0; i < loadPerWorker + rest; i++)
            {
                v[i] *= 5;
            }
            printf("M(%d,%d)\n", rank, coord);
            MPI_Send(v, N, MPI_INT, coord, 0, MPI_COMM_WORLD);
        }
        else
        {
            MPI_Recv(&offset, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
            MPI_Recv(v, N, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
            for (int i = offset / sizeof(int); i < loadPerWorker + offset / sizeof(int); i++)
            {
                v[i] *= 5;
            }
            printf("M(%d,%d)\n", rank, coord);
            MPI_Send(v, N, MPI_INT, coord, 0, MPI_COMM_WORLD);
        }
    }

    MPI_Finalize();
}
