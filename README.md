# TCP Quiz Client–Server

A command-line quiz application written in C using IPv4 TCP sockets. I independently developed the client and server as a university programming assignment.

## Features

* Connect to a server using an IPv4 address and port number.
* Answer five different questions randomly selected from a 43-question bank.
* Receive immediate feedback and the correct answer after an incorrect response.
* View a final score at the end of the quiz.
* Start with `Y` or exit with `q`.
* Serve clients sequentially, keeping the server running between sessions.

## Technologies

C, POSIX sockets, TCP/IP and GCC.

## Files

* `server.c` — accepts connections, selects questions, checks answers and calculates scores.
* `client.c` — connects to the server, displays questions and sends user input.
* `QuizDB.h` — question and answer data.

## Build and Run

Use macOS or Linux with GCC or a compatible C compiler.

Compile the programs:

```bash
gcc -Wall -Wextra server.c -o server
gcc -Wall -Wextra client.c -o client
```

Start the server in one terminal:

```bash
./server 127.0.0.1 5000
```

Start the client in another terminal:

```bash
./client 127.0.0.1 5000
```

Enter `Y` to begin. Answers are case-sensitive. Press `Ctrl+C` in the server terminal to stop the server.

## Implementation

The application uses a newline-delimited text protocol over TCP. Both programs read incoming messages one line at a time. The client loops when sending to handle partial writes.

The server checks that randomly selected questions do not repeat within a quiz and maintains a score for each session.

## Current Limitations

* The server handles one client at a time.
* Submitting an empty answer ends the connection.
* Long-input handling needs improvement; the client currently produces buffer-size warnings when compiled with warnings enabled.
* The server does not yet retry partial writes.

## Author

Weihao Li
University College Dublin
