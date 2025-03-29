from PIL import Image
import os
import glob

# Директории входа и выхода
INPUT_DIR = "src/images/bmp"
OUTPUT_DIR = "src/images/headers"

# Убедимся, что директория для хедеров существует
os.makedirs(OUTPUT_DIR, exist_ok=True)

def convert_image_to_bitmap(image_path):
    img = Image.open(image_path).convert("RGBA")
    width, height = img.size
    pixels = list(img.getdata())

    # Имя файла без расширения
    filename = os.path.splitext(os.path.basename(image_path))[0]
    header_filename = os.path.join(OUTPUT_DIR, f"{filename}.h")

    # Создаём заголовочный файл
    with open(header_filename, "w") as f:
        f.write(f"#ifndef {filename.upper()}_H\n")
        f.write(f"#define {filename.upper()}_H\n\n")
        f.write("#include <stdint.h>\n\n")
        f.write(f"#define {filename.upper()}_WIDTH {width}\n")
        f.write(f"#define {filename.upper()}_HEIGHT {height}\n\n")
        f.write(f"static const uint32_t {filename}[] = {{\n")

        for y in range(height):
            f.write("    ")
            for x in range(width):
                r, g, b, a = pixels[y * width + x]
                color = (a << 24) | (r << 16) | (g << 8) | b  # ARGB
                f.write(f"0x{color:08X}, ")
            f.write("\n")

        f.write("};\n\n")
        f.write("#endif\n")

    print(f"[OK] {header_filename} ({width}x{height})")

# Находим все файлы .bmp, .png, .jpg
image_files = glob.glob(os.path.join(INPUT_DIR, "*.bmp")) + \
              glob.glob(os.path.join(INPUT_DIR, "*.png")) + \
              glob.glob(os.path.join(INPUT_DIR, "*.jpg"))

if not image_files:
    print("[WARN] Нет изображений для обработки в", INPUT_DIR)
else:
    for img in image_files:
        convert_image_to_bitmap(img)
