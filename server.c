#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "QuizDB.h"

#define BUFFER_SIZE 1024

#define QUIZ_SIZE 5

// Sends a message to the client
void send_message(int socket_fd, char message[])
{
    // Send the message through the socket
    send(socket_fd, message, strlen(message), 0);
}

//Reads one line from the client
int read_line(int socket_fd, char buffer[])
{
    int i = 0;
    char c;

    // Read one character at a time until newline is found
    while (i < BUFFER_SIZE - 1)
    {
        int n = recv(socket_fd, &c, 1, 0);

        // If recv() fails or client closes connection
        if (n <= 0)
        {
            return n;
        }

        // Stop reading when newline is found
        if (c == '\n')
        {
            break;
        }

        if (c != '\r')
        {
            buffer[i] = c;
            i++;
        }
    }

    buffer[i] = '\0';

    return i;
}

//Checks if a question was already selected
int already_selected(int selected[], int count, int number)
{
    int i;

    for (i = 0; i < count; i++)
    {
        // If the number is already selected
        if (selected[i] == number)
        {
            return 1;
        }
    }

    return 0;
}

//Randomly chooses five different questions
void choose_questions(int selected[])
{
    int total_questions;
    int count;
    int random_number;

    // Find how many questions are in QuizQ
    total_questions = sizeof(QuizQ) / sizeof(QuizQ[0]);

    count = 0;

    while (count < QUIZ_SIZE)
    {
        // Pick a random question number
        random_number = rand() % total_questions;

        // Add the question only if it was not already selected
        if (already_selected(selected, count, random_number) == 0)
        {
            selected[count] = random_number;
            count++;
        }
    }
}

// This function handles the full quiz for one client.
void run_quiz(int client_socket)
{
    char buffer[BUFFER_SIZE];
    char message[BUFFER_SIZE];

    int selected[QUIZ_SIZE];
    int score = 0;
    int i;
    int question_number;

    // Send the quiz welcome message to the client.
    send_message(client_socket, "Welcome to Unix Programming Quiz!\n");
    send_message(client_socket, "The quiz comprises five questions posed to you one after the other.\n");
    send_message(client_socket, "You have only one attempt to answer a question.\n");
    send_message(client_socket, "Your final score will be sent to you after conclusion of the quiz.\n");
    send_message(client_socket, "To start the quiz, press Y and <enter>.\n");
    send_message(client_socket, "To quit the quiz, press q and <enter>.\n");

    // Read the client's first choice: Y or q
    if (read_line(client_socket, buffer) <= 0)
    {
        return;
    }

    // If the client sends q, end the connection
    if (strcmp(buffer, "q") == 0)
    {
        return;
    }
    // If the client does not send Y, quit the quiz
    if (strcmp(buffer, "Y") != 0)
    {
        send_message(client_socket, "Invalid input. Goodbye!\n");
        return;
    }

    // Randomly choose five questions
    choose_questions(selected);

    for (i = 0; i < QUIZ_SIZE; i++)
    {
        question_number = selected[i];

        sprintf(message, "Question %d: %s\n", i + 1, QuizQ[question_number]);

        send_message(client_socket, message);

        // Read the client's answer.
        if (read_line(client_socket, buffer) <= 0)
        {
            return;
        }

        // Check if the answer is correct.
        if (strcmp(buffer, QuizA[question_number]) == 0)
        {
            score++;
            send_message(client_socket, "Right Answer.\n");
        }
        else
        {
            // Tell the client the correct answer.
            sprintf(message,
                    "Wrong Answer. Right answer is %s.\n",
                    QuizA[question_number]);

            send_message(client_socket, message);
        }
    }

    // Send the final score to the client.
    sprintf(message, "Your quiz score is %d/5. Goodbye!\n", score);
    send_message(client_socket, message);
}

int main(int argc, char *argv[])
{
    int server_socket;
    int client_socket;
    int port_number;
    int option = 1;

    struct sockaddr_in server_address;
    struct sockaddr_in client_address;

    socklen_t client_length;

    // 1. IPv4 address
    // 2. Port number
    if (argc != 3)
    {
        printf("Usage: %s <IPv4 address> <port number>\n", argv[0]);
        exit(1);
    }

    // Start the random number generator.
    srand(time(NULL));

    // Convert port number from string to integer.
    port_number = atoi(argv[2]);

    // Create a TCP socket.
    server_socket = socket(AF_INET, SOCK_STREAM, 0);

    // Check if socket creation failed.
    if (server_socket < 0)
    {
        perror("socket");
        exit(1);
    }

    // Allow the port to be reused after the server stops.
    setsockopt(server_socket,
               SOL_SOCKET,
               SO_REUSEADDR,
               &option,
               sizeof(option));

    memset(&server_address, 0, sizeof(server_address));

    // Fill in the server address information.
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port_number);
    server_address.sin_addr.s_addr = inet_addr(argv[1]);

    // Bind the socket to the IP address and port number.
    if (bind(server_socket,
             (struct sockaddr *)&server_address,
             sizeof(server_address)) < 0)
    {
        perror("bind");
        close(server_socket);
        exit(1);
    }

    // Put the socket in listening mode.
    if (listen(server_socket, 5) < 0)
    {
        perror("listen");
        close(server_socket);
        exit(1);
    }

    // Print the required server messages.
    printf("<Listening on %s:%d>\n", argv[1], port_number);
    printf("<Press ctrl-C to terminate>\n");

    while (1)
    {
        client_length = sizeof(client_address);

        // Wait for a client to connect.
        client_socket = accept(server_socket,(struct sockaddr *)&client_address,&client_length);

        if (client_socket < 0)
        {
            perror("accept");
            continue;
        }

        // Run the quiz for this client.
        run_quiz(client_socket);

        // Close this client connection after the quiz ends.
        close(client_socket);
    }

    // Close the server socket.
    close(server_socket);

    return 0;
}