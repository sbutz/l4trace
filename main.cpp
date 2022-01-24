#include "external_sort.h"
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

bool compareRecords(const l4_tracebuffer_entry_t &a,
                    const l4_tracebuffer_entry_t &b)
{
    if (a._kclock != b._kclock)
        return a._kclock < b._kclock;
    else
        return a._number < b._number;
}

int main()
{
	signal(SIGINT, signal_callback_handler);

	TraceReader reader("/tmp/l4trace.out");
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
		}
        std::cout << std::dec << "\r count=" << count << std::flush;
    }

	ExternalSort<l4_tracebuffer_entry_t>::sort("/tmp/l4trace.out",
                                               "/tmp/l4trace_sorted.out",
                                               compareRecords,
                                               ExternalSort<l4_tracebuffer_entry_t>::MB * 100);
    //TODO: convert to ctf

	return 0;
}

