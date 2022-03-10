#IMPLEMENTATION[amd64]:

#include <cstdio>

#include "cpu.h"
#include "jdb.h"
#include "jdb_module.h"
#include "kmem.h"
#include "pci.h"

class Jdb_scream : public Jdb_module
{
public:
  Jdb_scream() FIASCO_INIT;

  struct Pcileechinfo
  {
    Mword Tbuf_status_page;
    Mword kdir;
  };

private:
  static struct Pcileechinfo _pcileechinfo;
  static const Unsigned32 _pci_ids_thblt[];
};

struct Jdb_scream::Pcileechinfo Jdb_scream::_pcileechinfo;

/* PCI device ids of the screamer and the thunderbolt bridges */
const Unsigned32 Jdb_scream::_pci_ids_thblt[] = {
	0x066610ee,
	0x15c08086,
	0x15e78086,
	0x15e88086,
	0x15e98086,
	0
};


PUBLIC
Jdb_module::Action_code
Jdb_scream::action(int cmd, void *&, char const *&, int &) override
{
  if (cmd)
    return NOTHING;

  /* Enable Busmastering for Screamer. */
  for (unsigned bus = 0; bus < 64; ++bus)
    for (unsigned dev = 0; dev < 32; ++dev)
      for (unsigned fn = 0; fn < 8; ++fn)
        {
          Pci::Cfg_addr pci_addr(bus, dev, fn);
          Unsigned32 id = Pci::read_cfg(pci_addr + 0, Pci::Cfg_width::Long);
          for (const Unsigned32 *pci_id = _pci_ids_thblt; *pci_id; ++pci_id)
            {
              if (id != *pci_id)
                continue;
	      Unsigned32 cmd = Pci::read_cfg(pci_addr + 4, Pci::Cfg_width::Short);
	      printf("Enable busmaster %8x %x:%x.%x cmd %x\n", id, bus, dev, fn, cmd);
	      cmd |= (1UL << 2);
	      Pci::write_cfg(pci_addr + 4, cmd);
            }
        }

  /* Place Pcileechinfo, so that screamer can find the tracebuffer. */
  auto pi = &_pcileechinfo;
  pi->Tbuf_status_page    = Mem_layout::Tbuf_status_page;
  pi->kdir                = Cpu::get_pdbr();
  Kip::k()->_res5[0] = Kmem::virt_to_phys(pi);

  return NOTHING;
}

PUBLIC
int
Jdb_scream::num_cmds() const override
{
  return 1;
}

PUBLIC
Jdb_module::Cmd const *
Jdb_scream::cmds() const
{
  static Cmd cs[] =
    {
      { 0, "G", "scream", "", "Activate Screamer", 0 }
    };
  return cs;
}

IMPLEMENT
Jdb_scream::Jdb_scream()
  : Jdb_module("MONITORING")
{
}

static Jdb_scream jdb_scream INIT_PRIORITY(JDB_MODULE_INIT_PRIO);