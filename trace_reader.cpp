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
}

TraceReader::~TraceReader()
{
	delete this->ptab;
	delete this->pi;
	delete this->dev;
}

bool TraceReader::isNewRecordAvailable()
{
	//read status
	std::cout << "Virt: "
		  << std::hex << this->pi->Tbuf_status_page
		  << std::endl;
	std::cout << "Phys: "
		  << std::hex << this->ptab->getPhysicalAddress(this->pi->Tbuf_status_page)
		  << std::endl;

	return false;
}

