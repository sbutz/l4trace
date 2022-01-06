#include "trace_reader.h"
#include <cassert>
#include <err.h>
#include <iostream>

TraceReader::TraceReader(int loglevel)
{
	/*
	 * Initialize screamer device.
	 */
	this->dev = new Device("fpga", loglevel);

	/*
	 * Read info struct written by the JDB extension.
	 */
	if (!this->dev->kip->_res5[0])
		errx(1, "no Pcileechinfo found");
	this->pi = new Pcileechinfo();
	this->dev->read(this->dev->kip->_res5[0], sizeof(*this->pi), this->pi);

	if (!this->pi->Tbuf_status_page)
		errx(1, "%s: no Tbuf_status_page found", __func__);

	this->ptab = new PageTable(this->dev, pi->kdir);


	struct Tracebuffer_status status = this->get_status();
	this->tbuf_start = status.window[0].tracebuffer;
	this->tbuf_end = status.window[0].tracebuffer + status.window[0].size
		+ status.window[1].size;
	this->last_read = status.current;

	std::cout << "tbuf_start: " << std::hex << tbuf_start << std::endl;
	std::cout << "tbuf_end: " << std::hex << tbuf_end << std::endl;
	std::cout << "tbuf_size: " << tbuf_end - tbuf_start << std::endl;
	std::cout << "num: " << (tbuf_end - tbuf_start) / sizeof(l4_tracebuffer_entry_t) << std::endl;
}

TraceReader::~TraceReader()
{
	delete this->ptab;
	delete this->pi;
	delete this->dev;
}

uint64_t version = 0;
bool TraceReader::is_record_available()
{
	struct Tracebuffer_status status = this->get_status();

	if (status.window[0].version != version) {
		version = status.window[0].version;
		std::cout << "new version: " << version << std::endl;
	}

	if (status.current == 0) {
		return false;
	}
	else if (this->last_read == 0) {
		// XXX: We never read the first triggered event.
		// XXX: Just leave this for now.
		// TODO: add next_read variable to fix this
		this->last_read = status.current;
		return false;
	}
	else {
		return status.current != this->last_read;
	}
}

//TODO: read multiple records at once (from last read until current, may cross end)
l4_tracebuffer_entry_t TraceReader::get_record()
{
	Address next;
	l4_tracebuffer_entry_t record = { 0 };

	/* The tracebuffer should contain events. */
	assert(this->last_read != 0);

	next = this->last_read + sizeof(l4_tracebuffer_entry_t);
	if (next > this->tbuf_end)
		next = this->tbuf_start;
	this->last_read = next;
	next = this->ptab->virt_to_phys(next);
	this->dev->read(next, sizeof(record), &record);

	return record;
}

struct Tracebuffer_status TraceReader::get_status()
{
	struct Tracebuffer_status status;

	this->dev->read(
		this->ptab->virt_to_phys(this->pi->Tbuf_status_page),
		sizeof(status),
		&status
	);

	return status;
}
