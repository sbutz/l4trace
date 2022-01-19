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

	if (!this->pi->Tbuf_status_page)
		errx(1, "%s: no Tbuf_status_page found", __func__);

	this->ptab = new PageTable(this->dev, pi->kdir);
	this->dev->add_page_table(this->ptab);

	struct Tracebuffer_status status = this->get_status();
	this->tbuf_start = status.window[0].tracebuffer;
	this->tbuf_start_phys = this->ptab->virt_to_phys(this->tbuf_start);
	this->tbuf_end = status.window[1].tracebuffer + status.window[1].size;
	this->tbuf_size = this->tbuf_end - this->tbuf_start;
	this->last_read = status.current;
	this->last_num = 0;

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

std::vector<l4_tracebuffer_entry_t> TraceReader::get_new_records()
{
	size_t n = this->tbuf_size / sizeof(l4_tracebuffer_entry_t);
	std::vector<l4_tracebuffer_entry_t> tracebuffer;
	tracebuffer.resize(n);
	std::vector<l4_tracebuffer_entry_t> records;

	struct Tracebuffer_status status = this->get_status();

	/* No trace records available */
	if (status.current == 0)
		return records;

	//TODO: dont always read full buffer
	this->dev->read_virt(this->tbuf_start, this->tbuf_size,
		(uint8_t *) tracebuffer.data());

	int current_idx = (status.current - this->tbuf_start) /
		sizeof(l4_tracebuffer_entry_t);
	int last_read_idx = (this->last_read - this->tbuf_start) /
		sizeof(l4_tracebuffer_entry_t);

	if (this->last_read == 0) {
		records.insert(records.begin(), tracebuffer.begin(),
			tracebuffer.begin()+current_idx+1);
	}
	else if (this->last_read == status.current &&
		 this->last_num == tracebuffer[current_idx]._number) {
		/* No new records */
		return records;
	}
	else if (this->last_read >= status.current) {
		records.insert(records.end(),
			tracebuffer.begin()+last_read_idx+1,
			tracebuffer.end());
		records.insert(records.end(),
			tracebuffer.begin(),
			tracebuffer.begin()+current_idx+1);
	}
	else /* this->last < status.current */ {
		records.insert(records.end(),
			tracebuffer.begin()+last_read_idx+1,
			tracebuffer.begin()+current_idx+1);
	}

	this->last_read = status.current;
	this->last_num = records.back()._number;

	return records;
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
