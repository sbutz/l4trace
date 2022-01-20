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

int main()
{
	signal(SIGINT, signal_callback_handler);

	TraceReader reader("/tmp/l4trace.out", 0x8);
	uint64_t count = 0;

	while (!aborted) {
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
            std::cout << std::dec << "\r count=" << count << std::flush;
		}
	}

    //TODO: convert to ctf

	return 0;
}

