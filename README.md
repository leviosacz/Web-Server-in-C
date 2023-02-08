# HTTP Web Server



## Overview

This is a simple web server application using C. The server supports the HTTP 1.0 protocol. It is capable of supporting HTML, TXT, JPG, GIF, JavaScript, CSS, and PNG files. It supports the 200, 404, 403 and 400 HTTP status codes. In addition to status codes, it also supports the Content-Type, Content-Length, and Date headers in responses.

The program runs on an infinite loop where the server accepts client connections from a user supplied port. For every connection it accepts, it creates a thread to handle the request, i.e. multithreading. In the request thread, the server parses the http request, and logs it. It checks if the request is well formed and sends an error otherwise. It then checks if the file exists and if permissions are set properly, and returns an error otherwise. Afterwards, it transmit the file and closes the connection.



## File List

- httpServer.c

  The c source code for this application.

- Makefile

  Makefile for this project.

- document_root

  a folder containing html, css, js and jpg files. It contains the necessary html files to load the www.scu.edu homepage and handle errors.

  

## How to Run the Program

1. Run the command `make` in the terminal to create the executable “httpServer”.

2. Type the command

   `./httpServer -document_root document_root -port 8888`

   8888 can be replaced with any valid port number. Note that ports between 8000 and 9999 are recommended for testing purposes.



## Screenshots

- Create the executable "httpServer" and run the server.
  ![start_server](https://user-images.githubusercontent.com/87473423/217447461-88709b56-182f-4483-aba3-9c233c36da25.png)

- A web browser accessing the web server through localhost:8888.
  ![browser1](https://user-images.githubusercontent.com/87473423/217447552-4609409a-a67d-475c-b24e-9e078bbd14c2.png)
  ![browser2](https://user-images.githubusercontent.com/87473423/217447573-d8f06386-5523-46b7-94d8-ee6bf24f132c.png)
  
- The web server accepting the user's request.
  ![server_get_requests](https://user-images.githubusercontent.com/87473423/217447595-f3ae323b-a1a8-4afe-b54b-dd82475c560f.png)
