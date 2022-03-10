#include "trace_reader.h"
#include <iostream>
#include <csignal>
#include <unistd.h>

bool aborted = false;
void signal_callback_handler(int signum) {
	if (!aborted)
		aborted = true;
	else
		exit(signum);
}

int main(int argc, char *argv[])
{
	/*
	 * Parameter: output file
	 */
	if (argc != 2) {
		std::cerr << "Usage: " << argv[0] << " FILE" << std::endl;
		return 1;
	}

	/*
	 * Run until interrupted.
	 */
	signal(SIGINT, signal_callback_handler);


	/*
	 * Init the reader.
	 */
	TraceReader reader(argv[1]);

	uint64_t count = 0;
	while (!aborted) {
		/*
		 * Check for new entries in the tracebuffer.
		 */
		std::pair<size_t,size_t> result = reader.get_new_records();
		if (result.first == 0) {
			usleep(1000*10);
			continue;
		}
		else {
		    reader.write_new_records();
		    if (result.second)
                std::cout << "Lost Events: " << result.second << std::endl;
            count += result.first;
		}

		/*
		 * Feedback for the user.
		 */
        std::cout << std::dec << "\r count=" << count << std::flush;
    }

	return 0;
}
