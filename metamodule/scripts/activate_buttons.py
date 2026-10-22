"""Draw the MetaModule frames of the small on/off buttons by some knobs (ActivateKnobSwitch).

  python3 activate_buttons.py

Writes ../assets/metamodule/activate-TYPE-STATE.png, for each TYPE (power, extended, vintage) and
STATE (off, on), drawn like ActivateKnobSwitch draws them in Rack (src/XTWidgets.h) in the dark
style, without the halo.
"""
import os
from PIL import Image, ImageDraw

HERE = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.join(HERE, '..', 'assets', 'metamodule')

# MetaModule PNG px per mm (240 px is 128.5 mm), and per Rack px (75 per inch)
PX_PER_MM = 240 / 128.5
PX_PER_RACKPX = PX_PER_MM * 25.4 / 75

SIZE_PX = 7  # the 3.5 mm button, rounded up
SS = 32  # supersampling

# XTStyle colors (DARK)
RULER = (82, 82, 82)
LIGHT_OFF = (0x82, 0x82, 0x82)
LIGHT_ON = (114, 234, 101)  # GREEN, the default power button color

BOX_MM = 3.5


def mm(v):
    # mm in the 3.5 mm box to supersampled px, centered in the image
    return (v - BOX_MM / 2) * PX_PER_MM * SS + SIZE_PX * SS / 2


def width(rack_px):
    return max(1, round(rack_px * PX_PER_RACKPX * SS))


def power(d, on):
    c, r = mm(BOX_MM / 2), 1.1 * PX_PER_MM * SS
    d.ellipse((c - r, c - r, c + r, c + r), fill=LIGHT_OFF, outline=RULER, width=width(0.75))
    if on:
        r *= 0.9
        d.ellipse((c - r, c - r, c + r, c + r), fill=LIGHT_ON)


def extended(d, on):
    cross_w, shrink = 0.6, 0.6
    c = BOX_MM / 2
    rects = [(c - cross_w / 2, shrink, c + cross_w / 2, BOX_MM - shrink),
             (shrink, c - cross_w / 2, BOX_MM - shrink, c + cross_w / 2)]
    rad = cross_w / 2 * PX_PER_MM * SS
    for x0, y0, x1, y1 in rects:
        d.rounded_rectangle((mm(x0), mm(y0), mm(x1), mm(y1)), rad, outline=RULER, width=width(1.2))
    for x0, y0, x1, y1 in rects:
        d.rounded_rectangle((mm(x0), mm(y0), mm(x1), mm(y1)), rad, fill=LIGHT_ON if on else LIGHT_OFF)


def vintage(d, on):
    shrink = 0.9
    pts = [(mm(shrink), mm(shrink)), (mm(BOX_MM / 2), mm(BOX_MM - shrink)), (mm(BOX_MM - shrink), mm(shrink))]
    d.line(pts, fill=RULER, width=width(2), joint='curve')
    for x, y in (pts[0], pts[2]):
        r = width(2) / 2
        d.ellipse((x - r, y - r, x + r, y + r), fill=RULER)
    d.line(pts, fill=LIGHT_ON if on else LIGHT_OFF, width=width(1), joint='curve')


def main():
    os.makedirs(OUT, exist_ok=True)
    for name, draw in (('power', power), ('extended', extended), ('vintage', vintage)):
        for state in ('off', 'on'):
            im = Image.new('RGBA', (SIZE_PX * SS, SIZE_PX * SS), (0, 0, 0, 0))
            draw(ImageDraw.Draw(im), state == 'on')
            im = im.resize((SIZE_PX, SIZE_PX), Image.Resampling.BOX)
            im.save(os.path.join(OUT, f'activate-{name}-{state}.png'))


if __name__ == '__main__':
    main()
