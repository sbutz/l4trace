#include "device.h"
#include "page_table.h"
#include <cassert>
#include <err.h>

static const uint64_t flag_present = 0x1;
static const uint64_t flag_large   = 0x1 << 7;

PageTable::PageTable(Device *dev, Address cr3)
	: dev(dev), cr3(cr3)
{
}

//TODO: FRAGE: daten die ueber mehrere pages gehen, wie lesen?
//TODO: save cr4 to know the usable bits
/*
 * Get physical address by traversing the page directory structure.
 *
 * For more info refer to https://www.lowlevel.eu/wiki/Paging.
 */
Address
PageTable::virt_to_phys(Address virt)
{
	/*
	 * The address has 64 bit in total. 48 of them are assumed to usable
	 * as follows:
	 * 16 unused
	 * 9 bit index for page directory pointer
	 * 9 bit index for page directory
	 * 9 bit index for page table
	 * 12 bit offset
	 */
	uint64_t mask = (~0x0ULL) >> (64 - 48);
	Address pdir_phys = this->cr3;
	Address pte = 0;

	for (int shift = 39; shift >= 12; shift -= 9)
	{
		/* Read page directory (possibly cached) */
		page_dir_t pdir = this->get_page_dir(pdir_phys & pagemask);

		/* Find entry */
		int offset = (virt & mask) >> shift;
		pte = pdir[offset];

		/* Is entry valid? */
		if (!(pte & flag_present))
			errx(1, "%s: page table entry not present (virt=%#lx)",
					__func__, virt);

		mask >>= 9;
		pdir_phys = pte & pagemask;

		/* Abort traversing if page is a large page */
		if (pte & flag_large)
			break;
	}

	return (pte & ~mask) | (virt & mask);
}

page_dir_t PageTable::get_page_dir(Address a)
{
	assert((a & pagemask) == a);

	auto it = this->page_dir_cache.find(a);
	if (it != this->page_dir_cache.end()) {
		return it->second;
	}
	else {
		page_dir_t tmp;
		this->dev->read(a, pagesize, (uint8_t *) tmp.data());
		this->page_dir_cache[a] = tmp;
		return tmp;
	}
}

