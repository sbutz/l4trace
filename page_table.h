#pragma once

#include "fiasco/fiasco.h"
#include "device.h"
#include <leechcore.h>

class PageTable
{
public:
	PageTable(Device *dev, Address cr3);
	Address getPhysicalAddress(Address virt);

private:
	Device *dev;
	Address cr3;
};

