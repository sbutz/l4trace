#pragma once

#include "fiasco/fiasco.h"
#include "fiasco/ktrace_events.h"
#include <leechcore.h>
#include <vector>

class Device;
class Kip;
class PageTable;
class Pcileechinfo;

class TraceReader
{
public:
	explicit TraceReader(int loglevel);
	~TraceReader();
	std::vector<l4_tracebuffer_entry_t> get_new_records();

private:
	struct Tracebuffer_status get_status();

	Device *dev;
	Pcileechinfo *pi;
	PageTable *ptab;

	Address tbuf_start;
	Address tbuf_start_phys;
	Address tbuf_end;
	size_t tbuf_size;
	Address last_read;
	uint64_t last_num;
};
