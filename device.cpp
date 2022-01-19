#include "device.h"
#include "fiasco/kip.h"
#include "page_table.h"
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <err.h>
#include <vector>

static HANDLE lc_init(std::string name, int loglevel);
static void *emalloc(size_t size);
static void setMemoryMap(HANDLE lc_ctx, Kip *kip);

static const size_t kip_size = 0x1000;
static const Address kip_addr = 0x400000;

Device::Device(std::string name, int loglevel)
{
	/*
	 * Initialize leechcore.
	 */
	this->lc_ctx = lc_init(name, loglevel);

	/*
	 * Read Kernel Info Page.
	 */
	this->kip = (Kip *)emalloc(kip_size);
	this->read(kip_addr, kip_size, this->kip);
	if (this->kip->magic != 0x4be6344c)
		errx(1, "%s: failed to read kernel info page", __func__);

	/*
	 * Set readable memory
	 */
	setMemoryMap(this->lc_ctx, this->kip);
}

Device::~Device()
{
	free(this->kip);
	LcClose(this->lc_ctx);
}

void Device::add_page_table(PageTable *ptab)
{
	this->ptab = ptab;
}

void Device::read(Address addr, size_t size, void *buf)
{
	if (!LcRead(this->lc_ctx, (ULONG64)addr, size, (PBYTE)buf))
		errx(1, "%s: LcRead failed for %#lx", __func__, addr);
}

void Device::read_virt(Address addr, size_t size, void *buf)
{
	PPMEM_SCATTER ppmens;

	assert(this->ptab != nullptr);
	/* XXX: For now all addresses are page aligned */
	assert(addr % pagesize == 0);
	assert(size > 0 && size % pagesize == 0);

	BOOL result = LcAllocScatter2(size, (PBYTE)buf, size/pagesize, &ppmens);
	if (!result)
		errx(1, "%s: LcAllocScatter2 failed", __func__);
	for (int i = 0; i < size/pagesize; i++) {
		ppmens[i]->f = false;
		/* Virtual address might no be continous. */
		ppmens[i]->qwA = this->ptab->virt_to_phys(addr + i*pagesize);
	}

	LcReadScatter(this->lc_ctx, size/pagesize, ppmens);
	for (int i = 0; i < size/pagesize; i++) {
		if (!ppmens[i]->f)
			errx(1, "%s: LcReadScatter failed", __func__);
	}

	LcMemFree(ppmens);
}

static HANDLE lc_init(std::string name, int loglevel)
{
	LC_CONFIG lc_cfg = { 0 };
	HANDLE lc_ctx = NULL;

	/*
	 * Enough to read KIP and memory map.
	 * Add readable memory as declared in the memory map.
	 */
	lc_cfg.paMax = 0x500000;
	/*
	 * Must be set.
	 */
	lc_cfg.dwVersion = LC_CONFIG_VERSION;
	/*
	 * Use fpga screamer.
	 */
	strncpy(lc_cfg.szDevice, name.c_str(), sizeof(MAX_PATH));
	/*
	 * Set log level.
	 * One of LC_CONFIG_PRINTF_* or 0xf
	 */
	lc_cfg.dwPrintfVerbosity = loglevel;

	lc_ctx = LcCreate(&lc_cfg);
	if (!lc_ctx)
		errx(1, "%s: Cannot create leechcore context", __func__);

	return lc_ctx;
}

static void setMemoryMap(HANDLE lc_ctx, Kip *kip)
{
	std::vector<LC_MEMMAP_ENTRY> memmap;
	Mem_desc *md = kip->mem_descs();

	for (int i = 0, len = kip->num_mem_descs(); i < len; i++, md++) {
		if (md->valid() &&
		    !md->is_virtual() &&
		    md->type() == Mem_desc::Mem_type::Conventional)
		{
			LC_MEMMAP_ENTRY tmp = {
				md->start(),
				md->end()-md->start()+1,
				0
			};
			memmap.push_back(tmp);
		}
	}

	if (!LcCommand(lc_ctx, LC_CMD_MEMMAP_SET_STRUCT,
			memmap.size()*sizeof(memmap[0]), (PBYTE) &memmap[0],
			NULL, 0))
		errx(1, "%s: Failed to set memmap", __func__);
}

static void *emalloc(size_t size)
{
	void *buf;

	buf = malloc(size);
	if (!buf)
		errx(1, "%s: out of memory", __func__);
	return buf;
}

