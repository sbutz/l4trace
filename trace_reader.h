#pragma once

#include "fiasco/fiasco.h"
#include "fiasco/ktrace_events.h"
#include <vector>
#include <fstream>

/* Ignore some warnings from LeechCore */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wwrite-strings"
#include <leechcore.h>
#pragma GCC diagnostic pop

class Device;
class Kip;
class PageTable;
class Pcileechinfo;

class TraceReader
{
public:
	explicit TraceReader(std::string path);
	~TraceReader();
	std::pair<size_t,size_t> get_new_records();
    void write_new_records();

private:
	struct Tracebuffer_status get_status();
	//TODO: create class MappedBuffer<>(dev, start, size)
	// update()
	// update(start, end)
	// get(element)
	size_t update_buffer(Address start, Address end);

	Device *dev;
	Pcileechinfo *pi;
	PageTable *ptab;
	std::vector<l4_tracebuffer_entry_t> buffer;
	std::fstream file;

	Address tbuf_start;
	Address tbuf_end;
	size_t tbuf_size;
	Address last_read;
    Address last_written;
	uint64_t last_num;
};
