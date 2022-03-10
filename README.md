# l4trace

Extract the fiasco trace buffer running on a target machine using a
PCIe screamer device.

## Equipment
- [M.2 Screamer](https://shop.lambdaconcept.com/home/43-screamer-m2.html)
- [Thunderbold Adapter](https://eshop.macsales.com/shop/envoy-express/thunderbolt-3)

## Patching Fiasco
The screamer access the tracebuffer memory via Busmastering DMA.
You need to set:
```
CONFIG_IOMMU_PASSTHROUGH=y
CONFIG JDB=y
CONFIG JDB LOGGING=y
```

To enable Busmastering DMA and place the tracebuffer address at
a well-known location, add the provided module (`./jdb_scream.cpp`)
to your kernel.

## Building l4trace
Clone submodules
```
git submodule update --init --recursive
```

Install dependencies
```
sudo apt install libusb-1.0-0-dev babeltrace2 libbabeltrace2-dev
```

Build LeechCore
```
make all
```

Optional: [Build Babeltrace for development](https://babeltrace.org/docs/v2.0/libbabeltrace2/guide-build-bt2-dev.html)


Optional: Add udev rules to use device as group member of `dialout`
```
sudo cp udev.rules /etc/udev/rules.d/99-screamerM2.rules
sudo udevadm control --reload-rules
```

## Run
```
./l4trace.sh
```