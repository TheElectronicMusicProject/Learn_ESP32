# How to use

Use `spiffsgen.py` as follows:
`python C:\Users\<USER>\<OTHER>\Learn-ESP32\Storage\9_8_spiffs_updating_files\spiffsgen.py 0x100000 spiffs_dir spiffs_dir.bin`

It creates the binary file you can place in partition to update a particular file/directory.

Use `esptool.py` as follows:
`python C:\Users\<USER>\<OTHER>\Learn-ESP32\Storage\9_8_spiffs_updating_files\esptool.py --chip esp32 --port COM<X> --baud 115200 write_flash -z 0x110000 spiffs_dir.bin`

It flashes the current SPIFFS partition with the new partition.
No firmware re-flash is required. It only updates the partition.
