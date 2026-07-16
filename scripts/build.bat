@echo off
setlocal


echo ==========================
echo  Build x86 OS
echo ==========================


rem ==========================
rem Toolchain
rem ==========================

set CC=i686-elf-gcc
set LD=i686-elf-ld
set OBJCOPY=i686-elf-objcopy



rem ==========================
rem Clean
rem ==========================

if not exist out mkdir out

del out\*.bin >nul 2>nul
del out\*.o   >nul 2>nul
del out\*.elf >nul 2>nul
del out\os.img >nul 2>nul
del out\*.vmdk >nul 2>nul




echo.
echo [1] Build bootloader


nasm boot/boot.asm -f bin -o out/boot.bin

if errorlevel 1 goto error



echo.
echo [2] Build loader


nasm boot/loader.asm -f bin -o out/loader.bin

if errorlevel 1 goto error





echo.
echo [3] Build kernel entry


nasm kernel/arch/x86/kernel_entry.asm -f elf32 -o out/kernel_entry.o

if errorlevel 1 goto error


echo.
echo [3] Build irq


nasm kernel/arch/x86/irq.asm -f elf32 -o out/irq.o

if errorlevel 1 goto error


echo.
echo [3] Build isr


nasm kernel/arch/x86/isr.asm -f elf32 -o out/isr.o

if errorlevel 1 goto error



echo.
echo [4] Compile kernel.c


%CC% ^
-m32 ^
-ffreestanding ^
-fno-pie ^
-fno-stack-protector ^
-I. ^
-c kernel/kernel.c ^
-o out/kernel.o


if errorlevel 1 goto error





echo.
echo [5-1] Compile vga.c


%CC% ^
-m32 ^
-ffreestanding ^
-fno-pie ^
-fno-stack-protector ^
-I. ^
-c drivers/video/vga.c ^
-o out/vga.o


if errorlevel 1 goto error


echo.
echo [5-2] Compile console.c


%CC% ^
-m32 ^
-ffreestanding ^
-fno-pie ^
-fno-stack-protector ^
-I. ^
-c kernel/console.c ^
-o out/console.o


if errorlevel 1 goto error


echo.
echo [5-3] Compile system.c


%CC% ^
-m32 ^
-ffreestanding ^
-fno-pie ^
-fno-stack-protector ^
-I. ^
-c kernel/system.c ^
-o out/system.o


if errorlevel 1 goto error


echo.
echo [5-4] Compile idt.c


%CC% ^
-m32 ^
-ffreestanding ^
-fno-pie ^
-fno-stack-protector ^
-I. ^
-c kernel/arch/x86/idt.c ^
-o out/idt.o


if errorlevel 1 goto error


echo.
echo [5-5] Compile pic.c


%CC% ^
-m32 ^
-ffreestanding ^
-fno-pie ^
-fno-stack-protector ^
-I. ^
-c kernel/arch/x86/pic.c ^
-o out/pic.o


if errorlevel 1 goto error


echo.
echo [5-6] Compile io.c


%CC% ^
-m32 ^
-ffreestanding ^
-fno-pie ^
-fno-stack-protector ^
-I. ^
-c kernel/arch/x86/io.c ^
-o out/io.o


if errorlevel 1 goto error


echo.
echo [5-7] Compile exception.c


%CC% ^
-m32 ^
-ffreestanding ^
-fno-pie ^
-fno-stack-protector ^
-I. ^
-c kernel/arch/x86/exception.c ^
-o out/exception.o


if errorlevel 1 goto error


echo.
echo [5-8] Compile timer.c


%CC% ^
-m32 ^
-ffreestanding ^
-fno-pie ^
-fno-stack-protector ^
-I. ^
-c kernel/arch/x86/timer.c ^
-o out/timer.o


if errorlevel 1 goto error


echo.
echo [5-9] Compile rtc.c


%CC% ^
-m32 ^
-ffreestanding ^
-fno-pie ^
-fno-stack-protector ^
-I. ^
-c kernel/arch/x86/rtc.c ^
-o out/rtc.o


if errorlevel 1 goto error


echo.
echo [5-10] Compile power.c


%CC% ^
-m32 ^
-ffreestanding ^
-fno-pie ^
-fno-stack-protector ^
-I. ^
-c kernel/arch/x86/power.c ^
-o out/power.o


if errorlevel 1 goto error


echo.
echo [5-11] Compile string.c


%CC% ^
-m32 ^
-ffreestanding ^
-fno-pie ^
-fno-stack-protector ^
-I. ^
-c kernel/lib/string.c ^
-o out/string.o


if errorlevel 1 goto error


echo.
echo [5-12] Compile device.c


%CC% ^
-m32 ^
-ffreestanding ^
-fno-pie ^
-fno-stack-protector ^
-I. ^
-c kernel/device/device.c ^
-o out/device.o


if errorlevel 1 goto error


echo.
echo [5-13] Compile ramdisk.c


%CC% ^
-m32 ^
-ffreestanding ^
-fno-pie ^
-fno-stack-protector ^
-I. ^
-c drivers/storage/ramdisk.c ^
-o out/ramdisk.o


if errorlevel 1 goto error


echo.
echo [5-14] Compile ramfs.c


%CC% ^
-m32 ^
-ffreestanding ^
-fno-pie ^
-fno-stack-protector ^
-I. ^
-c kernel/fs/ramfs.c ^
-o out/ramfs.o


if errorlevel 1 goto error


echo.
echo [5-15] Compile net.c


%CC% ^
-m32 ^
-ffreestanding ^
-fno-pie ^
-fno-stack-protector ^
-I. ^
-c kernel/net/net.c ^
-o out/net.o


if errorlevel 1 goto error



echo.
echo [5-16] Compile shell.c


%CC% ^
-m32 ^
-ffreestanding ^
-fno-pie ^
-fno-stack-protector ^
-I. ^
-c kernel/shell.c ^
-o out/shell.o


if errorlevel 1 goto error


echo.
echo [5-17] Compile heap.c


%CC% ^
-m32 ^
-ffreestanding ^
-fno-pie ^
-fno-stack-protector ^
-I. ^
-c kernel/memory/heap.c ^
-o out/heap.o


if errorlevel 1 goto error


echo.
echo [5-18] Compile keyboard.c


%CC% ^
-m32 ^
-ffreestanding ^
-fno-pie ^
-fno-stack-protector ^
-I. ^
-c drivers/input/keyboard.c ^
-o out/keyboard.o


if errorlevel 1 goto error



echo.
echo [6] Link kernel

%LD% ^
-T linker.ld ^
-o out/kernel.elf ^
out/kernel_entry.o ^
out/isr.o ^
out/kernel.o ^
out/system.o ^
out/shell.o ^
out/device.o ^
out/ramdisk.o ^
out/ramfs.o ^
out/net.o ^
out/string.o ^
out/heap.o ^
out/io.o ^
out/exception.o ^
out/timer.o ^
out/rtc.o ^
out/power.o ^
out/vga.o ^
out/console.o ^
out/idt.o ^
out/pic.o ^
out/keyboard.o ^
out/irq.o


if errorlevel 1 goto error





echo.
echo [7] Convert kernel


%OBJCOPY% ^
-O binary ^
out/kernel.elf ^
out/kernel.bin


if errorlevel 1 goto error






echo.
echo [8] Create image


fsutil file createnew out/os.img 10240000



rem boot 第0扇区

dd if=out/boot.bin ^
of=out/os.img ^
bs=512 ^
count=1 ^
conv=notrunc




rem loader 第1扇区

dd if=out/loader.bin ^
of=out/os.img ^
bs=512 ^
seek=1 ^
conv=notrunc




rem kernel 第5扇区

dd if=out/kernel.bin ^
of=out/os.img ^
bs=512 ^
seek=5 ^
conv=notrunc





echo.
echo ==========================
echo Build Success
echo ==========================
echo Use scripts\run.bat to start QEMU.

pause

goto end




:error

echo.
echo ==========================
echo Build Failed
echo ==========================

pause

exit /b 1




:end
exit /b 0
