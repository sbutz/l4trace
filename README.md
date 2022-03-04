# l4trace

Extract the fiasco trace buffer running on a target machine using a
PCIe screamer device. \
To locate the tracebuffer, fiasco requires an additional JDB module.

REFERENCE PATCH

## Building
Clone submodules
```
git submodule update --init --recursive
```

Install libusb and build LeechCore
```
sudo apt install libusb-1.0-0-dev 
make leechcore
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
