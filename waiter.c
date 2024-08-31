#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/ipc.h>

#define BUFF_SIZE 100 // shared memory for hotel manager
#define SHM_SIZE 2048 // Size of the shared memory segment for table.c
#define READY_FLAG 0  // Indicates if data is ready to be read 1 means it ready to be read, 0 means it is ready to write
#define VALID 1
#define MAX_MENU_ITEMS 100
#define TOTAL_INDEX 2
#define C 3 // write size of customers by parent
#define CON 4 //alwayts checks to see if the customers are continuing
int ARRAY_INDEX = 5; // Starting index of the array in shared memory

int main()
{
    int Price = 0;
    int C_N, size, found, count = 0;
    FILE *file = fopen("menu.txt", "r");
    if (file == NULL)
    {
        printf("Error opening file.\n");
        return 1;
    }

    int menu[MAX_MENU_ITEMS], prices[MAX_MENU_ITEMS], menuSize = 0;
    char line[256];

    while (fgets(line, sizeof(line), file) != NULL && menuSize < MAX_MENU_ITEMS)
    {
        sscanf(line, "%d. %*[^0-9] %d", &menu[menuSize], &prices[menuSize]);
        menuSize++;
    }
    fclose(file);

    int W_N, array[MAX_MENU_ITEMS];
    printf("Enter Waiter ID: \n");
    scanf("%d", &W_N);

    // attach to shared memory for individual table
    int *shmptr;
    int shmid;
    key_t key;

    if ((key = ftok("table.c", W_N)) == -1)
    {
        perror("Error in ftok\n");
        return 1;
    }

    // find and attach to shared memory
    while (1)
    {
        shmid = shmget(key, SHM_SIZE, 0666);
        if (shmid < 0)
        {
            printf("Retrying to contact waiter, please wait");
            usleep(100000);
            continue;
        }

        // attach to shared memory
        shmptr = shmat(shmid, NULL, 0);
        if (shmptr < 0)
        {
            printf("Retrying to contact waiter, please wait");
            usleep(100000);
            continue;
        }
        break;
    }
    printf("Successfully attached to table\n");
    printf("hello");

    // attach to shared memory of hotel manager
    int *shmptr1;
    int shmid1;
    key_t key1;

    if ((key1 = ftok("hotel_manager.c", 65 + W_N)) == -1) {
            perror("Error in ftok\n");
            return 1;
    }
    while (1)
    {
        shmid1 = shmget(key1, BUFF_SIZE, 0666);
        printf("Retrying to contact hotel manager, please wait");
        if (shmid1 < 0)
        {  
            usleep(100000);
            continue;
        }

        // attach to shared memory
        shmptr1 = shmat(shmid1, NULL, 0);
        printf("Retrying to contact hotel manager, please wait");
        if (shmptr1 < 0)
        {
            usleep(100000);
            continue;
        }
        break;
    }
    printf("Successfully contacted hotel manager\n ");

    // sum up the arrays individually
    

    while (1)
    {
        int arr[2048], c_a=0, totalPrice=0; // c_a stand to count the array indexes to be put in the array;

        // check if size of customers are  written (read 2)
        while(shmptr[C] == 0) {
            usleep(100);
        }

        while (shmptr[CON] ==0)
        {
            usleep(100);
        }
        if (shmptr[CON] == 2) {
            break;
        }

        //check if data is ready to be read, remembere if its 1 its ready to be read (read 1)
        while (shmptr[READY_FLAG] == 0)
        {
            usleep(100);
        }
        
        for (int j = 0; j<shmptr[C]; j++) {
            // read the size
            size = shmptr[ARRAY_INDEX];
            printf("size is %d\n", size);

            //read the array
            for (int i = 0; i < size; i++)
            {
                // read3

                printf("Read from shared memory %d\n\n", shmptr[ARRAY_INDEX + i + 1]);
                arr[c_a] = shmptr[ARRAY_INDEX + i + 1];
                c_a += 1;
            }
            ARRAY_INDEX = ARRAY_INDEX + size + 1;

        }
        
        // checking if the order given is valid
        int allFound = 1;
        for (int i = 0; i < c_a; i++)
        {
            found = 0;
            for (int j = 0; j < menuSize; j++)
            {
                if (arr[i] == menu[j])
                {
                    found = 1;
                    totalPrice += prices[j];
                    printf("the total price at each point is %d\n", totalPrice);
                    break;
                }
            }
            if (found)
            {
                printf("%d is present in the menu\n", arr[i]);
            }
            else
            {
                allFound = 0;
                printf("%d is not present in the menu, hence need to reorder\n", arr[i]);
                break;
            }
        }

        //check if allfound is right or wrong
        if(allFound == 0) {

            // write 1 - write if its valid
            shmptr[VALID] = 1;

            c_a = 0;
            
            ARRAY_INDEX = 5;
        }

        else {
            // write 3 - write total Price

            printf("Before writing into the total Price%d\n", totalPrice);
            shmptr[TOTAL_INDEX] = totalPrice;

            printf("plase wait the waiter is informing the hotel manager about your bill\n");

            while (shmptr1[0] == 1) {
                usleep(100);
            }

            // write earning into the shared memory first and then set the read flag
            shmptr1[1] = totalPrice;
            shmptr1[0] = 1;

            printf("Written the data into memory for hotel manager\n");
            // write 2 - write if ready_flag is ready
            shmptr[READY_FLAG] = 0;

            ARRAY_INDEX = 5;

            c_a = 0;
            // write1 - write if its valid
            shmptr[VALID] = 2;

        }
        shmptr[CON] = 0;
    }
    struct shmid_ds h1;
    shmctl(shmid, IPC_STAT, &h1);

    if (shmdt(shmptr) == 0) {
        printf("Succesfully the waiter has detached \n");
    }
    else {
        printf("watier Not detached from shared memory\n");   
    }

    printf("%d is the no of processes attached to table shared memory and its shmid is %d\n", h1.shm_nattch, shmid);

    
    if (shmdt(shmptr1) == 0) {
        printf("Succesfully the waiter has detached from hotel manager \n");
    }
    else {
        printf("Waiter has not detached from hotel manager\n");
    }

    struct shmid_ds h;
    shmctl(shmid1, IPC_STAT, &h);
    printf("%d is the no of processes attached for hotel manger shared memry and is shmid is %d\n", h.shm_nattch, shmid1);
    
}