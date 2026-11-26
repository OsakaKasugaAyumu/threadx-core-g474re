#!/usr/bin/env python3
"""Windows tutorial: build the pinned NUCLEO-G474RE demo from threadx_core only.
No programmer, old build tree, original main/ sources, network or non-stdlib modules.
"""
import argparse
from concurrent.futures import ThreadPoolExecutor
import hashlib
import json
from pathlib import Path
import re
import struct
import sys

ROOT = Path(__file__).resolve().parents[3]
sys.path.insert(0, str(ROOT / 'tools'))
from build_profile import CPU, CFLAGS, ENTRIES, check, run, sha
from build_nucleo_g474re import START, SYSTEM, LINK, LOCK, FLASH, PREFIX, ihex_image

C5_BIN = 'c0f9fd038cadc423d0768cf7d126de85cbddc35019989d0584260f8d064d9674'
PORTABLE_BIN = '07fd53b3a69a61442d3082700eaeb7ed62c705b2b751933ed58f9ebae60654ff'
# No private directory in a published recipe: recognize the historical __FILE__ prefix by hash.
C5_PREFIX_ID = 'b99efac0f869a9117f2d01909d5e4ead2e66ea1c4f65b05034b2afe75e52859c'
PUBLIC_PREFIX = 'threadx-learn/src/v6.2.0/cortex-m4f/gnu/os'  # same 42-byte width
COMPILER = '(GNU Tools for STM32 14.3.rel1.20251027-0700) 14.3.1 20250623'
SKIP = {'cortexm4_crt0.s', 'cortexm4_vectors.S', 'tx_initialize_low_level.S'}

def digest(data):
    return hashlib.sha256(data).hexdigest()

def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--gcc', type=Path, required=True, help='path to arm-none-eabi-gcc.exe')
    ap.add_argument('--cube-root', type=Path, required=True, help='X-CUBE-AZRTOS-G4 v2.0.0 root')
    ap.add_argument('--out', type=Path, required=True, help='new, not already existing, build directory')
    ap.add_argument('--portable', action='store_true', help='always use public diagnostic file names')
    args = ap.parse_args()
    gcc = args.gcc.resolve(strict=True); cube = args.cube_root.resolve(strict=True)
    core = ROOT / 'threadx_core'; glue = ROOT / 'boards/nucleo-g474re/board_threadx.c'
    manifest = json.loads((ROOT / 'reports/core-c2/manifest.json').read_bytes())
    files = {f['name']: f['after']['sha256'] for f in manifest['files']}
    check(len(files) == 215 and {p.name for p in core.iterdir() if p.is_file()} == set(files), 'Wrong core file set')
    for name, expected in files.items():
        check(sha(core / name) == expected, 'Changed threadx_core input (including EOL): ' + name)
    for p, expected in LOCK.items():
        check(digest((cube / p).read_bytes().replace(b'\r\n', b'\n')) == expected, 'Wrong ST input: ' + str(p))
    check(digest(glue.read_bytes().replace(b'\r\n', b'\n')) == 'bbc2210204899dc416f3fe4963c067f6c1881c97fd068d2153bc3e75cd583a27', 'Changed board glue')
    check(run([gcc, '-dumpmachine'], ROOT).strip() == 'arm-none-eabi' and
          run([gcc, '--version'], ROOT).splitlines()[0].endswith(COMPILER), 'Select the pinned ST GCC: ' + COMPILER)
    out = args.out.resolve(); out.mkdir(parents=True, exist_ok=False)
    (out / '.gitignore').write_text('*\n', encoding='ascii'); obj = out / 'obj'; obj.mkdir()
    def cmd(*argv): return run(argv, out)
    def tool(name): return gcc.with_name('arm-none-eabi-' + name + '.exe')
    def objfile(p): return obj / (p.name + '.o')
    logical = str(ROOT / 'main')  # diagnostic text only; main/ is never opened
    historical = not args.portable and digest(logical.encode()) == C5_PREFIX_ID
    mapped = '-fmacro-prefix-map=' + str(core) + '=' + (logical if historical else PUBLIC_PREFIX)
    asm = [*CPU, '-I', core, '-g', '-x', 'assembler-with-cpp', mapped]
    # In ST GCC 14.3 the aliases for the same BX LR need Thumb function metadata, not new instructions.
    raw = out / 'misra.raw.o'; inc = out / 'misra-thumb-types.inc'
    cmd(gcc, *asm, '-c', core / 'tx_misra.S', '-o', raw)
    def symbols(p):
        return {n:(int(a, 16), t, s) for a,t,s,n in re.findall(
            r'^\s*\d+:\s+([\da-fA-F]+)\s+\d+\s+(\w+)\s+GLOBAL\s+DEFAULT\s+(\d+)\s+(_tx_misra_\w+)$',
            cmd(tool('readelf'), '-Ws', p), re.M)}
    orig = symbols(raw); addr, kind, sec = orig['_tx_misra_pointer_to_ulong_convert']
    aliases = sorted(n for n, a in orig.items() if a[1] == 'NOTYPE')
    check(kind == 'FUNC' and addr & 1 and len(aliases) == 31 and
          all(orig[n] == (addr & ~1, 'NOTYPE', sec) for n in aliases), 'Unexpected Thumb aliases')
    inc.write_text(''.join('.type ' + n + ', %function\n' for n in aliases), encoding='ascii')
    sources = [core / n for n in sorted(files) if Path(n).suffix in {'.c', '.S', '.s'} and n not in SKIP]
    st = [cube / START, cube / SYSTEM, glue]
    def compile_one(p):
        if p == st[0]: flags = [*CPU, '-g', '-x', 'assembler']
        elif p.parent == core:
            flags = [*CPU, '-I', core, *CFLAGS, mapped] if p.suffix == '.c' else list(asm)
            if p.name == 'tx_misra.S': flags += ['-Xassembler', inc]
        else:
            flags = [*CPU, *CFLAGS, '-DSTM32G474xx', '-DUSER_VECT_TAB_ADDRESS', '-I', core,
                     '-I', cube / 'Drivers/CMSIS/Include', '-I', cube / 'Drivers/CMSIS/Device/ST/STM32G4xx/Include', mapped]
        argv = [str(a) for a in [gcc, *flags, '-c', p, '-o', objfile(p)]]; cmd(*argv)
        return dict(directory=str(out), file=str(p), arguments=argv, output=str(objfile(p)))
    with ThreadPoolExecutor(max_workers=6) as pool: db = list(pool.map(compile_one, sources + st))
    (out / 'compile_commands.json').write_text(json.dumps(db, indent=2), encoding='utf8')
    check(symbols(objfile(core / 'tx_misra.S')) == {n: (addr, kind, sec) if n in aliases else a for n,a in orig.items()}, 'Bad Thumb alias fixup')
    members = [objfile(p) for p in sources if p.name not in {'demo_threadx.c', 'tx_misra.c'}]
    # Avoid the Windows command-length limit; sorted archive order is part of the link recipe.
    (out / 'archive.rsp').write_bytes(''.join('"' + p.as_posix() + '"\n' for p in members).encode())
    archive = out / 'libthreadx.a'; cmd(tool('ar'), 'rcs', archive, '@archive.rsp')
    check(len(members) == 194 and cmd(tool('ar'), 't', archive).splitlines() == [p.name for p in members], 'Wrong archive')
    def crt(name): return Path(cmd(gcc, *CPU, '-print-file-name=' + name).strip()).resolve(strict=True)
    elf = out / (PREFIX + '.elf'); mp = out / (PREFIX + '.map')
    cmd(gcc, *CPU, '-nostartfiles', '-nostdlib', '-T', cube / LINK, '-Wl,--gc-sections,--no-undefined',
        '-Wl,-Map=' + str(mp), crt('crti.o'), *(objfile(p) for p in st), objfile(core/'demo_threadx.c'),
        '-Wl,--start-group', archive, '-lc', '-lgcc', '-Wl,--end-group', crt('crtn.o'), '-o', elf)
    bf = out / (PREFIX + '.bin'); hx = out / (PREFIX + '.hex')
    for fmt, p in [('binary', bf), ('ihex', hx)]: cmd(tool('objcopy'), '-O', fmt, elf, p)
    listing = cmd(tool('nm'), '-n', '--defined-only', elf); (out / 'symbols.txt').write_text(listing, encoding='utf8')
    syms = {n:int(a,16) for a,n in re.findall(r'^([0-9a-fA-F]+)\s+\w\s+(\w+)$', listing, re.M)}
    check(all(n in syms for n in ENTRIES) and not cmd(tool('nm'), '-u', elf).strip(), 'Missing ELF entry or undefined symbol')
    image = bf.read_bytes(); mem = ihex_image(hx); h = digest(image)
    check(len(image) == 14912 and max(mem) == FLASH+len(image)-1 and
          image == bytes(mem.get(FLASH+i, 0) for i in range(len(image))), 'HEX and BIN disagree')
    vectors = struct.unpack_from('<16I', image)
    check(vectors[0] == 0x20020000 and [vectors[i] for i in (1,14,15)] ==
          [syms[n] | 1 for n in ('Reset_Handler','PendSV_Handler','SysTick_Handler')], 'Wrong vectors')
    expected = C5_BIN if historical else PORTABLE_BIN
    check(h == expected, 'Unrecognized flash image. Do not program. SHA-256: ' + h)
    print('PASS |', 'C5 image' if historical else 'portable image (not the C5 board run)', '| 14912 bytes | ' + h)
    print(elf); print(bf)

if __name__ == '__main__':
    try: main()
    except (OSError, ValueError, RuntimeError, KeyError) as exc: raise SystemExit(str(exc))
