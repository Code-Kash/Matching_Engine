# Simple Cross Order Management System

The Simple Cross Order Management System is a program that processes order actions and performs order matching and execution based on specified rules. It maintains order books for different symbols and allows buying and selling orders to be matched and executed.

## Usage

To use the Simple Cross Order Management System, follow these steps:

1. Compile the code using a C++ compiler, such as g++.
2. Execute the compiled binary file.

Upon execution, the program reads a file named "actions.txt" located in the same directory as the executable. This file contains a sequence of order actions, each on a separate line. The program processes these actions and produces the corresponding results.

## Order Actions

The order actions in the "actions.txt" file should follow a specific format. Each line represents an order action and consists of space-separated values. The format for each type of action is as follows:

- Add Order:
  - Format: O \<oid> \<symbol> \<side> \<qty> \<px>
  - Example: O 10000 IBM B 10 100
  - Description: Adds a new order with the specified order ID (oid), symbol, side (B for Buy or S for Sell), quantity (qty), and price (px).

- Cancel Order:
  - Format: X \<oid>
  - Example: X 10000
  - Description: Cancels the order with the specified order ID (oid).

- Print Orders:
  - Format: P
  - Example: P
  - Description: Prints the existing orders in the order books.

## Results

After processing each order action, the program generates results based on the action's outcome. The results are displayed in the console. The format of the result messages is as follows:

- Fill:
  - Format: F \<oid> \<symbol> \<qty> \<px>
  - Example: F 10000 IBM 5 100
  - Description: Indicates that an order (oid) has been filled for a specified symbol, with the filled quantity (qty) at the given price (px).

- Error:
  - Format: E \<oid> \<error_message>
  - Example: E 10000 Invalid side
  - Description: Indicates that an error occurred for the specified order (oid) with the corresponding error message (error_message).

## File Structure

The codebase for the Simple Cross Order Management System is structured as follows:

- `main.cpp`: Contains the main entry point of the program. It reads the order actions from the "actions.txt" file and processes them using the SimpleCross class.

- `simple_cross.h` and `simple_cross.cpp`: Define and implement the SimpleCross class, which represents the core logic for order management and matching. It maintains the order books, processes order actions, and generates results.

- `order.h` and `order.cpp`: Define and implement the Order struct, which represents an individual order with its properties.

- `book_builder.h` and `book_builder.cpp`: Define and implement the BookBuilder class, which handles adding and removing orders from the order books.

- `CMakeLists.txt`: CMake configuration file for building the project.

## Dependencies

The Simple Cross Order Management System has the following dependencies:

- C++ compiler that supports C++17 or later.
- CMake build system (version 3.0 or later).

## Building the Project

To build the Simple Cross Order Management System, follow these steps:

1. Make sure you have a C++ compiler and CMake installed on your system.
2. Clone the project repository to your local machine.
3. Open a terminal or command

