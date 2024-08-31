# Hotel Management System

## Overview
This project, titled **Hotel Management System**, is an advanced C-based simulation that implements several key concepts from operating systems such as process creation, inter-process communication (IPC), synchronization, and shared memory management. The project represents a complex simulation of a hotel environment where multiple processes work concurrently to manage tables, customers, waiters, a hotel manager, and an admin. Each process is designed to operate independently, using IPC mechanisms like pipes and shared memory to communicate and coordinate their activities.

This system is a testament to the rigorous application of operating system principles and the careful design and implementation needed to simulate a real-world application with multiple interacting components.

## Project Structure
The system consists of the following key components:

1. **Table Process (`table.c`):**
    - Represents each table in the hotel.
    - Manages customer processes, which are its child processes.
    - Communicates with a corresponding waiter process via shared memory to place orders and receive bills.

2. **Customer Processes:**
    - Child processes of a table, representing individual customers sitting at that table.
    - They select menu items, which are communicated back to the table process.

3. **Waiter Process (`waiter.c`):**
    - Handles orders for a specific table by communicating with the corresponding table process via shared memory.
    - Validates the orders against a predefined menu and calculates the total bill.
    - Sends the bill amount to the hotel manager process and the respective table process.

4. **Hotel Manager Process (`hotel_manager.c`):**
    - Oversees the total operations of the hotel, including managing earnings and overseeing the termination of processes.
    - Receives earnings from each waiter process, calculates total hotel earnings, waiter wages, and hotel profit.
    - Responsible for cleaning up shared memory and ensuring that all processes terminate correctly.

5. **Admin Process (`admin.c`):**
    - Provides administrative control over the hotel, allowing the simulation to be terminated.
    - Signals the hotel manager to begin the termination process when the hotel is to be closed.

## Key Features and Concepts Demonstrated

1. **Process Management:**
    - Creation of multiple independent processes (`table`, `customer`, `waiter`, `hotel_manager`, and `admin`) using `fork()`.
    - Implementation of parent-child process relationships, particularly between tables and customers.

2. **Inter-Process Communication (IPC):**
    - Use of pipes for communication between table and customer processes.
    - Use of shared memory segments for communication between table-waiter pairs and between waiters and the hotel manager.

3. **Synchronization and Coordination:**
    - Ensured proper synchronization between processes using flags and checks within the shared memory.
    - Managed concurrent access to shared resources (e.g., shared memory) without the use of semaphores or mutexes, as per assignment constraints.

4. **Error Handling and Robustness:**
    - Comprehensive error handling for system calls (e.g., `shmget`, `shmat`, `shmdt`, `shmctl`, `pipe`, `fork`).
    - Ensured clean termination of processes and detachment of shared memory segments to prevent resource leaks.

5. **Simulation of Real-World Scenarios:**
    - The project simulates a realistic environment where multiple tables can have customers ordering food, and waiters attending to specific tables independently.
    - It also incorporates a system for managing and calculating the financial aspects of the hotel’s operations, including the total earnings, waiter wages, and profit.

## Execution and Usage

1. **Compilation:**
    - Each C file can be compiled using the following commands:
      ```bash
      gcc -o table table.c
      gcc -o waiter waiter.c
      gcc -o hotel_manager hotel_manager.c
      gcc -o admin admin.c
      ```
    - Ensure that all source files (`table.c`, `waiter.c`, `hotel_manager.c`, and `admin.c`) and the `menu.txt` file are in the same directory before compiling.

2. **Running the Simulation:**
    - Start the simulation by running the hotel_manager and admin processes.
    - In separate terminals, run instances of table and waiter processes corresponding to each table in the hotel.
    - Each table will prompt for the number of customers, and each customer will order from the menu. The corresponding waiter will validate and calculate the bill.
    - The admin can decide when to close the hotel, triggering the hotel manager to compute the final financials and clean up resources.

3. **Termination:**
    - The `admin.c` process allows for a graceful shutdown of the system, ensuring that all processes terminate properly and shared memory is cleaned up.

## Challenges and Complexity
This project is a culmination of multiple advanced topics in operating systems, requiring deep understanding and careful implementation of IPC, process synchronization, and memory management. The complexity involved in ensuring that all processes communicate correctly and terminate properly demonstrates the author’s strong grasp of these concepts.

The time and effort invested in this assignment reflect the commitment to producing a robust, functional, and well-coordinated system that adheres strictly to the constraints and requirements provided.

## Conclusion
The Hotel Management System project is not just a technical achievement but also a demonstration of the author’s ability to translate theoretical concepts into a working application. It showcases proficiency in C programming, operating system concepts, and problem-solving skills. The detailed and intricate nature of this assignment is a testament to the hard work and dedication involved in its completion.
