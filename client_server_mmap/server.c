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
    char buffer[256];
    fd_set read_fds;
    struct timeval timeout;

    do {
        printf("\n--- SERVER MENU ---\n");
        printf("1. Create mapping\n");
        printf("2. Write data\n");
        printf("3. Exit\n");
        printf("Choice: ");
        scanf("%d", &choice);
        getchar();

        switch(choice) {
            case 1:
                fd = open(FILENAME, O_RDWR | O_CREAT, 0666);
                if (fd == -1) {
                    perror("open");
                    break;
                }

                ftruncate(fd, FILESIZE);
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
                    printf("Create mapping first!\n");
                    break;
                }
                printf("Enter text: ");
                fgets(buffer, 256, stdin);
                buffer[strcspn(buffer, "\n")] = 0;
                sprintf(ptr, "%s", buffer);
                printf("Written: %s\n", buffer);

                FD_ZERO(&read_fds);
                FD_SET(fd, &read_fds);
                timeout.tv_sec = 5;
                timeout.tv_usec = 0;
                select(fd + 1, &read_fds, NULL, NULL, &timeout);
                break;

            case 3:
                if (ptr != NULL && ptr != MAP_FAILED) {
                    munmap(ptr, FILESIZE);
                }
                if (fd != -1) {
                    close(fd);
                    unlink(FILENAME);
                }
                printf("Exiting.\n");
                break;
        }
    } while(choice != 3);

    return 0;
}
