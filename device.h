#pragma once

#include "fiasco/fiasco.h"
#include <string>

/* Ignore some warnings from LeechCore */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wwrite-strings"
#include <leechcore.h>
#pragma GCC diagnostic pop


const uint64_t pagesize = 4096;
const uint64_t pagemask = ~(pagesize-1);

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
