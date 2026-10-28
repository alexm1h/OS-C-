#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <unistd.h>

#define FILENAME "shared.dat"
#define FILESIZE 4096

int main() {
    int fd;
    char *ptr;
    int choice;
    fd_set read_fds;
    struct timeval timeout;

    do {
        printf("\n--- CLIENT MENU ---\n");
        printf("1. Open mapping\n");
        printf("2. Read data\n");
        printf("3. Exit\n");
        printf("Choice: ");
        scanf("%d", &choice);
        getchar();

        switch(choice) {
            case 1:
                fd = open(FILENAME, O_RDWR, 0666);
                if (fd == -1) {
                    perror("open");
                    break;
                }

                ptr = mmap(NULL, FILESIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
                if (ptr == MAP_FAILED) {
                    perror("mmap");
                    close(fd);
                    break;
                }
                printf("File mapped at %p\n", ptr);
                break;

            case 2:
                if (ptr == NULL) {
                    printf("Open mapping first!\n");
                    break;
                }

                FD_ZERO(&read_fds);
                FD_SET(fd, &read_fds);
                timeout.tv_sec = 5;
                timeout.tv_usec = 0;
                
                if (select(fd + 1, &read_fds, NULL, NULL, &timeout) > 0) {
                    printf("Data read: %s\n", ptr);
                } else {
                    printf("Timeout or no data\n");
                }
                break;

            case 3:
                if (ptr != NULL && ptr != MAP_FAILED) {
                    munmap(ptr, FILESIZE);
                }
                if (fd != -1) {
                    close(fd);
                }
                printf("Exiting.\n");
                break;
        }
    } while(choice != 3);

    return 0;
}
