#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>
#include <unistd.h>

#define UCO "524725"

int main(int argc, char *argv[]) {
    int rank, size;
    int active = 1;
    int coin_flip;
    int global_any_heads;
    int round = 1;
    int leader = -1;
    int active_count;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    if (rank == 0) {
        printf("%s\n", UCO);
        fflush(stdout);
    }
    
    MPI_Barrier(MPI_COMM_WORLD);

    if (rank != 0) {
        sleep(0.0001);
    }
    
    srand(time(NULL) + rank);
        
    while (1) {
        MPI_Allreduce(&active, &active_count, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
        
        if (active_count == 1) {
            break;
        }
        
        if (active_count == 0) {
            break;
        }
        
        if (active) {
            coin_flip = rand() % 2;
        } else {
            coin_flip = 0;
        }
        
        int local_heads = (active && coin_flip == 1) ? 1 : 0;
        MPI_Allreduce(&local_heads, &global_any_heads, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);
        
        if (global_any_heads == 1) {
            if (active && coin_flip == 0) {
                active = 0;
            }
        }
        
        round++;
        
        if (round > 1000) {
            break;
        }
    }
    
    int my_status = active ? rank : -1;
    MPI_Allreduce(&my_status, &leader, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);
    
    MPI_Barrier(MPI_COMM_WORLD);
    
    printf("Sloužím ti, můj vládče, slunce naše jasné. [Proces %d]\n", leader);
    fflush(stdout);
    
    MPI_Finalize();
    
    return 0;
}