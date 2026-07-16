@echo off

qemu-system-i386.exe -drive format=raw,file=out/os.img -net nic,model=ne2k_isa,macaddr=52:54:00:12:34:56 -net user

echo Run OK

pause
