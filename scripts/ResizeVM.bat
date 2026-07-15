@echo off

qemu-img resize ../out/os.img 128M

qemu-img convert -f raw ../out/os.img -O vmdk ../out/VM.vmdk

echo Run OK

pause
