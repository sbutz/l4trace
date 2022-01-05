#pragma once

#include "device.h"
#include "fiasco/kip.h"
#include "fiasco/pcileechinfo.h"
#include "page_table.h"
#include <leechcore.h>

class TraceReader
{
public:
	TraceReader(int loglevel);
	~TraceReader();
	bool isNewRecordAvailable();

private:
	//TODO: use &references
	Device *dev;
	Pcileechinfo *pi;
	PageTable *ptab;
};
