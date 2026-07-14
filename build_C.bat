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

del *.bin >nul 2>nul
del *.o   >nul 2>nul
del *.elf >nul 2>nul
del os.img >nul 2>nul



echo.
echo [1] Build bootloader


nasm boot.asm -f bin -o boot.bin

if errorlevel 1 goto error



echo.
echo [2] Build loader


nasm loader.asm -f bin -o loader.bin

if errorlevel 1 goto error





echo.
echo [3] Build kernel entry


nasm kernel_entry.asm -f elf32 -o kernel_entry.o

if errorlevel 1 goto error





echo.
echo [4] Compile kernel.c


%CC% ^
-m32 ^
-ffreestanding ^
-fno-pie ^
-fno-stack-protector ^
-c kernel.c ^
-o kernel.o


if errorlevel 1 goto error





echo.
echo [5] Compile vga.c


%CC% ^
-m32 ^
-ffreestanding ^
-fno-pie ^
-fno-stack-protector ^
-c vga.c ^
-o vga.o


if errorlevel 1 goto error





echo.
echo [6] Link kernel



%LD% ^
-T linker.ld ^
-o kernel.elf ^
kernel_entry.o ^
kernel.o ^
vga.o


if errorlevel 1 goto error





echo.
echo [7] Convert kernel


%OBJCOPY% ^
-O binary ^
kernel.elf ^
kernel.bin


if errorlevel 1 goto error






echo.
echo [8] Create image


fsutil file createnew os.img 51200



rem boot 第0扇区

dd if=boot.bin ^
of=os.img ^
bs=512 ^
count=1 ^
conv=notrunc




rem loader 第1扇区

dd if=loader.bin ^
of=os.img ^
bs=512 ^
seek=1 ^
conv=notrunc




rem kernel 第5扇区

dd if=kernel.bin ^
of=os.img ^
bs=512 ^
seek=5 ^
conv=notrunc





echo.
echo ==========================
echo Build Success
echo ==========================




qemu-system-i386 ^
-drive format=raw,file=os.img




goto end




:error

echo.
echo ==========================
echo Build Failed
echo ==========================

pause
exit /b 1




:end

pause