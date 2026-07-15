@echo off

echo =====================
echo Building DavidyanOS
echo =====================

del boot.bin
del loader.bin
del os.img

nasm boot.asm -f bin -o boot.bin

if errorlevel 1 (
    echo boot.asm compile failed!
    pause
    exit /b
)


nasm loader.asm -f bin -o loader.bin

if errorlevel 1 (
    echo loader.asm compile failed!
    pause
    exit /b
)

nasm kernel.asm -f bin -o kernel.bin

if errorlevel 1 (
    echo kernel.asm compile failed!
    pause
    exit /b
)


rem 创建1.44MB软盘镜像

fsutil file createnew os.img 1474560



rem 写boot到第0扇区

dd if=boot.bin of=os.img bs=512 count=1 conv=notrunc



rem 写入loader到第1扇区

dd if=loader.bin of=os.img bs=512 seek=1 conv=notrunc


rem 写入kernel到第5扇区

dd if=kernel.bin of=os.img seek=5 conv=notrunc



echo.
echo =====================
echo Build Success!
echo =====================

pause