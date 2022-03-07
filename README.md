# l4trace

Extract the fiasco trace buffer running on a target machine using a
PCIe screamer device.

## Patching Fiasco
The screamer access the tracebuffer memory via Busmastering DMA.
You need to enable `CONFIG_IOMMU_PASSTHROUGH=y`. \
To enable Busmastering DMA and place the tracebuffer address at
a well-known location, apply the provided patch
in `./fiasco.patch`.

## Building
Clone submodules
```
git submodule update --init --recursive
```

Install dependencies and build LeechCore
```
sudo apt install libusb-1.0-0-dev 
make leechcore
```

Install dependencies and build babeltrace
```
sudo apt install automake autoconf libtool flex bison asciidoc xmlto
make babeltrace
```

Optional: Add udev rules to use device as group member of `dialout`
```
sudo cp udev.rules /etc/udev/rules.d/99-screamerM2.rules
sudo udevadm control --reload-rules
```

Build and run l4trace
```
make run
```
*Hint: To run `l4trace` as a standalone binary, you need to add
the content of `./lib/` to your `LD_LIBRARY_PATH`.
E.g. copy libraries to `/usr/lib/`.*

see babeltrace build instructions

## Problems
- where is tracebuffer -> pcileechinfo in kip
- virt_to_phys -> pdir aus kip, traverse pdir structure
- reading way to slow
	- cache/reduce/calc virt_to_phys
	- read multiple
	- reduce allocations
	- improve buf size
- records not ordered
	- cause unclear
	-> fix: sort
