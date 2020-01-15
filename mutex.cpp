#include <mpi.h>
#include <iostream>

#define _XOPEN_SOURCE

using namespace std;

int size, pid;



int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &pid);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    cout << "Hello from: " << size << ", " << pid << endl;
    
    MPI_Finalize();
    return 0;
}
