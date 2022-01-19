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

	TraceReader reader = TraceReader(0x8);
	uint64_t i = 0;
	uint64_t last_num = 0;

	//current:
	//status read (virt_to_phys 3 + status read 1) = 4
	//record read (virt_to_phys 3 + record read 1) = 4
	//8 reads for one record
	//
	//virt_to_phys reduce
	//ideas:
	//- cache status address
	//- calc record address
	//- cache pdir pages (more generic) [DONE]
	//2 reads for one record
	//
	//multiple reads at once
	//- always read whole buffer [DONE]
	//- read required parts (2 reads or 1 scatter read) [DONE]
	while (!aborted) {
		std::pair<size_t,size_t> result = reader.get_new_records();
		if (result.first == 0) {
			usleep(1000*10);
			continue;
		}
		else {
		    if (result.second)
                std::cout << "Lost Events: " << result.second << std::endl;
            i += result.first;
            std::cout << std::dec << "\r i=" << i << std::flush;
		}
		//TODO: write to file
	}

    //TODO: convert to ctf

	return 0;
}

