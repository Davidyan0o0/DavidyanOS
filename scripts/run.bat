@echo off

qemu-system-i386.exe -drive format=raw,file=out/os.img

echo Run OK

pause
