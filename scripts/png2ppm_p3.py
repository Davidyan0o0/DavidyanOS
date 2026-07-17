from PIL import Image
import sys


def png_to_ppm_p3_one_line(input_file, output_file):

    img = Image.open(input_file).convert("RGB")

    width, height = img.size

    with open(output_file, "w") as f:

        # 头部
        f.write(
            f"P3 {width} {height} 255 "
        )

        pixels = img.load()

        # 所有RGB全部写一行
        for y in range(height):
            for x in range(width):

                r, g, b = pixels[x, y]

                f.write(
                    f"{r} {g} {b} "
                )


    print(
        f"Convert success: {width}x{height}"
    )


if __name__ == "__main__":

    if len(sys.argv) != 3:
        print(
            "Usage: python png2ppm.py input.png output.ppm"
        )
        exit(1)

    png_to_ppm_p3_one_line(
        sys.argv[1],
        sys.argv[2]
    )