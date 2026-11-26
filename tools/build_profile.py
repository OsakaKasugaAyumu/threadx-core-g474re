#!/usr/bin/env python3
"""Build + verify the #215 learning profile. Python 3.9+, GNU arm-none-eabi, no CMake or IDE needed."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
from concurrent.futures import ThreadPoolExecutor

ROOT = Path(__file__).resolve().parents[1]
SRC = ROOT / 'main'
BUILD = ROOT / 'build' / 'gnu-m4f'
HEADER = SRC / 'autoconf.h'
LOCK214 = '7bee4913bec7bef8607f9b597ededbbbf6eb6c2f2bfa43e43622a21af149aa3c'
CPU = ['-mcpu=cortex-m4', '-mthumb', '-mfpu=fpv4-sp-d16', '-mfloat-abi=hard']
CFLAGS = ['-std=c11', '-O1', '-g', '-ffunction-sections', '-fdata-sections', '-fno-common']
# Independent acceptance expectations; these are never passed to the compiler as -D.
OFF = '''TX_DISABLE_ERROR_CHECKING TX_ENABLE_MULTI_ERROR_CHECKING TX_NO_TIMER TX_TIMER_PROCESS_IN_ISR
TX_DISABLE_STACK_FILLING TX_ENABLE_STACK_CHECKING TX_DISABLE_NOTIFY_CALLBACKS TX_DISABLE_REDUNDANT_CLEARING
TX_DISABLE_PREEMPTION_THRESHOLD TX_INLINE_THREAD_RESUME_SUSPEND TX_NOT_INTERRUPTABLE TX_REACTIVATE_INLINE
TX_NO_FILEX_POINTER TX_ENABLE_EVENT_TRACE TX_ENABLE_EVENT_LOGGING TX_EXECUTION_PROFILE_ENABLE
TX_ENABLE_EXECUTION_CHANGE_NOTIFY TX_ENABLE_WFI TX_LOW_POWER TX_PORT_USE_BASEPRI TX_PORT_BASEPRI
TX_ENABLE_IAR_LIBRARY_SUPPORT TX_64_BIT'''.split() + ['TX_'+x+'_ENABLE_PERFORMANCE_INFO' for x in
    'BLOCK_POOL BYTE_POOL EVENT_FLAGS MUTEX QUEUE SEMAPHORE THREAD TIMER'.split()]
ENTRIES = ['thread_'+x+'_entry' for x in ('0','1','2','3_and_4','5','6_and_7')]

def fail(msg):
    raise RuntimeError(msg)

def check(cond, msg):
    if not cond: fail(msg)

def sha(p):
    return hashlib.sha256(p.read_bytes()).hexdigest()

def writej(p, data):
    p.write_text(json.dumps(data, indent=2, ensure_ascii=True)+'\n', encoding='utf-8')

def inputs():
    ps = sorted((p for p in SRC.iterdir() if p.is_file() and p != HEADER), key=lambda p:p.name)
    # Only a single exact directive may replace the first original empty line, with its EOL untouched.
    # Undo this reversible overlay in memory before checking the original, case-sensitive byte lock.
    inc = b'#include "autoconf.h"'; original = []
    for p in ps:
        data = p.read_bytes(); match = re.search(rb'^' + re.escape(inc) + rb'(?=\r?\n)', data, re.M)
        check(match and data.count(inc)==1, 'Missing or duplicate exact include: '+p.name)
        raw = data[:match.start()] + data[match.end():]
        check(re.search(rb'^(?=\r?\n)', raw, re.M).start()==match.start(), 'Moved include: '+p.name)
        original.append(p.name+' '+hashlib.sha256(raw).hexdigest()+'\n')
    check(len(ps)==214 and hashlib.sha256(''.join(original).encode()).hexdigest()==LOCK214,
          'Original 214 byte lock failed after undoing the include (check source and EOL).')
    return ps

def env():
    # No per-TU flags or include path injection from a shell/IDE.
    return {k:v for k,v in os.environ.items() if k.upper() not in {
        'CPATH','C_INCLUDE_PATH','CPLUS_INCLUDE_PATH','OBJC_INCLUDE_PATH','GCC_EXEC_PREFIX',
        'COMPILER_PATH','LIBRARY_PATH','DEPENDENCIES_OUTPUT','SUNPRO_DEPENDENCIES'}}

def run(cmd, cwd, inp=None, reject=None):
    cmd = list(map(str,cmd))
    result = subprocess.run(cmd, cwd=cwd, env=env(), input=inp, text=True, encoding='utf-8',
                            errors='replace', stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    if reject is not None:
        check(result.returncode != 0 and reject in result.stderr, 'Negative test did not fail as expected: '+str(cmd))
    elif result.returncode:
        fail(subprocess.list2cmdline(cmd)+'\n'+result.stdout+result.stderr)
    elif result.stderr.strip():
        print(result.stderr.rstrip(),file=sys.stderr)  # Never silently hide a successful tool warning.
    return result.stdout

def predefs(text):
    return dict(re.findall(r'^#define (\w+)(?:[ \t]+(.*))?$',text,re.M))

def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--gcc',default=os.environ.get('THREADX_ARM_GCC'), help='exact GCC executable or name on PATH')
    ap.add_argument('-j','--jobs',type=int,default=min(8,os.cpu_count() or 1)); args=ap.parse_args()
    found=shutil.which(args.gcc or 'arm-none-eabi-gcc')
    if not found and not args.gcc:
        local=(ROOT.parent/'threadx_build'/'tools'/'STM32CubeIDE'/'plugins').glob('*.gnu-tools-for-stm32.*/tools/bin/arm-none-eabi-gcc.exe')
        candidates=list(local); found=str(candidates[0]) if len(candidates)==1 else None
    check(found, 'Set THREADX_ARM_GCC or supply --gcc <path to arm-none-eabi-gcc>.')
    gcc=Path(found).resolve(); ps=inputs(); sources=[p for p in ps if p.suffix in {'.c','.S','.s'}]
    objdir=BUILD/'obj'; objdir.mkdir(parents=True, exist_ok=True)
    ok=BUILD/'verified.json'; ok.unlink(missing_ok=True)  # A failed rerun must not look verified.
    def cmd(*items, **kw): return run(items, BUILD, **kw)
    def tool(name):
        p=gcc.with_name('arm-none-eabi-'+name+gcc.suffix); check(p.is_file(), 'Missing matching tool: '+str(p)); return p
    check(cmd(gcc,'-dumpmachine').strip()=='arm-none-eabi','Wrong GNU target')
    version=cmd(gcc,'--version').splitlines()[0]; selected={
        m[1]:m[2] for m in re.finditer(r'^#define (TX_\w+)\s+(\w+)',HEADER.read_text(encoding='utf-8'),re.M)}
    check(selected.get('TX_MISRA_ENABLE')=='1' and selected.get('TX_SAFETY_CRITICAL')=='1' and len(selected)==8,'Unexpected profile contract')
    low=(SRC/'tx_initialize_low_level.S').read_text(encoding='utf-8')
    tick=re.search(r'SYSTICK_CYCLES\s*=\s*\(\(SYSTEM_CLOCK / (\d+)\) -1\)',low)
    check(tick and int(selected['TX_TIMER_TICKS_PER_SECOND'].rstrip('ULul'))==int(tick[1]),
          'Software tick rate differs from the fixed startup assembly divisor')
    def obj(p): return objdir/(p.name+'.o')  # tx_misra.c and tx_misra.S are not the same object!
    # The 6.2.0 port stacks Thumb entry labels before ONE bx lr. GNU as can mark
    # only the first label FUNC; ld rightly rejects Thumb calls to the NOTYPE aliases.
    # Supply symbol TYPE declarations (no instructions, branches, stubs or renamed symbols).
    misra=SRC/'tx_misra.S'; raw=BUILD/'misra.raw.o'; inc=BUILD/'misra-thumb-types.inc'
    cmd(gcc,*CPU,'-I',SRC,'-g','-x','assembler-with-cpp','-c',misra,'-o',raw)
    def misra_symbols(o):
        return {n:(int(addr,16),typ,sec) for addr,typ,sec,n in re.findall(
            r'^\s*\d+:\s+([0-9a-fA-F]+)\s+\d+\s+(\w+)\s+GLOBAL\s+DEFAULT\s+(\d+)\s+(_tx_misra_\w+)$',
            cmd(tool('readelf'),'-Ws',o),re.M)}
    original=misra_symbols(raw); anchor=original['_tx_misra_pointer_to_ulong_convert']; addr=anchor[0]&~1
    aliases=sorted(n for n,(a,t,section) in original.items() if t=='NOTYPE')
    check(anchor[1]=='FUNC' and anchor[0]&1 and len(aliases) in (0,31) and
          all(original[n]==(addr,'NOTYPE',anchor[2]) for n in aliases), 'Unexpected GNU alias set; review the exact port')
    check(re.search(r'^\s*'+format(addr,'x')+r':\s+4770\s+bx\s+lr\b',cmd(tool('objdump'),'-d',raw),re.M), 'Not the shared BX LR entry')
    inc.write_text(''.join('.type '+n+', %function\n' for n in aliases),encoding='ascii')
    def compile_one(p):
        flags=[str(gcc),*CPU,'-I',str(SRC)]
        flags += CFLAGS if p.suffix=='.c' else ['-g','-x','assembler-with-cpp']
        if p==misra and aliases: flags+=['-Xassembler',str(inc)]
        full=[*flags,'-c',str(p),'-o',str(obj(p))]; cmd(*full)
        snap=predefs(cmd(*flags,'-E','-dM',p))
        check(all(snap.get(k)==v for k,v in selected.items()) and not (set(OFF)&snap.keys()) and
                  snap.get('__ARM_FP')=='4' and snap.get('__ARM_PCS_VFP')=='1', 'Macro/ABI divergence: '+p.name)
        if p.suffix=='.c': check('TX_DISABLE_INLINE' in snap and 'TX_INLINE_INITIALIZATION' not in snap, 'Wrong C port: '+p.name)
        if p.name in ('tx_misra.c','tx_thread_initialize.c'):
            check(('TX_THREAD_INIT' in snap)==(p.name=='tx_misra.c'),'Early MISRA branch failed: '+p.name)
        return dict(directory=str(BUILD),file=str(p),arguments=full,output=str(obj(p)))
    with ThreadPoolExecutor(max_workers=max(1,args.jobs)) as pool: db=list(pool.map(compile_one,sources))
    final=misra_symbols(obj(misra)); check(final.keys()==original.keys() and all(
        final[n]==(anchor if n in aliases else original[n]) for n in original),'Unexpected symbol change')
    section_hashes={}
    for section in ('.text','.data'):
        a=BUILD/('misra.raw'+section+'.bin'); b=BUILD/('misra.final'+section+'.bin')
        for source,dest in ((raw,a),(obj(misra),b)): cmd(tool('objcopy'),'-j',section,'-O','binary',source,dest)
        check(a.read_bytes()==b.read_bytes(), 'GAS metadata changed machine bytes: '+section); section_hashes[section]=sha(b)
    writej(BUILD/'compile_commands.json',db)
    # Use ONE complete provider in the link. Compile portable tx_misra.c for branch reading,
    # but never archive/link it together with tx_misra.S (overlapping strong definitions).
    direct={'demo_threadx.c','cortexm4_vectors.S','tx_initialize_low_level.S','cortexm4_crt0.s'}
    kernel=[p for p in sources if p.name not in direct|{'tx_misra.c'}]
    archive=BUILD/'libthreadx.a'; elf=BUILD/'demo_threadx.elf'; mapfile=BUILD/'demo_threadx.map'
    archive.unlink(missing_ok=True)  # ar r does not evict stale members on its own
    def rsp(path,items):
        # GNU response files: forward slashes keep Windows paths readable by libiberty.
        path.write_text(''.join('"'+str(i).replace('\\','/').replace('"','\\"')+'"\n' for i in items),encoding='utf-8')
    rsp(BUILD/'archive.rsp',[obj(p) for p in kernel]); cmd(tool('ar'),'rcs',archive,'@archive.rsp')
    members=cmd(tool('ar'),'t',archive).splitlines(); check(set(members)=={obj(p).name for p in kernel} and len(members)==len(kernel),'Archive contents differ')
    # The generic link script is itself one of the 214 reading files; GNU ld does not run CPP.
    ld_src=SRC/'demo_threadx.crt0.ld'; ld=BUILD/'demo_threadx.preprocessed.ld'
    ld_flags=[str(gcc),*CPU,'-I',str(SRC),'-x','assembler-with-cpp']; cmd(*ld_flags,'-E','-P',ld_src,'-o',ld)
    # One standalone probe per header (including the unused template) and one for the ld input.
    for p in [q for q in ps if q.suffix=='.h']+[ld_src]:
        flags=[gcc,*CPU,'-I',SRC,'-x','c' if p.suffix=='.h' else 'assembler-with-cpp']
        snap=predefs(cmd(*flags,'-dM','-E',p)); check(all(snap.get(k)==v for k,v in selected.items()),'Header/ld policy missing: '+p.name)
    writej(BUILD/'linker-cpp.json',dict(directory=str(BUILD),file=str(ld_src),arguments=[*ld_flags,'-E','-P',str(ld_src),'-o',str(ld)]))
    rsp(BUILD/'link.rsp',[*CPU,'-nostartfiles','-nostdlib','-T',ld,'-Wl,-e,reset_handler',
        '-Wl,--gc-sections','-Wl,--no-undefined','-Wl,-Map=demo_threadx.map',*[obj(SRC/f) for f in sorted(direct)],
        '-Wl,--start-group',archive,'-lc','-lgcc','-Wl,--end-group','-o',elf]); cmd(gcc,'@link.rsp')
    nm=tool('nm'); symbols=cmd(nm,'-n','--defined-only',elf); names={l.split()[-1] for l in symbols.splitlines() if l.split()}
    required=ENTRIES+['tx_application_define','reset_handler','__tx_PendSVHandler','__tx_SysTickHandler','_tx_thread_schedule',
        '_txe_thread_create','_tx_thread_create','_tx_timer_thread_entry','_tx_safety_critical_exception_handler',
        '_tx_misra_ipsr_get','_tx_misra_memset','_tx_misra_message_copy','_tx_thread_interrupt_disable','_tx_thread_interrupt_restore']
    check(set(required)<=names and not cmd(nm,'-u',elf).strip(), 'Missing or unresolved ELF symbols: '+str(set(required)-names))
    m=mapfile.read_text(encoding='utf-8',errors='replace'); check('tx_misra.S.o)' in m and 'tx_misra.c.o)' not in m, 'Wrong MISRA provider in map')
    dis=cmd(tool('objdump'),'-d',elf)
    def calls(p): return [l for l in dis.splitlines() if re.search(r'\bbl(?:\.w)?\s+\w+\s+<'+re.escape(p)+'>',l)]
    for fn in ('_txe_thread_create','_tx_misra_message_copy','_tx_misra_ipsr_get','_tx_misra_memset'):
        check(calls(fn), 'Expected call target missing: '+fn)
    attrs=cmd(tool('readelf'),'-A',elf); check('v7E-M' in attrs and 'VFPv4-D16' in attrs and 'VFP registers' in attrs,'Unexpected ELF attributes: '+attrs)
    # Exercise failure in both C and preprocessed assembly, also reject wrong numeric policy / ABI.
    rejects=[('c',['-DTX_MISRA_ENABLE=0'],'set project choices here'),('assembler-with-cpp',['-DTX_DISABLE_ERROR_CHECKING=0'],'incompatible kernel policy'),
             ('c',['-DTX_MAX_PRIORITIES=64'],'set project choices here'),('assembler-with-cpp',['-mfloat-abi=soft'],'requires 32-bit little-endian')]
    for lang,extra,why in rejects: cmd(gcc,*CPU,*extra,'-x',lang,'-I',SRC,'-E','-',inp='#include "autoconf.h"\n',reject=why)
    outputs={'symbols.txt':symbols,'disassembly.txt':dis,'attributes.txt':attrs}
    for f,txt in outputs.items(): (BUILD/f).write_text(txt,encoding='utf-8')
    report=dict(compiler=version,target='arm-none-eabi',cpu=CPU,source_lock=LOCK214,autoconf_sha256=sha(HEADER),
        toolchain_libs={n:cmd(gcc,*CPU,'-print-file-name='+n).strip() for n in ('libc.a','libgcc.a')},
        compiled={s:sum(p.suffix==s for p in sources) for s in ('.c','.S','.s')},explicit_inputs=len(ps),cpp_units=len(sources)+1,header_probes=14,
        policy=selected,off_tested=OFF,archived=len(members),provider='tx_misra.S',thumb_aliases=aliases,unchanged_sections=section_hashes,compiled_not_linked=['tx_misra.c'],
        version_id_in_elf='_tx_version_id' in names,entry_points=ENTRIES,checked_symbols=required,negative_tests=len(rejects),
        undefined_symbols=0,elf_sha256=sha(elf),map_sha256=sha(mapfile),compile_db_sha256=sha(BUILD/'compile_commands.json'))
    writej(BUILD/'threadx-m4f.code-workspace',{'folders':[{'path':'../..'}],'settings':{
        'C_Cpp.default.compilerPath':str(gcc),'C_Cpp.default.compilerArgs':CPU,
        'C_Cpp.default.compileCommands':'${workspaceFolder}/build/gnu-m4f/compile_commands.json'}})
    writej(ok,report); print(f"{version}\nVerified #215: {len(sources)} objects, {report['cpp_units']} CPP inputs including ld, 14 header checks, {len(ENTRIES)} entries, {len(rejects)} rejected conflicts.\n{elf}")

if __name__=='__main__':
    try: main()
    except (RuntimeError,OSError) as e: raise SystemExit(str(e))
