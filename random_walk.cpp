#include <iostream>
#include <cstdlib>
#include <ctime>
#include <mpi.h>

void runWalker();
void runCoordinator();

int boundaryLimit;
int stepLimit;
int processId;
int totalProcesses;

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &totalProcesses);
    MPI_Comm_rank(MPI_COMM_WORLD, &processId);

    if (argc != 3)
    {
        if (processId == 0)
        {
            std::cerr << "Usage: mpirun -np <num_procs> " << argv[0] << " <domain_limit> <max_steps>" << std::endl;
        }
        MPI_Finalize();
        return 1;
    }

    boundaryLimit = atoi(argv[1]);
    stepLimit = atoi(argv[2]);

    if (processId == 0)
    {
        runCoordinator();
    }
    else
    {
        runWalker();
    }

    MPI_Finalize();
    return 0;
}

void runWalker()
{
    srand(time(NULL) + processId);

    int currentPos = 0;
    int stepCount = 0;

    std::cout << "Process " << processId << ": Starting random walk..." << std::endl;

    while (stepCount < stepLimit)
    {
        int direction = (rand() % 2 == 0) ? -1 : 1;
        currentPos += direction;
        stepCount++;

        if (currentPos < -boundaryLimit || currentPos > boundaryLimit)
        {
            std::cout << "Process " << processId << ": Walker finished in " << stepCount << " steps." << std::endl;
            MPI_Send(&stepCount, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
            return;
        }
    }

    std::cout << "Process " << processId << ": Reached max steps (" << stepLimit << "), final position = " << currentPos << std::endl;
    std::cout << "Process " << processId << ": Walker finished in " << stepCount << " steps." << std::endl;
    MPI_Send(&stepCount, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
}

void runCoordinator()
{
    int activeWalkers = totalProcesses - 1;
    int recvSteps;

    for (int i = 0; i < activeWalkers; ++i)
    {
        MPI_Status msgStatus;
        MPI_Recv(&recvSteps, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &msgStatus);
        std::cout << "Coordinator: Received completion from Process " << msgStatus.MPI_SOURCE
                  << " who took " << recvSteps << " steps." << std::endl;
    }

    std::cout << "Coordinator: All " << activeWalkers << " walkers have finished." << std::endl;
}
