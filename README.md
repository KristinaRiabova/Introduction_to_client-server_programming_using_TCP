Protocol Overview

This communication protocol facilitates interaction between a client and server for a simple file transfer application over TCP. The protocol defines a set of commands, their formats, and the expected responses.

Message Format


The command initiated by the client or server (e.g., GET, LIST, PUT, DELETE, INFO, EXIT).
 An optional argument associated with certain commands (e.g., filename).

Message Size

The recommended maximum message size is 1024 bytes, including the command and argument. This ensures efficient communication and helps prevent fragmentation issues.

Commands

GET <filename>

Client to Server: Requests a specific file from the server.
Server to Client: Sends the requested file content.

LIST

Client to Server: Requests a list of available files on the server.
Server to Client: Sends a list of files in the server directory.

PUT <filename>

Client to Server: Uploads a file to the server.
Server to Client: Sends a confirmation message.

DELETE <filename>

Client to Server: Requests the server to delete a specific file.
Server to Client: Sends a confirmation or an error message.

INFO <filename>

Client to Server: Requests information about a specific file (e.g., size, last modified).
Server to Client: Sends information about the requested file.

EXIT

Client to Server: Requests to terminate the connection gracefully.
Server to Client: Acknowledges the request and disconnects the client.

Error Handling
If a command is not recognized or an error occurs during command processing, the server responds with an error message.
