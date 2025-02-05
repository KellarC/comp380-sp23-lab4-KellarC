#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <mpi.h>
#include <openssl/des.h>

int main(int argc, char **argv) {
  double start = 0.0, end = 0.0;
  int rank, size;
  int i, j, k, l, m;
  int found = 0, local_found = 0;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  if (rank == 0)
    printf("starting run with %d processors\n", size);
  MPI_Barrier(MPI_COMM_WORLD);  

  char* encrypted_password = "wDqVEHozzDT1E"; // my encrypted_password = Q91ZjS22BeiWa 
  char* salt = "wD";                          // my salt = Q9

  
  char guess[6]; // 5 ascii characters + null terminator
  guess[5] = '\0'; // null terminate string
  char* test;
  int subtree;

  start = MPI_Wtime();
  for (i = 0; i < 95; i++) {
    for (j = 0; j < 95; j++) {
      subtree = i * 94 + j; // 0 to 8,835
      // allow ranks to search only their subtrees beyond 2nd level, leftovers get repeatedly assigned to rank 0 through size-1 
      if (rank == subtree % size) { 
        for (k = 0; k < 95; k++) {
          for (l = 0; l < 95; l++) {
            for (m = 0; m < 95; m++) {
              if (found == 1)
                goto gtfo;
              guess[0] = i + 32;
              guess[1] = j + 32;
              guess[2] = k + 32;
              guess[3] = l + 32;
              guess[4] = m + 32;
              
              test = DES_crypt(guess,salt);
              if (strcmp(test, encrypted_password) == 0) {
                printf("OG password: %s found by process %d\n", guess, rank);
                local_found = 1;
                goto gtfo;
              } 
            }
          }
        }
      }    
    }
  }

gtfo:
  end = MPI_Wtime();
  MPI_Reduce(&local_found, &found, 1, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD);
  MPI_Barrier(MPI_COMM_WORLD);

  if (rank == 0) 
    printf("elapsed time: %7.4f s\n", end-start);
  
  MPI_Finalize();
}
