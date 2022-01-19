#pragma once

#include "fiasco/fiasco.h"
#include <array>
#include <leechcore.h>
#include <map>

class Device;

typedef std::array<Address, 512> page_dir_t;

class PageTable
{
public:
	PageTable(Device *dev, Address cr3);
	Address virt_to_phys(Address virt);

private:
	page_dir_t get_page_dir(Address a);

	Device *dev;
	Address cr3;
	std::map<Address, page_dir_t> page_dir_cache;
};

