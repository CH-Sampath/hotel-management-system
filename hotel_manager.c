#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/ipc.h>

#define BUFF_SIZE 100

#define FILENAME "earnings.txt"

// Function to calculate and display financials, and append them to the file
void calculate_and_display_financials()
{
    FILE *file;
    FILE *file_append;
    char line[256];
    int total_earnings = 0;
    int earnings;
    int total_wages;
    int profit;

    // Open the earnings file in read mode to read existing earnings
    file = fopen(FILENAME, "r");
    if (file == NULL)
    {
        perror("Error opening earnings file for reading");
        exit(1);
    }

    // Read each line from the file and sum up the earnings
    while (fgets(line, sizeof(line), file) != NULL)
    {
        // Extract the earnings value from the line
        if (sscanf(line, "Earning from Table %*d: %d INR", &earnings) == 1)
        {
            total_earnings += earnings;
        }
    }

    // Close the file after reading
    if (fclose(file) != 0)
    {
        perror("Error closing earnings file after reading");
        exit(1);
    }

    // Calculate total wages (40% of total earnings)
    total_wages = (total_earnings * 40) / 100;

    // Calculate profit
    profit = total_earnings - total_wages;

    // Open the earnings file in append mode to add the calculated values
    file_append = fopen(FILENAME, "a");
    if (file_append == NULL)
    {
        perror("Error opening earnings file for appending");
        exit(1);
    }

    // Append the calculated values to the file
    fprintf(file_append, "Total Earnings of Hotel: %d INR\n", total_earnings);
    fprintf(file_append, "Total Wages of Waiters: %d INR\n", total_wages);
    fprintf(file_append, "Total Profit: %d INR\n", profit);

    // Close the file after appending
    if (fclose(file_append) != 0)
    {
        perror("Error closing earnings file after appending");
        exit(1);
    }

    // Display the results
    printf("Total Earnings of Hotel: %d INR\n", total_earnings);
    printf("Total Wages of Waiters: %d INR\n", total_wages);
    printf("Total Profit: %d INR\n", profit);
}

void write_earnings_to_file(int table_number, int earnings)
{
    FILE *file;

    // Open the file in append mode
    file = fopen("earnings.txt", "a");
    if (file == NULL)
    {
        perror("fopen");
        exit(1);
    }

    // Write the earnings to the file
    fprintf(file, "Earning from Table %d: %d INR\n", table_number, earnings);

    // Close the file
    if (fclose(file) != 0)
    {
        perror("fclose");
        exit(1);
    }
}

int main()
{
    // total number of tables (to be given sequentially)
    int T_N;

    // key and shared memory for admin to communicate when to shutdown
    key_t key1;
    int shmid1;
    int *shmptr1;

    // creaate shared memory to communicate with admin
    if ((key1 = ftok("admin.c", '3')) == -1)
    {
        perror("Error in the ftok function \n");
    }

    shmid1 = shmget(key1, BUFF_SIZE, 0666);
    if (shmid1 == -1)
    {
        perror("error in creating/accessing shared memory with admin\n");
        return 1;
    }

    shmptr1 = shmat(shmid1, NULL, 0);
    if (shmptr1 == (void *)-1)
    {
        perror("error in creating/accessing shared memory\n");
        return 1;
    }

    printf("Enter the total nummber of tables in the hotel\n");
    scanf("%d", &T_N);

    // creation of shared memory to communcate from waiter

    // a 1-D array to maintain keys
    key_t key[T_N];

    // 1d arry that contains buffer to get no of attached processes
    struct shmid_ds buff[T_N];

    for (int i = 0; i < T_N; i++)
    {
        if ((key[i] = ftok("hotel_manager.c", 65 + i + 1)) == -1)
        {
            perror("Error in ftok\n");
            return 1;
        }
    }

    // array of ids and pointers to shared memories
    int shmid[T_N];
    int *shmptr[T_N];

    for (int i = 0; i < T_N; i++)
    {

        shmid[i] = shmget(key[i], BUFF_SIZE, 0666 | IPC_CREAT);
        if (shmid[i] == -1)
        {
            perror("error in creating/accessing shared memory\n");
            return 1;
        }
        shmptr[i] = shmat(shmid[i], NULL, 0);
        if (shmptr[i] == (void *)-1)
        {
            perror("error in pointing to shared memory\n");
            return 1;
        }
    }
    printf("Created all the shared memoreis with waiters\n");

    // after creating shmptrs and ids, next connect to waiter

    while (1)
    {

        // break condition from waiter when thhere are no more customers
        if (shmptr1[0] == 1)
        {
            printf("Imagine this is insane\n");
            break;
        }
        usleep(100);
        for (int i = 0; i < T_N; i++)
        {
            // get status of each shared memory of waiteer
            if (shmctl(shmid[i], IPC_STAT, &buff[i]) == -1)
            {
                perror("shmctl");
                exit(1);
            }
            // check if no of processes attached is euqal to 2, i.e. hotel manager and waiterw
            if (buff[i].shm_nattch == 2)
            {
                // printf("waiter %d has 2 processes\n", i + 1);
                //  check if its ready to read
                if (shmptr[i][0] == 1)
                {
                    printf("%d waiter is ready to read\n", i + 1);
                    fflush(stdout);

                    printf("shmptr[%d][1] has value: %d\n", i, shmptr[i][1]);
                    fflush(stdout);

                    int earn = 0;
                    earn = shmptr[i][1];
                    printf("%d table has earning of %d\n", i + 1, earn);
                    fflush(stdout);
                    write_earnings_to_file(i + 1, earn);
                    // once read reset the shmptr read flag
                    shmptr[i][0] = 0;
                }
            }
        }
    }

    printf("Broke off from the amdin\n");

    // new array to store the buffer
    struct shmid_ds buff1[T_N];
    for (int i = 0; i < T_N; i++)
    {
        if (shmctl(shmid[i], IPC_STAT, &buff1[i]) == -1)
        {
            perror("shmctl");
            exit(1);
        }
        // check if number of attached processes is >= 2 (if yes the waiter is still attached)
        // which means wait till customers leave
        // if(buff1[i].shm_nattch >= 2) {
        //     printf("%d is the waiter with 2 processes\n", i+1);
        // }

        while (1)
        {
            // printf("%d is tthe shmid \n", shmid[i]);
            usleep(100000);
            if (shmctl(shmid[i], IPC_STAT, &buff1[i]) == -1)
            {
                perror("shmctl");
                exit(1);
            }
            // printf("%d\n is the nof  of attched processes\n", buff1[i].shm_nattch);
            if (buff[i].shm_nattch == 2)
            {
                // printf("waiter %d has 2 processes\n", i + 1);
                //  check if its ready to read
                if (shmptr[i][0] == 1)
                {
                    printf("%d waiter is ready to read\n", i + 1);
                    fflush(stdout);

                    printf("shmptr[%d][1] has value: %d\n", i, shmptr[i][1]);
                    fflush(stdout);

                    int earn = 0;
                    earn = shmptr[i][1];
                    printf("%d table has earning of %d\n", i + 1, earn);
                    fflush(stdout);
                    write_earnings_to_file(i + 1, earn);
                    // once read reset the shmptr read flag
                    shmptr[i][0] = 0;
                }
            }
            if (buff1[i].shm_nattch == 1)
            {
                printf("Will break now\n");
                break;
            }
        }

        while (buff1[i].shm_nattch >= 2)
        {
            usleep(100);
        }
        if (buff1[i].shm_nattch == 1)
        {
            printf("%d is the waiter with 1 processes\n", i + 1);
        }
        if (buff1[i].shm_nattch == 1)
        {
            if (shmdt(shmptr[i]) == 0)
            {
                printf("%d waiter has detached and hotel manager also being detached\n", i + 1);
            }
            else
            {
                printf("Error in detaching hotel manager memoryy\n");
            }
        }
        if (shmctl(shmid[i], IPC_RMID, NULL) == -1)
        {
            perror("Error in removing shared memory hotel manager\n");
        }
        else
        {
            printf("Removed shared memory segment hotel manager\n");
        }
    }
    calculate_and_display_financials();
    if (shmdt(shmptr1) == 0)
    {
        printf("hotel mager has detached from manager\n");
    }
    else
    {
        printf("hotel mager has got an error detached from manager\n");
    }

    // cleanup of shared mempry
    if (shmctl(shmid1, IPC_RMID, NULL) == -1)
    {
        printf("hotel mager has got an error deleting from manager to admin\n");
    }
    else
    {
        printf("deleted shared memory segment hotel manager to admin\n");
    }
}