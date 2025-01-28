#include <stdio.h>
#include "request.h"
#include "io_helper.h"

//
// TODO: Implement a more sophisticated client that can send simultaneous
// requests at a certain frequency to allow testing of multi-threaded webserver
//
// Example: Create a function that will send 100k requests randomly every 10 sec
// Verify the responses returned to what's expected
// Verify and compare the time it takes to the single-threaded webserver
//
//
// TODO: Implement the handling of program arguments such that it will handle this:
// prompt> ./wserver [-d basedir] [-p port] [-t threads] [-b buffers] [-s schedalg]
// refer to the README.md for more detailed explanation of the expected arguments
//
// TODO: Probably create a separate module called thread and send accepted connections
// there. Thread will do the necessary work to spawn worker threads and synchornization
// to handle the accepted connections concurrently.
// We should probably implement a thread pool that accepts work submissions.
// A work should have a struct describing it's details that can be submitted to the pool.
// The pool should first just implement a basic queue but later on a scheduling mechanism
// is needed to pop from the queue the right work.
// A simple get or pop API can be implemented which will give the proper work to the query.
// FIFO is already the queue itself, so implement another module queue.c or something.
// SFF is about the file sizes, so implement a way that it will be preprocessed about size
// then will enqueue it to the jobs list, also make sure to not starve large files, so
// probably extend the algorithm with some multiplier as to for how long they have been
// in the queue or some mechanism to randomly select every now and then
//
// TODO: Implement security measures so that any other file up the directory provided is not
// accessible.
//
// TODO: Implement caching to serve files that are frequently requested faster and cache
// replacement algorithms of some sort to support robustness.
//
// TODO: Add testing of different flavors. Also run tests in sanitizer builds. Unit tests,
// integration tests, functional tests (end to end), perhaps stress tests.
//

char default_root[] = ".";
static int listen_fd = -1;

// signal handler to close the listening socket gracefully on shutdown
void cleanup(int signal) {
    (void) signal;
    if (listen_fd != -1) {
        close(listen_fd);
        printf("socket closed gracefully\n");
    }
    exit(0);
}

// ./wserver [-d <basedir>] [-p <portnum>]
int main(int argc, char *argv[]) {
    char *root_dir = default_root;
    int port = 0;

    int c;
    while ((c = getopt(argc, argv, "d:p:")) != -1) {
		switch (c) {
			case 'd':
				root_dir = optarg;
				break;
			case 'p':
				port = atoi(optarg);
				break;
			default:
				fprintf(stderr, "usage: wserver [-d basedir] [-p port]\n");
				exit(1);
		}
	}

    // run out of this directory
    chdir_or_die(root_dir);

	// log current working directory
	char cwd[MAXBUF];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("current working directory: %s\n", cwd);
    } else {
        fprintf(stderr, "getcwd() error\n");
        exit(1);
    }

	// register signal handlers
    signal(SIGINT, cleanup);  // ctrl + c
    signal(SIGTSTP, cleanup); // ctrl + z
    signal(SIGTERM, cleanup); // kill

    // now, get to work
    listen_fd = open_listen_fd_or_die(port);
    while (1) {
		struct sockaddr_in client_addr;
		int client_len = sizeof(client_addr);
		int conn_fd = accept_or_die(listen_fd, (sockaddr_t *) &client_addr, (socklen_t *) &client_len);
		request_handle(conn_fd);
		close_or_die(conn_fd);
    }
    return 0;
}
