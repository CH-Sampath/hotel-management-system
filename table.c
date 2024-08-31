#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/ipc.h>

#define READ_END 0
#define WRITE_END 1
#define MAX_MENU 20

#define SHM_SIZE 2048 // Size of the shared memory segment
#define READY_FLAG 0  // Indicates if data is ready to be read 1 means it ready to be read, 0 means it is ready to write
#define VALID 1       // if 1 wrong, if 2 right orderu8o
#define TOTAL_INDEX 2
#define C 3 // 0 means wait for table to write size of customers
#define CON 4
int ARRAY_INDEX = 5; // Starting index of the array in shared memory

void write_to_shared_memory(int *array, int size, int **shm_p)
{
    // while (*shm_p[READY_FLAG] == 1)
    // {
    //     usleep(100);
    // }
    printf("Enterring into writing shared memory\n");

    // WRITE 2 - writing size of array
    (*shm_p)[ARRAY_INDEX] = size;

    // WRITE 3 - writing the entire array
    for (int i = 0; i < size; i++)
    {
        (*shm_p)[ARRAY_INDEX + i + 1] = array[i];
    }
    ARRAY_INDEX = ARRAY_INDEX + size + 1;

    // //write 4 - writing if tis okay to read
    // (*shm_p)[READY_FLAG] = 1;
}
int main()
{
    FILE *file;
    int N, C_N;
    char line[100];
    // creation of shared memory and usage for waiter
    int shmid;
    int *shmptr;
    key_t key;

    // new to check if they want to take again
    int new = 0;

    // retake if not valid
    int n_v = 0;

    // open file to print menu
    // take the input of table number (to be assigned sequentially)
    printf("Enter Table number (Expected to give sequentially): \n");
    scanf("%d", &N);

    //take number of customers input
    printf("Enter the number of customers at table %d (maximum no. of customers can be 5):\n", N);
    scanf("%d", &C_N);

    // to communicate with the waiter process
    if ((key = ftok("table.c", N)) == -1)
    {
        perror("Error in ftok\n");
        return 1;
    }

    shmid = shmget(key, SHM_SIZE, 0666 | IPC_CREAT);
    if (shmid == -1)
    {
        printf("Error in shmget");
        return 1;
    }

    shmptr = shmat(shmid, NULL, 0);
    if (shmptr == (void *)-1)
    {
        printf("shmat error in table");
        exit(1);
    }

    //print menu in menu.txt
    file = fopen("menu.txt", "r");
    if (file == NULL)
    {
        printf("error opening the menu file: \n");
        exit(EXIT_FAILURE);
    }
    while (fgets(line, sizeof(line), file) != NULL)
    {
        printf("%s\n", line);
    }
    fclose(file);

    do {
        shmptr[CON] = 1;
        printf("%d is the shared memory total price index value\n", shmptr[TOTAL_INDEX]);
        // basic pipe requirements to create pipes from child to parent
        int pipefd[C_N][2]; // a 2-D array to hold the pipe file pointers
        pid_t pid[C_N];
        for (int i = 0; i < C_N; i++)
        {
            //array of 2 length, must be passed to this pipe
            if (pipe(pipefd[i]) == -1)
            {
                perror("Unable to create pipe\n");
                exit(EXIT_FAILURE);
            }
        }
        // create the processes
        for (int i = 0; i < C_N; i++)
        {

            pid[i] = fork();
            if (pid[i] == 0)
            {
                printf("Customer %d is ordering now\n", i);
                printf("enter menu items id, when completeted enter -1\n");
                // close the read end
                close(pipefd[i][READ_END]);
                // create an array of the menu items to be ordered
                int numbers[MAX_MENU];
                int input;
                int size = 0;
                while (1)
                {
                    scanf("%d", &input);
                    if (input == -1)
                    {
                        break; // Exit the loop when -1 is entered
                    }
                    numbers[size] = input; // Store the entered value in the array
                    size++;
                }
                // Write the size of the array to the pipe
                write(pipefd[i][WRITE_END], &size, sizeof(size));

                // write the array to the pipe
                write(pipefd[i][WRITE_END], numbers, size * sizeof(int));
                // close the write end
                close(pipefd[i][WRITE_END]);
                exit(EXIT_SUCCESS);
            }
            else
            {
                // close the write end for parent processes
                close(pipefd[i][WRITE_END]);
                wait(NULL);
            }
        }

        // WRITE 1 - write size of customers
        shmptr[C] = C_N;
        for (int i = 0; i < C_N; i++)
        {
            // read size of customers
            int size;
            read(pipefd[i][READ_END], &size, sizeof(int));
            printf("the menu size of %d child is %d\n", i, size);

            // read the array itslef
            int numbers[size]; // buffer to store the array from child
            read(pipefd[i][READ_END], numbers, size * sizeof(int));

            // printf("reading and printing array from child\n");
            // for (int j = 0; j < size; j++)
            // {
            //     printf("%d ", numbers[j]);
            // }            
            write_to_shared_memory(numbers, size, &shmptr);
            printf("\n");
            close(pipefd[i][READ_END]);
        }

        // remember that pipes are automatically cleaned when processes are destroyed.

        // WRITE 4 - write the ready flag
        shmptr[READY_FLAG] = 1;


        printf("Intially waiter validation %d\n", shmptr[VALID]);
        struct shmid_ds buf;
        printf("Waiting for waiter Validation\n");
        while (shmptr[VALID] == 0)
        {
            usleep(100);
        }
        printf("waiter Validation complete\n");
        if (shmptr[VALID] == 2)
        {
            while (shmptr[TOTAL_INDEX] == 0)
            {
                usleep(100);
            }
            printf("Your orcders are valid heres the bill %d\n", shmptr[TOTAL_INDEX]);

            // reset 1 - reset the valid flag for next time
            shmptr[VALID] = 0;

            // reset 2 - reset the total price
            shmptr[TOTAL_INDEX] = 0;

            // reatake if needed
            printf("Do you wish to enter a new set of customers if yes enter 1 or else enter -1\n");
            scanf("%d", &new);
            if (new == -1) {
                n_v = 0;
                new = 0;
            }

        }
        else
        {
            printf("Your orcders are NOT valid renter everything\n");
            n_v = 1;
        }

         // After completing the interaction
        if (n_v == 1 || new == 1) {
            // Waiter should continue, so set CON to 1 again if needed
            shmptr[CON] = 1;
        } else {
            // Set CON to 2 to indicate that the waiter can terminate
            shmptr[CON] = 2;
            printf("Table service complete. Notifying waiter to terminate.\n");
        }
        shmptr[READY_FLAG] = 0;
        shmptr[VALID] = 0;
        shmptr[TOTAL_INDEX] = 0;
        ARRAY_INDEX = 5;

        shmctl(shmid, IPC_STAT, &buf);
        printf("Number of processes attached: %ld\n", buf.shm_nattch);

    } while (n_v== 1 || new == 1);

    // detach table from shared memory
    if (shmdt(shmptr) == 0) {
        printf("Succesfully the table has detached \n");
    }
    else {
        printf("Not detached from shared memory\n");
    }
    struct shmid_ds buf;
    shmctl(shmid, IPC_STAT, &buf);
    printf("Number of processes attached: %d\n", buf.shm_nattch);

    // //delete shared memory
    // if (shmctl(shmid, IPC_RMID, NULL) == -1) {
    //     perror("Error in removing shared memory\n");
    // }
    // else {
    //     printf("Removed shared memory segment\n");
    // }
}