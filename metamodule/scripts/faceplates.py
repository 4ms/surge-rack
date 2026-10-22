"""Make the MetaModule faceplate PNGs from the Rack plugin's panels.

  python3 faceplates.py RACK_PLUGIN_DIR [SLUG ...]

RACK_PLUGIN_DIR is an installed Rack build of this plugin (the folder with plugin.json and
plugin.dylib). The script runs VCV Rack's screenshot mode on it, with a user folder of its own
and SURGEXT_PANEL_ONLY set, so the modules draw only their panel art and the labels that don't
change (see XTModuleWidget::panelOnly). Then it scales each screenshot to the MetaModule's
faceplate height, dithers it to the MetaModule's RGB565 colors (or the panel's gradient shows as
tinted bands), and writes it to ../assets/faceplates/SLUG.png.
"""
import argparse, json, os, shutil, subprocess, sys, tempfile
import numpy as np
from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
PLUGIN_MM_JSON = os.path.join(HERE, '..', 'plugin-mm.json')
FACEPLATES = os.path.join(HERE, '..', 'assets', 'faceplates')

RACK = '/Applications/VCV Rack 2 Free.app/Contents/MacOS/Rack'
PLUGINS_DIR = 'plugins-mac-arm64'
DARK_STYLE = 10001  # XTStyle::DARK, the MetaModule's style

FACEPLATE_HEIGHT = 240
# Rack draws the screenshots at this zoom, so they're scaled down to the faceplate height
ZOOM = 4


def screenshot(plugin_dir, user_dir, rack):
    os.makedirs(os.path.join(user_dir, PLUGINS_DIR), exist_ok=True)
    link = os.path.join(user_dir, PLUGINS_DIR, 'SurgeXTRack')
    if not os.path.exists(link):
        os.symlink(os.path.abspath(plugin_dir), link)

    os.makedirs(os.path.join(user_dir, 'SurgeXTRack'), exist_ok=True)
    with open(os.path.join(user_dir, 'SurgeXTRack', 'default-skin.json'), 'w') as f:
        json.dump({'defaultSkin': DARK_STYLE}, f)

    shutil.rmtree(os.path.join(user_dir, 'screenshots'), ignore_errors=True)
    env = dict(os.environ, SURGEXT_PANEL_ONLY='1')
    subprocess.run([rack, '-u', user_dir, '-t', str(ZOOM)], env=env, check=True,
                   stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    return os.path.join(user_dir, 'screenshots', 'SurgeXTRack')


BAYER_4X4 = np.array([[0, 8, 2, 10], [12, 4, 14, 6], [3, 11, 1, 9], [15, 7, 13, 5]]) / 16 - 0.5


def dither_rgb565(im):
    rgb = np.asarray(im.convert('RGB'), dtype=float)
    h, w, _ = rgb.shape
    threshold = np.tile(BAYER_4X4, (h // 4 + 1, w // 4 + 1))[:h, :w]
    for c, bits in enumerate((5, 6, 5)):
        levels = 2**bits - 1
        rgb[:, :, c] = np.clip(np.round(rgb[:, :, c] * levels / 255 + threshold), 0, levels) * 255 / levels
    return Image.fromarray(np.round(rgb).astype(np.uint8))


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('plugin_dir')
    ap.add_argument('slugs', nargs='*')
    ap.add_argument('--rack', default=RACK)
    ap.add_argument('--user-dir', help='Rack user folder to use (default: a temporary one)')
    args = ap.parse_args()

    with open(PLUGIN_MM_JSON) as f:
        slugs = [m['slug'] for m in json.load(f)['MetaModuleIncludedModules']]
    if args.slugs:
        slugs = [s for s in slugs if s in args.slugs]

    user_dir = args.user_dir or tempfile.mkdtemp(prefix='surgext-faceplates-')
    shots = screenshot(args.plugin_dir, user_dir, args.rack)

    for slug in slugs:
        path = os.path.join(shots, slug + '.png')
        if not os.path.exists(path):
            sys.exit(f'{slug}: no screenshot at {path}')
        im = Image.open(path).convert('RGB')
        # Rack panels are 380 px (128.5 mm) tall and 15 px (5.08 mm) per HP, which differ slightly
        hp = round(im.width / im.height * 380 / 15)
        width = round(hp * 5.08 / 128.5 * FACEPLATE_HEIGHT)
        im = im.resize((width, FACEPLATE_HEIGHT), Image.Resampling.LANCZOS)
        dither_rgb565(im).save(os.path.join(FACEPLATES, slug + '.png'))
        print(f'{slug}: {width} x {FACEPLATE_HEIGHT}')


if __name__ == '__main__':
    main()
