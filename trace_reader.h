#pragma once

#include "device.h"
#include "fiasco/kip.h"
#include "fiasco/ktrace_events.h"
#include "fiasco/pcileechinfo.h"
#include "page_table.h"
#include <leechcore.h>

class TraceReader
{
public:
	TraceReader(int loglevel);
	~TraceReader();
	bool is_record_available();
	l4_tracebuffer_entry_t get_record();

private:
	struct Tracebuffer_status get_status();

	//TODO: use &references
	Device *dev;
	Pcileechinfo *pi;
	PageTable *ptab;

	Address tbuf_start;
	Address tbuf_end;
	Address last_read;
};
