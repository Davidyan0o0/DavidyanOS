from PIL import Image
import sys


def png_to_ppm(input_file, output_file):
    # 打开 PNG
    img = Image.open(input_file)

    # 转换为 RGB
    img = img.convert("RGB")

    width, height = img.size

    # 写入 PPM(P6)
    with open(output_file, "wb") as f:

        # PPM 文件头
        f.write(
            f"P6\n{width} {height}\n255\n".encode()
        )

        # 写入 RGB 数据
        f.write(img.tobytes())


    print(f"Convert success: {width}x{height}")
    print(output_file)



if __name__ == "__main__":

    if len(sys.argv) != 3:
        print(
            "Usage: python png2ppm.py input.png output.ppm"
        )
        sys.exit(1)


    png_to_ppm(
        sys.argv[1],
        sys.argv[2]
    )