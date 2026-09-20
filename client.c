#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

//Size of the buffer used to send and receive messages
#define BUFFER_SIZE 1024


#define QUIZ_SIZE 5


//Sends a full message to the server
int send_message(int socket_fd, char message[])
{
    int total_sent = 0;
    int message_length = strlen(message);

    while (total_sent < message_length)
    {
        int n = send(socket_fd,message + total_sent,message_length - total_sent,0);

        //If send() fails
        if (n <= 0)
        {
            return -1;
        }

        total_sent = total_sent + n;
    }

    return 0;
}


//Reads one line from the socket ,stops reading when it sees '\n'
int read_line(int socket_fd, char buffer[])
{
    int i = 0;
    char c;

    while (i < BUFFER_SIZE - 1)
    {
        int n = recv(socket_fd, &c, 1, 0);

        //If recv() fails or the server closes the connection, return
        if (n <= 0)
        {
            return n;
        }

        //Stop reading at the end of the line
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

// This function removes the newline from user input
void remove_newline(char str[])
{
    int length;

    length = strlen(str);

    if (length > 0 && str[length - 1] == '\n')
    {
        str[length - 1] = '\0';
    }
}

int main(int argc, char *argv[])
{
    int client_socket;
    int port_number;
    int i;

    struct sockaddr_in server_address;

    char buffer[BUFFER_SIZE];
    char answer[BUFFER_SIZE];
    char message[BUFFER_SIZE];

   
    // Server IPv4 address and Server port number
    if (argc != 3)
    {
        printf("Usage: %s <IPv4 address> <port number>\n", argv[0]);
        exit(1);
    }

    //Convert the port number from string to integer
    port_number = atoi(argv[2]);

    // Create a TCP socket.
    client_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (client_socket < 0)
    {
        perror("socket");
        exit(1);
    }

    //Clear the server address structure
    memset(&server_address, 0, sizeof(server_address));

    //Fill in the server address details
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port_number);
    server_address.sin_addr.s_addr = inet_addr(argv[1]);

    
    //Connect to the server
    if (connect(client_socket,
                (struct sockaddr *)&server_address,
                sizeof(server_address)) < 0)
    {
        perror("connect");
        close(client_socket);
        exit(1);
    }

    // Read and print welcome message from the server
    for (i = 0; i < 6; i++)
    {
        if (read_line(client_socket, buffer) <= 0)
        {
            printf("Server closed connection.\n");
            close(client_socket);
            exit(1);
        }

        printf("%s\n", buffer);
    }

    //Ask the user to enter Y to start or q to quit
    printf("> ");

    if (fgets(answer, BUFFER_SIZE, stdin) == NULL)
    {
        close(client_socket);
        exit(1);
    }

    //Remove newline from the user input
    remove_newline(answer);

    sprintf(message, "%s\n", answer);

    // Send Y or q to the server
    if (send_message(client_socket, message) < 0)
    {
        perror("send");
        close(client_socket);
        exit(1);
    }

    //If the user enters q, close the connection and quit.
    if (strcmp(answer, "q") == 0)
    {
        close(client_socket);
        return 0;
    }

    
    // If the user does not enter Y, print server message and quit.
    if (strcmp(answer, "Y") != 0)
    {
        if (read_line(client_socket, buffer) > 0)
        {
            printf("%s\n", buffer);
        }

        close(client_socket);
        return 0;
    }

    
    //The quiz starts here
    for (i = 0; i < QUIZ_SIZE; i++)
    {
        // Receive one question from the server
        if (read_line(client_socket, buffer) <= 0)
        {
            printf("Server closed connection.\n");
            close(client_socket);
            exit(1);
        }

        printf("%s\n", buffer);

        // Read the user's answer from the keyboard
        printf("Answer: ");

        if (fgets(answer, BUFFER_SIZE, stdin) == NULL)
        {
            close(client_socket);
            exit(1);
        }

        //Remove newline from the answer
        remove_newline(answer);

        sprintf(message, "%s\n", answer);

        //Send the answer to the server
        if (send_message(client_socket, message) < 0)
        {
            perror("send");
            close(client_socket);
            exit(1);
        }

        //Receive the result from the server.
        if (read_line(client_socket, buffer) <= 0)
        {
            printf("Server closed connection.\n");
            close(client_socket);
            exit(1);
        }

        printf("%s\n", buffer);
    }

    //After five questions, receive and print the final score.
    if (read_line(client_socket, buffer) > 0)
    {
        printf("%s\n", buffer);
    }

    close(client_socket);

    return 0;
}