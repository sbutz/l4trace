#include "fiasco/ktrace_events.h"
#include "trace_reader.h"
#include <iostream>
#include <signal.h>
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

	/*
	 * High level:
	 *
	 * while not ctr-c:
	 * 	if (buf available)
	 * 		read next buf
	 * 		write to file
	 * 	sleep
	 *
	 * if not raw
	 * 	convert to ctf
	 */

	TraceReader reader = TraceReader(0x8);
	uint64_t i = 0;
	uint64_t last_num = 0;
	while (!aborted) {
		//std::cout << "Looping..." << std::endl;

		if (!reader.is_record_available()) {
			//usleep(1000*10);
			usleep(1000*10);
			continue;
		}
		//std::cout << "Records available..." << std::endl;

		l4_tracebuffer_entry_t record = reader.get_record();
		//std::cout << "number: " << record._number << std::endl;

		//TODO: catch lost events
		if (last_num != 0 && record._number != last_num+1)
			std::cout << "Lost " << record._number - last_num
				<< " events" << std::endl;
		last_num = record._number;


		i++;
		std::cout << "\r i=" << i << std::flush;
	}
	std::cout << std::endl;

	std::cout << "Exiting..." << std::endl;

	return 0;
}

