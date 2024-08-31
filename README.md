Overview
This project, titled Hotel Management System, is an advanced C-based simulation that implements several key concepts from operating systems such as process creation, inter-process communication (IPC), synchronization, and shared memory management. The project represents a complex simulation of a hotel environment where multiple processes work concurrently to manage tables, customers, waiters, a hotel manager, and an admin. Each process is designed to operate independently, using IPC mechanisms like pipes and shared memory to communicate and coordinate their activities.

This system is a testament to the rigorous application of operating system principles and the careful design and implementation needed to simulate a real-world application with multiple interacting components.

1. Table Process (table.c):
    Represents each table in the hotel.
    Manages customer processes, which are its child processes.
    Communicates with a corresponding waiter process via shared memory to place orders and receive bills.

2. Customer Process:
    Child processes of a table, representing individual customers sitting at that table.
    They select menu items, which are communicated back to the table process.

3. Waiter Process (waiter.c):
    Handles orders for a specific table by communicating with the corresponding table process via shared memory.
    Validates the orders against a predefined menu and calculates the total bill.
    Sends the bill amount to the hotel manager process and the respective table process.

4. Hotel Manager Process (hotel_manager.c):
    Oversees the total operations of the hotel, including managing earnings and overseeing the termination of processes.
    Receives earnings from each waiter process, calculates total hotel earnings, waiter wages, and hotel profit.
    Responsible for cleaning up shared memory and ensuring that all processes terminate correctly.

5. Admin Process (admin.c):
    Provides administrative control over the hotel, allowing the simulation to be terminated.
    Signals the hotel manager to begin the termination process when the hotel is to be closed.

