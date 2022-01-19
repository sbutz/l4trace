#pragma once

#include "fiasco/fiasco.h"
#include <leechcore.h>
#include <string>

const size_t pagesize = 4096;

class PageTable;
class Kip;

class Device
{
public:
	Device(std::string name, int loglevel);
	~Device();

	void add_page_table(PageTable *ptab);
	void read(Address addr, size_t size, void *buf);
	void read_virt(Address addr, size_t size, void *buf);

	Kip *kip;

private:
	HANDLE lc_ctx;
	PageTable *ptab;
};
