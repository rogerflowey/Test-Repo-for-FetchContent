#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <liburing.h>
#include <fcntl.h>
#include <unistd.h>

#define QD_DEPTH 4 // Queue depth
#define BLOCK_SZ 512

int main(int argc, char *argv[]) {
    struct io_uring ring;
    struct io_uring_sqe *sqe;
    struct io_uring_cqe *cqe;
    int ret;
    char *buffer;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <filename_to_read>\n", argv[0]);
        return 1;
    }
    const char *filename = argv[1];

    // Initialize io_uring
    ret = io_uring_queue_init(QD_DEPTH, &ring, 0);
    if (ret < 0) {
        perror("io_uring_queue_init failed");
        return 1;
    }
    printf("io_uring initialized successfully.\n");

    // Open file
    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        perror("open failed");
        io_uring_queue_exit(&ring);
        return 1;
    }
    printf("File '%s' opened successfully (fd: %d).\n", filename, fd);

    // Allocate buffer
    buffer = malloc(BLOCK_SZ);
    if (!buffer) {
        perror("malloc failed");
        close(fd);
        io_uring_queue_exit(&ring);
        return 1;
    }

    // Get a submission queue entry (SQE)
    sqe = io_uring_get_sqe(&ring);
    if (!sqe) {
        fprintf(stderr, "Could not get SQE.\n");
        free(buffer);
        close(fd);
        io_uring_queue_exit(&ring);
        return 1;
    }

    // Prepare a read operation
    io_uring_prep_read(sqe, fd, buffer, BLOCK_SZ, 0); // Read BLOCK_SZ bytes from offset 0
    io_uring_sqe_set_data(sqe, (void*)0xDEADBEEF); // User data (optional)

    // Submit the request
    ret = io_uring_submit(&ring);
    if (ret < 0) {
        perror("io_uring_submit failed");
        free(buffer);
        close(fd);
        io_uring_queue_exit(&ring);
        return 1;
    }
    printf("Read request submitted.\n");

    // Wait for completion
    ret = io_uring_wait_cqe(&ring, &cqe);
    if (ret < 0) {
        perror("io_uring_wait_cqe failed");
        free(buffer);
        close(fd);
        io_uring_queue_exit(&ring);
        return 1;
    }

    // Check result
    if (cqe->res < 0) {
        fprintf(stderr, "Read failed: %s\n", strerror(-cqe->res));
    } else {
        printf("Read %d bytes successfully.\n", cqe->res);
        // printf("First few bytes: %.*s\n", (int)cqe->res > 10 ? 10 : (int)cqe->res, buffer);
    }

    // Mark completion queue entry (CQE) as seen
    io_uring_cqe_seen(&ring, cqe);

    // Cleanup
    free(buffer);
    close(fd);
    io_uring_queue_exit(&ring);
    printf("io_uring cleaned up.\n");

    return 0;
}