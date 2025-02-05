#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <mpi.h>
#include <openssl/des.h>

int main(int argc, char **argv) {
  double start, end;
  int rank, size;
  int i, j, k, l;
  int subtree;
  char guess[4];
  char* needle = "cca";
  int found = 0, local_found = 0;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  if (rank == 0)
    printf("starting run with %d processors\n", size);
  
  guess[3] = '\0';
  
  start = MPI_Wtime();
  for (i = 0; i < 3; i++) {
    for (j = 0; j < 3; j++) {
      subtree = i * 3 + j; 
      if (rank == subtree % size) {
        for (k = 0; k < 3; k++) {
          if (found == 1)
            goto gtfo;
          guess[0] = i + 97;
          guess[1] = j + 97;
          guess[2] = k + 97;
          //printf("%s owned by %d\n", guess, rank);

          if (strcmp(guess, needle) == 0) {
            printf("guess %s found by %d\n", guess, rank);
            local_found = 1;
            goto gtfo;
          }
        }
      }  
    }  
  }
gtfo:
  end = MPI_Wtime();
  MPI_Reduce(&local_found, &found, 1, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD);
  MPI_Barrier(MPI_COMM_WORLD);

  if (rank == 0) {
    printf("Password found: %s\n", guess);
    printf("elapsed time: %7.4f s\n", end-start);
  }
  MPI_Finalize();
}
