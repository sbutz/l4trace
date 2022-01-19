#include "device.h"
#include "fiasco/kip.h"
#include "fiasco/pcileechinfo.h"
#include "page_table.h"
#include "trace_reader.h"
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

	/*
	 * Validate read info struct and setup address resolution.
	 */
	if (!this->pi->Tbuf_status_page)
		errx(1, "%s: no Tbuf_status_page found", __func__);

	this->ptab = new PageTable(this->dev, pi->kdir);
	this->dev->add_page_table(this->ptab);

	/*
	 * Read Tracebuffer_status to get its address, size and the current position.
	 */
	struct Tracebuffer_status status = this->get_status();
	this->tbuf_start = status.window[0].tracebuffer;
	this->tbuf_start_phys = this->ptab->virt_to_phys(this->tbuf_start);
	this->tbuf_end = status.window[1].tracebuffer + status.window[1].size;
	this->tbuf_size = this->tbuf_end - this->tbuf_start;
	this->last_read = status.current;
	this->last_num = 0;

	/*
	 * Initialize the internal buffer, the remote tracebuffer will be copied into.
	 */
    size_t n = this->tbuf_size / sizeof(l4_tracebuffer_entry_t);
    this->buffer.resize(n);

	std::cout << "tbuf_start: " << std::hex << tbuf_start << std::endl;
	std::cout << "tbuf_start_phys: " << std::hex << tbuf_start_phys << std::endl;
	std::cout << "tbuf_end: " << std::hex << tbuf_end << std::endl;
	std::cout << "tbuf_size: " << tbuf_size << std::endl;
	std::cout << "num: " << tbuf_size / sizeof(l4_tracebuffer_entry_t) << std::endl;
}

TraceReader::~TraceReader()
{
	delete this->ptab;
	delete this->pi;
	delete this->dev;
}

std::pair<size_t,size_t> TraceReader::get_new_records()
{
    std::pair<size_t,size_t> result(0,0);
	struct Tracebuffer_status status = this->get_status();

	/* No trace records available yet */
	if (status.current == 0)
		return result;

	result.first = this->update_buffer(this->last_read, status.current);

    /* Detect missed records */
    size_t idx_begin = (this->last_read - this->tbuf_start) / sizeof(l4_tracebuffer_entry_t);
    uint64_t next_num = this->buffer[idx_begin]._number;
    if (next_num != this->last_num + 1)
        result.second = next_num - this->last_num;

    this->last_read = status.current;
    size_t idx_end = (this->last_read - this->tbuf_start) / sizeof(l4_tracebuffer_entry_t) - 1;
    this->last_num = this->buffer[idx_end]._number;

    return result;
}

size_t TraceReader::update_buffer(Address start, Address end) {
    if (start < end) {
        /* align addresses */
        start = start & pagemask;
        end = end % pagesize == 0 ? end : (end & pagemask) + pagesize;
        size_t size = end - start;
        size_t idx = (start - this->tbuf_start) / sizeof(l4_tracebuffer_entry_t);

       this->dev->read_virt(start, size, &this->buffer[idx]);
       return size / sizeof(l4_tracebuffer_entry_t);
    } else {
        return this->update_buffer(start, this->tbuf_end) +
            this->update_buffer(this->tbuf_start, end);
    }
}

struct Tracebuffer_status TraceReader::get_status()
{
	struct Tracebuffer_status status = { 0 };

	this->dev->read(
		this->ptab->virt_to_phys(this->pi->Tbuf_status_page),
		sizeof(status),
		&status
	);

	return status;
}
