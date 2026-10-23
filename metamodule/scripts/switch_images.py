"""Draw the MetaModule images of the switches that Rack draws with code (src/XTWidgets.h).

  python3 switch_images.py

Writes, in ../assets/metamodule/, drawn like Rack draws them in the dark style, without the halo:
  - activate-TYPE-STATE.png: the frames of the small on/off buttons by some knobs
    (ActivateKnobSwitch), for each TYPE (power, extended, vintage) and STATE (off, on)
  - four-lights-bg.png, four-lights-handle.png: the slide switch of Reverb's room shape
    (ThereAreFourLights): a ring per position, and the lit ring that the MetaModule moves over them
"""
import os
from PIL import Image, ImageDraw

HERE = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.join(HERE, '..', 'assets', 'metamodule')

# MetaModule PNG px per mm (240 px is 128.5 mm), and per Rack px (75 per inch)
PX_PER_MM = 240 / 128.5
PX_PER_RACKPX = PX_PER_MM * 25.4 / 75

SS = 32  # supersampling

# XTStyle colors (DARK)
RULER = (82, 82, 82)
LIGHT_OFF = (0x82, 0x82, 0x82)
LIGHT_ON = (114, 234, 101)  # GREEN, the default power button color
VALUE = (0xFF, 0x90, 0x00)  # ORANGE, the default control value color

# The buttons are 3.5 mm, rounded up to 7 px
BUTTON_PX = 7
BUTTON_MM = 3.5

# ThereAreFourLights rings, each centered in a slot of the slide switch (the MetaModule divides it
# into equal slots, one per position)
RING_MM = 2.6
SLOT_PX = 7


def width(rack_px):
    return max(1, round(rack_px * PX_PER_RACKPX * SS))


def button_mm(v):
    # mm in the button's box to supersampled px, centered in the image
    return (v - BUTTON_MM / 2) * PX_PER_MM * SS + BUTTON_PX * SS / 2


def power(d, on):
    c, r = button_mm(BUTTON_MM / 2), 1.1 * PX_PER_MM * SS
    d.ellipse((c - r, c - r, c + r, c + r), fill=LIGHT_OFF, outline=RULER, width=width(0.75))
    if on:
        r *= 0.9
        d.ellipse((c - r, c - r, c + r, c + r), fill=LIGHT_ON)


def extended(d, on):
    cross_w, shrink = 0.6, 0.6
    c = BUTTON_MM / 2
    rects = [(c - cross_w / 2, shrink, c + cross_w / 2, BUTTON_MM - shrink),
             (shrink, c - cross_w / 2, BUTTON_MM - shrink, c + cross_w / 2)]
    rad = cross_w / 2 * PX_PER_MM * SS
    m = button_mm
    for x0, y0, x1, y1 in rects:
        d.rounded_rectangle((m(x0), m(y0), m(x1), m(y1)), rad, outline=RULER, width=width(1.2))
    for x0, y0, x1, y1 in rects:
        d.rounded_rectangle((m(x0), m(y0), m(x1), m(y1)), rad, fill=LIGHT_ON if on else LIGHT_OFF)


def vintage(d, on):
    shrink = 0.9
    m = button_mm
    pts = [(m(shrink), m(shrink)), (m(BUTTON_MM / 2), m(BUTTON_MM - shrink)), (m(BUTTON_MM - shrink), m(shrink))]
    d.line(pts, fill=RULER, width=width(2), joint='curve')
    for x, y in (pts[0], pts[2]):
        r = width(2) / 2
        d.ellipse((x - r, y - r, x + r, y + r), fill=RULER)
    d.line(pts, fill=LIGHT_ON if on else LIGHT_OFF, width=width(1), joint='curve')


def ring(d, slot, fill):
    c, r = SLOT_PX * SS / 2, RING_MM / 2 * PX_PER_MM * SS
    cy = (slot + 0.5) * SLOT_PX * SS
    d.ellipse((c - r, cy - r, c + r, cy + r), fill=fill, outline=RULER, width=width(1))


def save(name, size, draw):
    w, h = size
    im = Image.new('RGBA', (w * SS, h * SS), (0, 0, 0, 0))
    draw(ImageDraw.Draw(im))
    im.resize(size, Image.Resampling.BOX).save(os.path.join(OUT, name + '.png'))


def main():
    os.makedirs(OUT, exist_ok=True)
    for name, draw in (('power', power), ('extended', extended), ('vintage', vintage)):
        for state in ('off', 'on'):
            save(f'activate-{name}-{state}', (BUTTON_PX, BUTTON_PX), lambda d: draw(d, state == 'on'))

    save('four-lights-bg', (SLOT_PX, SLOT_PX * 4), lambda d: [ring(d, i, LIGHT_OFF) for i in range(4)])
    save('four-lights-handle', (SLOT_PX, SLOT_PX), lambda d: ring(d, 0, VALUE))


if __name__ == '__main__':
    main()
