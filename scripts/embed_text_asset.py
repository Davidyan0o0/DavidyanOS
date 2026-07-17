import sys
from pathlib import Path


def escape_c_string(text):
    result = []

    for char in text:
        if char == "\\":
            result.append("\\\\")
        elif char == '"':
            result.append('\\"')
        elif char == "\n":
            result.append("\\n")
        elif char == "\r":
            result.append("\\r")
        elif char == "\t":
            result.append("\\t")
        else:
            code = ord(char)
            if code < 32 or code > 126:
                result.append(f"\\x{code:02x}")
            else:
                result.append(char)

    return "".join(result)


def main():
    if len(sys.argv) != 5:
        print("Usage: python embed_text_asset.py <input> <output> <symbol> <file_name>")
        return 1

    input_path = Path(sys.argv[1])
    output_path = Path(sys.argv[2])
    symbol = sys.argv[3]
    file_name = sys.argv[4]

    text = input_path.read_text(encoding="ascii")
    escaped = escape_c_string(text)

    output_path.parent.mkdir(parents=True,exist_ok=True)
    output_path.write_text(
        "#ifndef RAMFS_ASSETS_H\n"
        "#define RAMFS_ASSETS_H\n\n"
        f"static char {symbol}_name[] = \"{file_name}\";\n"
        f"static unsigned int {symbol}_size = {len(text)};\n"
        f"static char {symbol}_data[] = \"{escaped}\";\n\n"
        "#endif\n",
        encoding="ascii",
    )

    print(f"Embedded {input_path} as {file_name} ({len(text)} bytes)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
