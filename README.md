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

Install dependencies
```
sudo apt install libusb-1.0-0-dev babeltrace
```

Build LeechCore
```
make leechcore
```

Optional: [Build Babeltrace for development](https://babeltrace.org/docs/v2.0/libbabeltrace2/guide-build-bt2-dev.html)


Optional: Add udev rules to use device as group member of `dialout`
```
sudo cp udev.rules /etc/udev/rules.d/99-screamerM2.rules
sudo udevadm control --reload-rules
```

Build and run l4trace
```
make run
```