"""Float32 WAV helpers for the headless simulator.
  gen OUT SECONDS FREQ_L FREQ_R AMP     - write a stereo sine
  stats FILE [SKIP_SECONDS]             - per-channel RMS / peak / NaN count / DC
  diff A B                              - RMS of the difference
"""
import math, struct, sys

def write(path, frames, sr=48000):
    n = len(frames)
    with open(path, 'wb') as f:
        f.write(b'RIFF' + struct.pack('<I', 36 + 2 + n * 8) + b'WAVEfmt ')
        f.write(struct.pack('<IHHIIHHH', 0x12, 3, 2, sr, sr * 8, 8, 32, 0))
        f.write(b'data' + struct.pack('<I', n * 8))
        f.write(b''.join(struct.pack('<ff', l, r) for l, r in frames))

def read(path):
    d = open(path, 'rb').read()
    i = d.index(b'data') + 8
    v = struct.unpack('<%df' % ((len(d) - i) // 4), d[i:i + ((len(d) - i) // 4) * 4])
    return list(zip(v[0::2], v[1::2]))

cmd = sys.argv[1]
if cmd == 'gen':
    out, secs, fl, fr, amp = sys.argv[2], float(sys.argv[3]), float(sys.argv[4]), float(sys.argv[5]), float(sys.argv[6])
    sr = 48000
    write(out, [(amp * math.sin(2 * math.pi * fl * i / sr), amp * math.sin(2 * math.pi * fr * i / sr)) for i in range(int(secs * sr))])
elif cmd == 'stats':
    fr = read(sys.argv[2])
    skip = int(float(sys.argv[3]) * 48000) if len(sys.argv) > 3 else 0
    fr = fr[skip:]
    for c in range(2):
        x = [f[c] for f in fr]
        nan = sum(1 for v in x if v != v)
        y = [v for v in x if v == v]
        rms = math.sqrt(sum(v * v for v in y) / max(1, len(y)))
        print(f'ch{c}: n={len(x)} rms={rms:.5f} peak={max(abs(v) for v in y) if y else 0:.5f} dc={sum(y)/max(1,len(y)):.5f} nan={nan}')
elif cmd == 'diff':
    a, b = read(sys.argv[2]), read(sys.argv[3])
    n = min(len(a), len(b))
    for c in range(2):
        print(f'ch{c}: rms diff={math.sqrt(sum((a[i][c]-b[i][c])**2 for i in range(n))/n):.5f}')
