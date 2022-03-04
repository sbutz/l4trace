# l4trace

Extract the fiasco trace buffer running on a target machine using a
fpga screamer device.

highly vs-top specific (fiasco version, amd64, genua tweaks)


## Building
Clone Repo.
```shell
git clone --recurse-submodules git@github.com:sbutz/l4trace.git
```
Install libusb and pkg-config to build LeechCore.
```
apt-get install libusb-1.0-0-dev pkg-config
```
Babeltrace requirements
see babeltrace build instructions

- put so files in /usr/lib OR LD_LIBRARY_PATH to GIT_ROOT/lib

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
