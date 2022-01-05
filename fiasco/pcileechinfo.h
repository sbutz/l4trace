#pragma once

#include "fiasco.h"
#include "jdb_ktrace.h"

struct Pcileechinfo
{
  Mword Kernel_image_offset;
  Mword Kernel_image;
  Mword Kernel_image_end;
  Mword physmem_offs;
  Mword Physmem;
  Mword Physmem_end;
  Mword Tbuf_status_page;
  Mword econ_out_buf;
  Mword econ_out_buf_size;
  Mword econ_out_buf_r;
  Mword econ_out_buf_w;
  Mword kdir;
  Mword jdb_log_table;
  Mword jdb_log_table_end;
};

struct Tb_entry_formatter;
struct Tb_log_table_entry
{
  char const *name;
  unsigned char *patch;
  Tb_entry_formatter *fmt;
};

