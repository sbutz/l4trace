#pragma once

#include "fiasco/fiasco.h"
#include "fiasco/kip.h"
#include <leechcore.h>
#include <string>

const size_t pagesize = 4096;

class Device
{
public:
	Device(std::string name, int loglevel);
	~Device();
	void read(Address addr, size_t size, void *buf);

	Kip *kip;

private:
	HANDLE lc_ctx;
};
