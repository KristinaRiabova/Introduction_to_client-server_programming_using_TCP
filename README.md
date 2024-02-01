Protocol Overview

This communication protocol facilitates interaction between a client and server for a simple file transfer application over TCP. The protocol defines a set of commands, their formats, and the expected responses.

Request Messages from Client to Server:
GET <filename>:

The client sends a request to the server to retrieve the content of the specified file.
Example: GET file.txt
The server responds with the content of the file or a message indicating that the file was not found.
LIST:

The client requests the server to provide a list of files available for the client.
No additional arguments are needed.
The server responds with a list of filenames or an error message if listing files fails.
PUT <filename>:

The client sends a request to the server to upload a file with the specified filename.
Example: PUT file.txt
The server expects the client to follow up with the file content.
After successful upload, the server responds with a success message.
DELETE <filename>:

The client requests the server to delete the specified file.
Example: DELETE file.txt
The server deletes the file and responds with a success message or an error message if deletion fails.
INFO <filename>:

The client requests information about the specified file, such as its size and last modified time.
Example: INFO file.txt
The server responds with the requested information or an error message if information retrieval fails.
EXIT:

The client requests to terminate the connection and exit the application.
No additional arguments are needed.
The server acknowledges the request and closes the connection gracefully.
Response Messages from Server to Client:
File Content:

Response to a GET request containing the content of the requested file.
The server sends the content of the file or a message indicating that the file was not found.
List of Files:

Response to a LIST request containing a list of files available for the client.
The server sends a list of filenames or an error message if listing files fails.
Success Message:

Response to successful operations like PUT, DELETE, and others.
The server sends a message indicating the success of the operation.
Error Message:

Response to indicate errors occurred during operations.
The server sends an error message with details about the encountered error.
File Not Found:

Response to indicate that the requested file does not exist on the server.
The server informs the client that the requested file is not available.
File Information:

Response to an INFO request containing information about the specified file.
The server provides information such as file size and last modified time or an error message.
Message Format:
Each message is terminated by a newline character ('\n') or any other agreed-upon delimiter.
Messages consist of command strings followed by optional arguments, separated by spaces.
Example Communication:
Client Request: GET file.txt

Server Response: File content of file.txt or "File not found" message.

Client Request: LIST

Server Response: List of available files on the server.

Client Request: PUT new_file.txt

Server Response: Success message or error message.

Client Request: DELETE file.txt

Server Response: Success message or error message.

Client Request: INFO file.txt

Server Response: Information about the specified file.

Client Request: EXIT

Server Response: No response needed. The server will terminate the connection.


![image](https://github.com/KristinaRiabova/Introduction_to_client-server_programming_using_TCP/assets/103763577/e061b847-11d1-4460-9cfe-c8b793a86fcd)
