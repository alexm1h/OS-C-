#define _GNU_SOURCE
#include <iostream>
#include <cstdlib>
#include <aio.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <cstring>
#include <cerrno>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>

using namespace std;

#define MAX_OPS 16

struct aio_operation {
    struct aiocb control_block;
    char *buffer;
    int is_write;
};

aio_operation *free_ops[MAX_OPS];
int free_count = 0;
int ops_in_progress = 0;
off_t next_read_offset = 0;
off_t file_size;
int input_fd, output_fd;
int all_reads_submitted = 0;
int all_writes_completed = 0;
int block_size = 4096;
int total_blocks = 0;
int writes_completed = 0;

void start_new_read();
void start_write(aio_operation *read_op);

void aio_completion_handler(sigval_t sigval) {
    aio_operation *op = (aio_operation *)sigval.sival_ptr;
    int ret = aio_return(&op->control_block);
    
    if (op->is_write) {
        free(op->buffer);
        free_ops[free_count++] = op;
        ops_in_progress--;
        writes_completed++;
        
        if (writes_completed == total_blocks) {
            all_writes_completed = 1;
        }
        start_new_read();
    } else {
        if (ret > 0) {
            start_write(op);
        } else {
            free(op->buffer);
            free_ops[free_count++] = op;
            ops_in_progress--;
        }
    }
}

void start_new_read() {
    if (free_count == 0 || next_read_offset >= file_size) {
        if (next_read_offset >= file_size) {
            all_reads_submitted = 1;
        }
        return;
    }

    aio_operation *read_op = free_ops[--free_count];
    read_op->is_write = 0;

    size_t bytes_to_read = (next_read_offset + block_size <= file_size) ? 
                           block_size : file_size - next_read_offset;

    read_op->buffer = new char[block_size];

    memset(&read_op->control_block, 0, sizeof(struct aiocb));
    read_op->control_block.aio_fildes = input_fd;
    read_op->control_block.aio_buf = read_op->buffer;
    read_op->control_block.aio_nbytes = bytes_to_read;
    read_op->control_block.aio_offset = next_read_offset;
    
    read_op->control_block.aio_sigevent.sigev_notify = SIGEV_THREAD;
    read_op->control_block.aio_sigevent.sigev_notify_function = aio_completion_handler;
    read_op->control_block.aio_sigevent.sigev_value.sival_ptr = read_op;

    if (aio_read(&read_op->control_block) == -1) {
        delete[] read_op->buffer;
        free_ops[free_count++] = read_op;
        return;
    }
    
    next_read_offset += bytes_to_read;
    ops_in_progress++;
}

void start_write(aio_operation *read_op) {
    read_op->is_write = 1;

    size_t nbytes = read_op->control_block.aio_nbytes;
    off_t offset = read_op->control_block.aio_offset;

    memset(&read_op->control_block, 0, sizeof(struct aiocb));
    read_op->control_block.aio_fildes = output_fd;
    read_op->control_block.aio_buf = read_op->buffer;
    read_op->control_block.aio_nbytes = nbytes;
    read_op->control_block.aio_offset = offset;
    
    read_op->control_block.aio_sigevent.sigev_notify = SIGEV_THREAD;
    read_op->control_block.aio_sigevent.sigev_notify_function = aio_completion_handler;
    read_op->control_block.aio_sigevent.sigev_value.sival_ptr = read_op;
    
    if (aio_write(&read_op->control_block) == -1) {
        delete[] read_op->buffer;
        free_ops[free_count++] = read_op;
        ops_in_progress--;
        return;
    }
    
    ops_in_progress++;
}

int main() {
    const char *src_path = "source.txt";
    const char *dst_path = "dest.txt";
    struct stat file_stat;
    struct timeval start, end;

    input_fd = open(src_path, O_RDONLY);
    if (input_fd == -1) {
        perror("Failed to open source file");
        return 1;
    }

    if (fstat(input_fd, &file_stat) == -1) {
        perror("Failed to get file info");
        close(input_fd);
        return 1;
    }
    file_size = file_stat.st_size;
    total_blocks = (file_size + block_size - 1) / block_size;
    
    cout << "Source file size: " << file_size << " bytes" << endl;
    cout << "Block size: " << block_size << " bytes" << endl;
    cout << "Total blocks: " << total_blocks << endl;
    cout << "Max concurrent operations: " << MAX_OPS << endl;

    output_fd = open(dst_path, O_WRONLY | O_CREAT | O_TRUNC, 0664);
    if (output_fd == -1) {
        perror("Failed to create destination file");
        close(input_fd);
        return 1;
    }

    for (int i = 0; i < MAX_OPS; i++) {
        free_ops[i] = new aio_operation;
        free_ops[i]->buffer = nullptr;
        free_count++;
    }

    gettimeofday(&start, NULL);

    for (int i = 0; i < MAX_OPS && next_read_offset < file_size; i++) {
        start_new_read();
    }

    while (!all_writes_completed) {
        usleep(100000);
    }

    gettimeofday(&end, NULL);
    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
    double speed = (file_size / 1024.0 / 1024.0) / elapsed;

    cout << "\n=== Copy completed ===" << endl;
    cout << "Time: " << elapsed << " seconds" << endl;
    cout << "Speed: " << speed << " MB/s" << endl;
    
    for (int i = 0; i < MAX_OPS; i++) {
        delete free_ops[i];
    }

    close(input_fd);
    close(output_fd);
    
    cout << "\nVerifying copy with diff command:" << endl;
    int diff_result = system("diff source.txt dest.txt > /dev/null 2>&1");
    if (diff_result == 0) {
        cout << "✓ Files are identical" << endl;
    } else {
        cout << "✗ Files are different" << endl;
    }
    
    return 0;
}