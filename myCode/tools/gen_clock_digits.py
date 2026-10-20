"""
生成锁屏大时钟用的数字位图（0-9 与冒号）。

为什么不用字体？
    CanMV 那份 MicroPython 代码用 lv.font_load() 在运行时加载 150px 的二进制字库，
    而 PC 上的 LVGL v8 C 版本没有运行时字库加载能力，内置的 Montserrat 最大只到 48px。
    所以这里用 Pillow 把标题级大字号预先渲染成 PNG，运行时当作普通图片用
    （见 myCode/GUI/big_clock.c，图片放在 myCode/Fonts/）。

约定：
    * 所有图片高度一致（= ascent + descent），基线位置一致，
      这样把它们放进一个横向 flex 容器里，纵向自动对齐，不需要额外计算。
    * 字形是"纯白 + alpha"，运行时用 lv_obj_set_style_img_recolor() 上色，
      深色模式变白、浅色模式变黑都可以。
    * 每个数字的单元格宽度一致（tabular），秒变化时数字不会左右抖动。

用法：
    python gen_clock_digits.py [输出目录] [字号] [字体文件]
"""
import os
import sys

from PIL import Image, ImageDraw, ImageFont

DEFAULT_OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "Fonts")
DEFAULT_SIZE = 150
DEFAULT_FONT = r"C:\Windows\Fonts\seguisb.ttf"  # Segoe UI Semibold


def main() -> int:
    out_dir = sys.argv[1] if len(sys.argv) > 1 else DEFAULT_OUT
    size = int(sys.argv[2]) if len(sys.argv) > 2 else DEFAULT_SIZE
    font_path = sys.argv[3] if len(sys.argv) > 3 else DEFAULT_FONT

    out_dir = os.path.abspath(out_dir)
    os.makedirs(out_dir, exist_ok=True)

    font = ImageFont.truetype(font_path, size)
    ascent, descent = font.getmetrics()
    height = ascent + descent

    # 数字用等宽单元格：取 0-9 的最大推进宽度
    digit_adv = max(font.getlength(str(d)) for d in range(10))
    cell_w = int(round(digit_adv))
    colon_adv = max(1, int(round(font.getlength(":"))))

    print(f"font={font_path} size={size} ascent={ascent} descent={descent} "
          f"height={height} digit_w={cell_w} colon_w={colon_adv}")

    def render(ch, width, name):
        img = Image.new("RGBA", (width, height), (255, 255, 255, 0))
        draw = ImageDraw.Draw(img)
        # anchor="ms" = 水平居中 + 基线对齐，横向居中、纵向按基线摆放
        draw.text((width / 2.0, ascent), ch, font=font, fill=(255, 255, 255, 255), anchor="ms")
        path = os.path.join(out_dir, name)
        img.save(path)
        return path

    rendered = []
    for d in range(10):
        rendered.append(render(str(d), cell_w, f"clock_digit_{d}.png"))
    rendered.append(render(":", colon_adv, "clock_colon.png"))

    for p in rendered:
        print(f"  wrote {os.path.basename(p)} ({os.path.getsize(p)} bytes)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
