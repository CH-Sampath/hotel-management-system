#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/ipc.h>

#define BUFF_SIZE 100

int main() {
    int ans;
    key_t key;
    int shmid;
    int *shmptr;

    FILE *file = fopen("earnings.txt", "w");
    fclose(file);
    // Generate a unique key
    if ((key = ftok("admin.c", '3')) == -1) {
        perror("Error in ftok function");
        exit(1);
    }

    // Create or access shared memory segment
    shmid = shmget(key, BUFF_SIZE, 0666 | IPC_CREAT);
    if (shmid == -1) {
        perror("Error in shmget function");
        exit(1);
    }

    // Attach to the shared memory segment
    shmptr = shmat(shmid, NULL, 0);
    if (shmptr == (void *)-1) {
        perror("Error in shmat function");
        exit(1);
    }

    while (1) {
        printf("Do you want to close the hotel (y/n)? ");
        scanf("%d", &ans);  // Note the space before %c to ignore leading whitespace
        
        if (ans == 1) {
            shmptr[0] = 1;  // Set flag to indicate hotel should be closed
            printf("Hotel closing initiated.\n");
            break;  // Exit the loop after setting the flag
        } else if (ans == -1) {
            printf("Hotel remains open.\n");
        } else {
            printf("Invalid input. Please enter 'y' or 'n'.\n");
        }
    }

    // Detach from shared memory
    if (shmdt(shmptr) == -1) {
        perror("Error in shmdt function");
        exit(1);
    }

    

    return 0;
}
