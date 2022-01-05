#pragma once

#include "fiasco/kip.h"
#include "fiasco/pcileechinfo.h"
#include <leechcore.h>

class TraceReader {
public:
	TraceReader();
	~TraceReader();

private:
	HANDLE lc_ctx;
	Pcileechinfo *pi;
};
