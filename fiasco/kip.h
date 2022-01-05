// AUTOMATICALLY GENERATED -- DO NOT EDIT!         -*- c++ -*-

#ifndef kip_h
#define kip_h

#include "fiasco.h"


static char const *const memory_desc_types[] = {
    "Undefined",
    "Conventional",
    "Reserved",
    "Dedicated",
    "Shared",
    "(undef)",
    "(undef)",
    "(undef)",
    "(undef)",
    "(undef)",
    "(undef)",
    "(undef)",
    "(undef)",
    "(undef)",
    "Bootloader",
    "Arch"
};


//
// INTERFACE definition follows 
//


class Mem_desc
{
public:
  enum Mem_type
  {
    Undefined    = 0x0,
    Conventional = 0x1,
    Reserved     = 0x2,
    Dedicated    = 0x3,
    Shared       = 0x4,
    Kernel_tmp   = 0x7,

    Info         = 0xd,
    Bootloader   = 0xe,
    Arch         = 0xf,
  };

  enum Ext_type_info
  {
    Info_acpi_rsdp = 0
  };

private:
  Mword _l, _h;

public:  
  inline Mem_desc(Address start, Address end, Mem_type t, bool v = false,
                     unsigned st = 0);
  
  inline Address start() const;
  
  inline Address end() const;
  
  inline void
  type(Mem_type t);
  
  inline Mem_desc::Mem_type type() const;
  
  inline unsigned ext_type() const;
  
  inline unsigned is_virtual() const;
  
  inline bool contains(Address addr) const;
  
  inline bool valid() const;
};

/** Kernel info page */
class Kip
{
public:
  void print() const;

  char const *version_string() const;


  /* 0x00 */
  Mword      magic;
  Mword      version;
  Unsigned8  offset_version_strings;
  Unsigned8  fill0[sizeof(Mword) - 1];
  Unsigned8  kip_sys_calls;
  Unsigned8  fill1[sizeof(Mword) - 1];

  /* the following stuff is undocumented; we assume that the kernel
     info page is located at offset 0x1000 into the L4 kernel boot
     image so that these declarations are consistent with section 2.9
     of the L4 Reference Manual */

  /* 0x10   0x20 */
  Mword      sched_granularity;
  Mword      _res1[3];

  /* 0x20   0x40 */
  Mword      sigma0_sp, sigma0_ip;
  Mword      _res2[2];

  /* 0x30   0x60 */
  Mword      sigma1_sp, sigma1_ip;
  Mword      _res3[2];

  /* 0x40   0x80 */
  Mword      root_sp, root_ip;
  Mword      _res4[2];

  /* 0x50   0xA0 */
  Mword      _res_50;
  Mword      _mem_info;
  Mword      _res_58[2];

  /* 0x60   0xC0 */
  Mword      _res5[16];

  /* 0xA0   0x140 */
  volatile Cpu_time clock;
  Unsigned64 _res6;

  /* 0xB0   0x150 */
  Mword      frequency_cpu;
  Mword      frequency_bus;

  /* 0xB8   0x160 */
  Mword      _res7[10 + ((sizeof(Mword) == 8) ? 2 : 0)];

  /* 0xE0   0x1C0 */
  Mword      user_ptr;
  Mword      vhw_offset;
  Mword      _res8[2];

  /* 0xF0   0x1E0 */


private:

public:
  struct Platform_info
  {
    char name[16];
    Unsigned32 is_mp;
  };

  /* 0x1E0 */
  Platform_info platform_info;
  Unsigned32 __reserved[3];

public:  
  inline unsigned num_mem_descs() const;
  
#if 0
  inline cxx::static_vector<Mem_desc>
  mem_descs_a();
  
  inline cxx::static_vector<Mem_desc const>
  mem_descs_a() const;
#endif
  
  inline void num_mem_descs(unsigned n);
  
  Mem_desc *add_mem_region(Mem_desc const &md);
  
  static inline Kip *k();

public:  
  inline Mem_desc *mem_descs();
  
  inline Mem_desc const *mem_descs() const;
};

#define L4_KERNEL_INFO_MAGIC (0x4BE6344CL) /* "L4µK" */

//============================================================================





inline Mem_desc::Mem_desc(Address start, Address end, Mem_type t, bool v,
                   unsigned st)
: _l((start & ~0x3ffUL) | (t & 0x0f) | ((st << 4) & 0x0f0)
     | (v?0x0200:0x0)),
  _h(end)
{}



inline ALWAYS_INLINE Address Mem_desc::start() const
{ return _l & ~0x3ffUL; }



inline ALWAYS_INLINE Address Mem_desc::end() const
{ return _h | 0x3ffUL; }



inline ALWAYS_INLINE void
Mem_desc::type(Mem_type t)
{ _l = (_l & ~0x0f) | (t & 0x0f); }



inline ALWAYS_INLINE Mem_desc::Mem_type Mem_desc::type() const
{ return (Mem_type)(_l & 0x0f); }



inline unsigned Mem_desc::ext_type() const
{ return (_l >> 4) & 0x0f; }



inline ALWAYS_INLINE unsigned Mem_desc::is_virtual() const
{ return _l & 0x200; }



inline bool Mem_desc::contains(Address addr) const
{
  return start() <= addr && end() >= addr;
}



inline ALWAYS_INLINE bool Mem_desc::valid() const
{ return type() && start() < end(); }



inline ALWAYS_INLINE unsigned Kip::num_mem_descs() const
{ return _mem_info & ((1UL << (MWORD_BITS/2))-1); }




inline void Kip::num_mem_descs(unsigned n)
{
  _mem_info = (_mem_info & ~((1UL << (MWORD_BITS / 2)) - 1))
              | (n & ((1UL << (MWORD_BITS / 2)) - 1));
}



inline ALWAYS_INLINE Mem_desc *Kip::mem_descs()
{ return (Mem_desc*)(((Address)this) + (_mem_info >> (MWORD_BITS/2))); }



inline Mem_desc const *Kip::mem_descs() const
{ return (Mem_desc const *)(((Address)this) + (_mem_info >> (MWORD_BITS/2))); }


 








#endif // kip_h
