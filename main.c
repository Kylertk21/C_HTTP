#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8080
#define IMAGE_PATH "salmon.jpg"

int main(int argc, char const *arv[]) {

  int server_fd, new_socket;
  long valread;
  struct sockaddr_in address;
  int addrlen = sizeof(address);

  // Creating socket File Descriptor
  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
    perror("in socket");
    exit(EXIT_FAILURE);
  }

  // Binding socket
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(PORT);

  memset(address.sin_zero, '\0', sizeof address.sin_zero); // init padding to 0

  if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) <
      0) { // bind socket address
    perror("in bind");
    exit(EXIT_FAILURE);
  }
  // Listen for new connection
  if (listen(server_fd, 10) < 0) { // listen on socket w/ backlog of 10
    perror("in listen");
    exit(EXIT_FAILURE);
  }
  // Accept incoming connections
  while (1) {
    printf("\n++++++++++ Waiting for Connection ++++++++++++\n\n");
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                             (socklen_t *)&addrlen)) < 0) {
      perror("in accept");
      exit(EXIT_FAILURE);
    }

    char buffer[30000] = {0};
    valread = read(new_socket, buffer, 30000);
    FILE *img = fopen(IMAGE_PATH, "rb"); // Open image at .
    if (!img) {
      // 404 response
      char *not_found = "HTTP/1.1 404 Not Found\r\n\
                           Content-Length: 0\r\n\r\n";
      write(new_socket, not_found, strlen(not_found));
      close(new_socket);
      continue;
    }

    // Get file size
    fseek(img, 0, SEEK_END);
    long img_size = ftell(img);
    rewind(img);

    char header[256];
    int header_len = snprintf(header, sizeof(header),
                              "HTTP/1.1 200 OK\r\n"
                              "Content-Type: image/jpeg\r\n"
                              "Content-Length: %ld\r\n"
                              "\r\n",
                              img_size);

    // Read image into buffer
    unsigned char *img_data = malloc(img_size);
    fread(img_data, 1, img_size, img);
    fclose(img);

    printf("%s\n", buffer);
    if (valread < 0) {
      printf("No bytes to read");
    }

    write(new_socket, header, strlen(header));

    write(new_socket, img_data, img_size);

    free(img_data);
    close(new_socket);
    printf("----------Salmon Image Sent-------------\n");
  }
  return 0;
}
