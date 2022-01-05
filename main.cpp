#include <cstring>
#include <iostream>
#include <leechcore.h>

int main()
{
	LC_CONFIG cfg = { 0 };
	HANDLE lc_ctx;

	//cfg.paMax = 0x500000;
	cfg.dwVersion = LC_CONFIG_VERSION;
	strncpy(cfg.szDevice, "fpga", sizeof(cfg.szDevice));
	cfg.dwPrintfVerbosity = 0xf;

	lc_ctx = LcCreate(&cfg);
	if (!lc_ctx) {
		std::cerr << __func__ << ": Cannot create leechcore context" << std::endl;
		return 1;
	}
	LcClose(lc_ctx);

	return 0;
}
