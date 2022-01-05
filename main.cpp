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

	TraceReader reader = TraceReader();
	while (!aborted) {
		usleep(1000*1000);
		//usleep(1000*10);
		std::cout << "Looping..." << std::endl;
	}

	std::cout << "Exiting..." << std::endl;

	return 0;
}
