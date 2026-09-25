"""Convert the boot leaf PNG to LVGL's RGB565+alpha binary image format."""

from pathlib import Path
import struct

from PIL import Image


ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "assets" / "boot" / "pothos-leaf.png"
OUTPUT = ROOT / "spiffs_image" / "intro" / "pothos.bin"
CANVAS_SIZE = 128
LEAF_SIZE = (82, 92)
LV_IMG_CF_TRUE_COLOR_ALPHA = 5


def main() -> None:
    image = Image.open(SOURCE).convert("RGBA")
    alpha = image.getchannel("A")
    image = image.crop(alpha.getbbox())
    image.thumbnail(LEAF_SIZE, Image.Resampling.LANCZOS)

    canvas = Image.new("RGBA", (CANVAS_SIZE, CANVAS_SIZE))
    position = ((CANVAS_SIZE - image.width) // 2, (CANVAS_SIZE - image.height) // 2)
    canvas.alpha_composite(image, position)

    # Artemis is built with LV_COLOR_16_SWAP, so RGB565 bytes are high then low.
    pixels = bytearray()
    for red, green, blue, alpha in canvas.getdata():
        rgb565 = ((red & 0xF8) << 8) | ((green & 0xFC) << 3) | (blue >> 3)
        pixels.extend((rgb565 >> 8, rgb565 & 0xFF, alpha))

    header = LV_IMG_CF_TRUE_COLOR_ALPHA | (CANVAS_SIZE << 10) | (CANVAS_SIZE << 21)
    OUTPUT.write_bytes(struct.pack("<I", header) + pixels)
    print(f"Wrote {OUTPUT} ({OUTPUT.stat().st_size} bytes)")


if __name__ == "__main__":
    main()
