@echo off

qemu-img convert -f raw ../out/os.img -O vmdk ../out/os.vmdk

echo Run OK

pause
