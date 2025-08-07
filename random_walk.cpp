#include <iostream>
#include <cstdlib>  // For atoi, rand, srand
#include <ctime>    // For time
#include <mpi.h>

int domain_size;
int max_steps;
int world_rank;
int world_size;

void walker_process() {
    // Seed random number generator uniquely for each rank
    srand(time(NULL) * world_rank);

    int position = 0;
    int steps_taken = 0;

    while (steps_taken < max_steps) {
        // Move: -1 or +1
        int step = (rand() % 2 == 0) ? -1 : 1;
        position += step;
        steps_taken++;

        if (position < -domain_size || position > domain_size) {
            // Finished: out of domain
            std::cout << "Rank " << world_rank << ": Walker finished in " << steps_taken << " steps." << std::endl;
            MPI_Send(&steps_taken, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
            return;
        }
    }

    // Finished: reached max steps but still within domain
    std::cout << "Rank " << world_rank << ": Walker finished in " << steps_taken << " steps." << std::endl;
    MPI_Send(&steps_taken, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
}

void controller_process() {
    int num_walkers = world_size - 1;
    int steps;
    MPI_Status status;

    for (int i = 0; i < num_walkers; ++i) {
        MPI_Recv(&steps, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
        // Optional per-walker output
        // std::cout << "Controller: Received from Rank " << status.MPI_SOURCE << ", steps = " << steps << std::endl;
    }

    // Final output (required)
    std::cout << "Controller: All " << num_walkers << " walkers have finished." << std::endl;
}

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // Check argument count
    if (argc != 3) {
        if (world_rank == 0) {
            std::cerr << "Usage: mpirun -np <p> " << argv[0] << " <domain_size> <max_steps>" << std::endl;
        }
        MPI_Finalize();
        return 1;
    }

    // Parse command-line arguments
    domain_size = atoi(argv[1]);
    max_steps = atoi(argv[2]);

    if (world_rank == 0) {
        controller_process();
    } else {
        walker_process();
    }

    MPI_Finalize();
    return 0;
}
