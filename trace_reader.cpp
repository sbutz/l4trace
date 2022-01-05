#include "trace_reader.h"
#include <cstdlib>
#include <cstring>
#include <err.h>
#include <iostream>
#include <stdio.h>
#include <vector>


static void *emalloc(size_t size);
static void dma_read(HANDLE ctx, uint64_t addr, size_t size, void *buf);
static void setMemoryMap(HANDLE lc_ctx, Kip *kip);

TraceReader::TraceReader()
{
	LC_CONFIG lc_cfg = { 0 };
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
	strncpy(lc_cfg.szDevice, "fpga", sizeof(MAX_PATH));
	/*
	 * Set log level.
	 * Maximum is 0xf;
	 */
	lc_cfg.dwPrintfVerbosity = LC_CONFIG_PRINTF_VVV;


	this->lc_ctx = LcCreate(&lc_cfg);
	if (!this->lc_ctx)
		errx(1, "%s: Cannot create leechcore context", __func__);

	/*
	 * Read Kernel Info Page.
	 */
	//TODO: use new keyword
	//TODO: kip addr+size in constants
	Kip *kip = (Kip *)emalloc(0x1000);
	dma_read(this->lc_ctx, 0x400000, 0x1000, kip);

	if (kip->magic != 0x4be6344c)
		errx(1, "%s: failed to read kernel info page", __func__);

	/*
	 * Set readable memory
	 */
	setMemoryMap(this->lc_ctx, kip);

	/*
	 * Read info struct.
	 */
	if (!kip->_res5[0])
		errx(1, "no Pcileechinfo found");
	Pcileechinfo *pi = new Pcileechinfo();
	dma_read(this->lc_ctx, kip->_res5[0], sizeof(*pi), pi);

	free(kip);
}

TraceReader::~TraceReader()
{
	LcClose(this->lc_ctx);
}

static void *emalloc(size_t size) {
	void *buf;

	buf = malloc(size);
	if (!buf)
		errx(1, "%s: out of memory", __func__);
	return buf;
}

static void dma_read(HANDLE ctx, uint64_t addr, size_t size, void *buf)
{
	if (!LcRead(ctx, (ULONG64)addr, size, (PBYTE)buf))
		errx(1, "%s: LcRead failed", __func__);
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
