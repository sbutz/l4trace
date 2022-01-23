#include "device.h"
#include "fiasco/kip.h"
#include "fiasco/pcileechinfo.h"
#include "page_table.h"
#include "trace_reader.h"
#include <err.h>
#include <iostream>
#include <cassert>

TraceReader::TraceReader(std::string path)
{
	/*
	 * Initialize screamer device.
	 *
	 * Algorithm:
	 * 0,1  windows only
	 * 2 	old normal read
	 * 3 	old tiny read       <- best
	 *
	 * Loglevel: heavy performance penalty if greater zero
	 */
	this->dev = new Device("FPGA://algo=2", 0x0);

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
	this->tbuf_end = status.window[1].tracebuffer + status.window[1].size;
	this->tbuf_size = this->tbuf_end - this->tbuf_start;
	this->last_read = this->tbuf_start;
    this->last_written = this->last_read;
	this->last_num = 0;

	/*
	 * Initialize the internal buffer, the remote tracebuffer will be copied into.
	 */
    size_t n = this->tbuf_size / sizeof(l4_tracebuffer_entry_t);
    this->buffer.resize(n);

    /*
     * Open output file.
     */
    this->file.open(path, std::ios::out | std::ios::binary);

	std::cout << "tbuf_start: " << std::hex << tbuf_start << std::endl;
	std::cout << "tbuf_end: " << std::hex << tbuf_end << std::endl;
	std::cout << "tbuf_size: " << tbuf_size << std::endl;
	std::cout << "num: " << tbuf_size / sizeof(l4_tracebuffer_entry_t) << std::endl;
}

TraceReader::~TraceReader()
{
    this->file.close();
	delete this->ptab;
	delete this->pi;
	delete this->dev;
}

std::pair<size_t,size_t> TraceReader::get_new_records()
{
    /* Need to call write_new_records() to not loose records */
    assert(this->last_written == this->last_read);

    std::pair<size_t,size_t> result(0,0);
	struct Tracebuffer_status status = this->get_status();

	/* No trace records available yet */
	if (status.current == 0)
		return result;

	Address start = this->last_read;
    Address end = this->last_read < status.current ? status.current : this->tbuf_end;
    result.first = this->update_buffer(start, end);

    /* Detect missed records */
    uint64_t next_num = this->buffer[address_to_idx(start)]._number;
    if (next_num != this->last_num + 1)
        result.second = next_num - this->last_num;

    this->last_read = end;
    this->last_num = this->buffer[address_to_idx(end) - 1]._number;

    return result;
}

void TraceReader::write_new_records() {
    assert(this->last_written < this->last_read);

    size_t last_idx = address_to_idx(this->last_written);
    size_t current_idx = address_to_idx(this->last_read);

    this->file.write((const char *) &this->buffer[last_idx], (current_idx-last_idx) * sizeof(l4_tracebuffer_entry_t));

    /* Go back to buffer start */
    if (this->last_read == this->tbuf_end)
        this->last_read = this->tbuf_start;

    this->last_written = this->last_read;
}

size_t TraceReader::update_buffer(Address start, Address end) {
    assert(start < end);

    /* align addresses */
    Address start_aligned = start & pagemask;
    Address end_aligned = end % pagesize == 0 ? end : (end & pagemask) + pagesize;
    size_t idx = address_to_idx(start_aligned);

   this->dev->read_virt(start_aligned, end_aligned-start_aligned, &this->buffer[idx]);
   return (end - start) / sizeof(l4_tracebuffer_entry_t);
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

size_t TraceReader::address_to_idx(Address a) const {
    assert(a >= this->tbuf_start);
    assert(a <= this->tbuf_end);

    return (a - this->tbuf_start) / sizeof(l4_tracebuffer_entry_t);
}
