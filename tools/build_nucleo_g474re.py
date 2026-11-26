#!/usr/bin/env python3
"""Build and verify a flash image for NUCLEO-G474RE; never programs a target."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import struct
import subprocess
import sys
from build_profile import ROOT, SRC, BUILD as COMMON, CPU, CFLAGS, HEADER, ENTRIES, run, check, sha, writej

BOARD = 'nucleo-g474re'
OUT = ROOT / 'build' / BOARD
STM_PROJECT = Path('Projects/NUCLEO-G474RE/Applications/ThreadX/Tx_Thread_Sync')
START = STM_PROJECT / 'STM32CubeIDE/Application/Startup/startup_stm32g474retx.s'
SYSTEM = STM_PROJECT / 'Src/system_stm32g4xx.c'
LINK = STM_PROJECT / 'STM32CubeIDE/STM32G474RETX_FLASH.ld'
LOCK = {START:'60cbc83b0a02932442bd1c9dd0ef4961620d1f914a5de9d9c21303db4157a18e',
        SYSTEM:'875f78f4a45764120e7626493e10a826cd936404c727131ffc15b4d18982b94c',
        LINK:'6b220759c5dd3645cece10543717ece893ba5ba939c065f3af1358d99d3f480e'}
PREFIX = 'demo_threadx_g474re'
FLASH, FLASH_END = 0x08000000, 0x08080000
RAM, RAM_END = 0x20000000, 0x20020000

def cube_root(override):
    if override:
        root=Path(override).expanduser().resolve(); check((root/START).is_file(),'Not the X-CUBE-AZRTOS-G4 package root: '+str(root)); return root
    home=Path(ROOT.anchor)/'stm_workspace'; matches=[]
    if home.is_dir():
        for pat in ('x-cube-azrtos-g4*','x-cube-azrtos-g4*/x-cube-azrtos-g4*'):
            matches.extend(p.resolve() for p in home.glob(pat) if (p/START).is_file())
    check(len(matches)==1,'Specify --cube-root <X-CUBE-AZRTOS-G4 root>, or set STM32CUBE_G4_ROOT.'); return matches[0]

def ihex_image(path):
    # Verify all checksums, absolute data addresses and EOF, not just the extension.
    mem={}; bank=0; eof=False
    for line in path.read_text(encoding='ascii').splitlines():
        check(not eof and line.startswith(':'),'Invalid Intel HEX record'); r=bytes.fromhex(line[1:]); n=r[0]; a=int.from_bytes(r[1:3],'big'); kind=r[3]; data=r[4:-1]
        check(len(data)==n and not(sum(r)&255),'Bad Intel HEX checksum or length')
        if kind==0:
            for i,b in enumerate(data):
                k=bank+a+i; check(FLASH<=k<FLASH_END and k not in mem,'HEX outside flash or overlaps'); mem[k]=b
        elif kind in (2,4):
            check(n==2 and a==0,'Invalid HEX address'); bank=int.from_bytes(data,'big')<<(4 if kind==2 else 16)
        elif kind in (3,5): check(n==4 and a==0,'Invalid HEX entry')
        elif kind==1: check(a==n==0,'Invalid HEX EOF'); eof=True
        else: raise RuntimeError('Unknown HEX record '+str(kind))
    check(eof and mem and min(mem)==FLASH,'Missing flash base or EOF'); return mem

def main():
    ap=argparse.ArgumentParser(description=__doc__); ap.add_argument('--cube-root',default=os.environ.get('STM32CUBE_G4_ROOT')); ap.add_argument('--gcc'); ap.add_argument('-j','--jobs',type=int); args=ap.parse_args()
    OUT.mkdir(parents=True,exist_ok=True); ok=OUT/'verified.json'; ok.unlink(missing_ok=True)
    cube=cube_root(args.cube_root)
    for relative,h in LOCK.items():
        check(hashlib.sha256((cube/relative).read_bytes().replace(b'\r\n',b'\n')).hexdigest()==h,
              'Changed ST startup/system/linker input; review it before use: '+str(relative))
    # Rebuild first. Never trust an old static library or mix incompatible ABI/flags.
    sub=[sys.executable,'-B',str(ROOT/'tools/build_profile.py')]; sub += ['--gcc',args.gcc] if args.gcc else []; sub += ['-j',str(args.jobs)] if args.jobs else []
    subprocess.run(sub,cwd=ROOT,check=True)
    common=json.loads((COMMON/'verified.json').read_text(encoding='utf8')); db=json.loads((COMMON/'compile_commands.json').read_text(encoding='utf8'))
    gcc=Path(db[0]['arguments'][0]); archive=COMMON/'libthreadx.a'; demo=COMMON/'obj/demo_threadx.c.o'
    def command(*a): return run(a,OUT)
    def tool(t): return gcc.with_name('arm-none-eabi-'+t+gcc.suffix)
    includes=[SRC,cube/'Drivers/CMSIS/Include',cube/'Drivers/CMSIS/Device/ST/STM32G4xx/Include']
    c=[*CPU,*CFLAGS,'-DSTM32G474xx','-DUSER_VECT_TAB_ADDRESS',*(a for p in includes for a in ('-I',str(p)))]
    records=[]; objs=[]; glue=ROOT/'boards'/BOARD/'board_threadx.c'
    for p,flags in ((cube/START,[*CPU,'-g','-x','assembler']), (cube/SYSTEM,c), (glue,c)):
        obj=OUT/(p.name+'.o'); full=[str(gcc),*flags,'-c',str(p),'-o',str(obj)]; command(*full); objs.append(obj)
        records.append(dict(directory=str(OUT),file=str(p),arguments=full,output=str(obj)))
    elf=OUT/(PREFIX+'.elf'); mapfile=OUT/(PREFIX+'.map'); hexfile=OUT/(PREFIX+'.hex'); binfile=OUT/(PREFIX+'.bin')
    # Use the matching newlib constructor prologue/epilogue for ST's call to __libc_init_array.
    def crt(name):
        p=Path(command(gcc,*CPU,'-print-file-name='+name).strip()); check(p.is_file(),'Missing runtime startup: '+name); return p
    command(gcc,*CPU,'-nostartfiles','-nostdlib','-T',cube/LINK,'-Wl,--gc-sections,--no-undefined',
            '-Wl,-Map='+str(mapfile),crt('crti.o'),*objs,demo,'-Wl,--start-group',archive,'-lc','-lgcc','-Wl,--end-group',crt('crtn.o'),'-o',elf)
    command(tool('objcopy'),'-O','ihex',elf,hexfile); command(tool('objcopy'),'-O','binary',elf,binfile)
    data=binfile.read_bytes(); hx=ihex_image(hexfile); check(len(data)==max(hx)-FLASH+1 and all(data[a-FLASH]==b for a,b in hx.items()),'HEX/BIN mismatch')
    symbols=command(tool('nm'),'-n','--defined-only',elf); syms={n:int(a,16) for a,n in re.findall(r'^(\w+)\s+\S\s+(\S+)$',symbols,re.M)}
    check(not command(tool('nm'),'-u',elf).strip(),'Undefined ELF symbols')
    fields=command(tool('readelf'),'-h',elf); attrs=command(tool('readelf'),'-A',elf); ph=command(tool('readelf'),'-lW',elf)
    check(all(x in attrs for x in ('v7E-M','VFPv4-D16','VFP registers')),'Wrong ARM ELF ABI')
    entry=int(re.search(r'Entry point address:\s*(0x[0-9a-fA-F]+)',fields)[1],16)
    check(entry==syms['Reset_Handler']|1,'Wrong entry point'); check(syms['g_pfnVectors']==FLASH and syms['_estack']==RAM_END,'Wrong memory origins')
    seg=[]
    for v,p,fs,ms in re.findall(r'^\s*LOAD\s+0x\w+\s+(0x\w+)\s+(0x\w+)\s+(0x\w+)\s+(0x\w+)',ph,re.M):
        v,p,fs,ms=map(lambda x:int(x,16),(v,p,fs,ms)); check((FLASH<=v<=v+ms<=FLASH_END or RAM<=v<=v+ms<=RAM_END) and (not fs or FLASH<=p<p+fs<=FLASH_END),'Bad LOAD segment'); seg.append([v,p,fs,ms])
    check(any(x[0]==FLASH and x[2] for x in seg) and any(x[0]==RAM and x[2] for x in seg),'Missing flash vectors or initialized RAM')
    for sym in ENTRIES+['main','SystemInit','_tx_initialize_low_level','__libc_init_array','_tx_thread_schedule','_tx_timer_interrupt','_txe_thread_create','_tx_misra_ipsr_get','_tx_safety_critical_exception_handler']:
        check(FLASH<=syms[sym]<FLASH_END,'Missing flash code: '+sym)
    for i in range(8):
        for sym in ('thread_'+str(i),'thread_'+str(i)+'_counter'): check(RAM<=syms[sym]<RAM_END,'Missing RAM object: '+sym)
    check(RAM<=syms['_sdata']<=syms['_edata']<=syms['_sbss']<=syms['_ebss']<=RAM_END-0x400 and FLASH<=syms['_sidata']<FLASH_END,'Data/BSS/stack layout')
    vector_src=(cube/START).read_text(encoding='utf8').split('g_pfnVectors:',1)[1].split('.weak',1)[0]
    names=re.findall(r'^\s*\.word\s+(\w+)',vector_src,re.M); check(len(names)==118 and names[-1]=='FMAC_IRQHandler','Wrong G474 device vector table')
    words=struct.unpack_from('<118I',data)
    for i,n in enumerate(names):
        w=words[i]; expected=0 if n=='0' else syms[n] if i==0 else syms[n]|1
        check(w==expected and (not i or not w or FLASH<=w<FLASH_END),'Wrong vector: '+str(i)+' '+n)
    check(words[1]==entry and words[14]==syms['__tx_PendSVHandler']|1 and words[15]==syms['SysTick_Handler']|1 and syms['Default_Handler'] not in [words[14]&~1, words[15]&~1], 'Weak/default exception override')
    dis=command(tool('objdump'),'-d',elf); mp=mapfile.read_text(encoding='utf8',errors='replace')
    def body(n): return dis.split('<'+n+'>:',1)[1].split('\n\n',1)[0]
    check('<_tx_timer_interrupt>' in body('SysTick_Handler'),'SysTick does not actually call the ThreadX tick')
    check(all(f not in mp for f in ('cortexm4_crt0.s.o','cortexm4_vectors.S.o','tx_initialize_low_level.S.o','tx_misra.c.o)')) and 'tx_misra.S.o)' in mp,'Generic startup or wrong MISRA provider leaked into the board map')
    # Navigation DB: all verified reading commands (including unused generic sources), plus actual board files.
    writej(OUT/'compile_commands.json',db+records); editor=ROOT/'build/editor'; editor.mkdir(exist_ok=True); writej(editor/'compile_commands.json',db+records)
    writej(OUT/'nucleo-g474re.code-workspace',dict(folders=[dict(path='../..')],settings={'C_Cpp.default.compilerPath':str(gcc),'C_Cpp.default.compilerArgs':CPU,'C_Cpp.default.compileCommands':'${workspaceFolder}/build/nucleo-g474re/compile_commands.json'}))
    for name,text in [('symbols.txt',symbols),('disassembly.txt',dis),('segments.txt',ph),('attributes.txt',attrs)]: (OUT/name).write_text(text,encoding='utf8')
    report=dict(board='NUCLEO-G474RE',mcu='STM32G474RET6',compiler=common['compiler'],source_lock=common['source_lock'],autoconf_sha256=sha(HEADER),board_glue_sha256=sha(glue),
        package_inputs={str(k.as_posix()):v for k,v in LOCK.items()},kernel_archive_sha256=sha(archive),st_vector_words=len(names),initial_msp=hex(words[0]),
        reset=hex(entry),pendsv=hex(words[14]),systick=hex(words[15]),flash_base=hex(FLASH),flash_image_bytes=len(data),sram_data_bss_bytes=syms['_ebss']-syms['_sdata'],
        reset_hclk_hz=16000000,tick_hz=int(common['policy']['TX_TIMER_TICKS_PER_SECOND'].rstrip('ULul')),entries=ENTRIES,undefined_symbols=0,load_segments=seg,
        artifacts={p.name:sha(p) for p in (elf,hexfile,binfile,mapfile)},evidence='static build/image checks; not hardware execution')
    writej(ok,report); print(f"Verified NUCLEO-G474RE: {len(data)} image bytes at 0x{FLASH:08X}, MSP=0x{words[0]:08X}, {len(names)} vectors, {len(ENTRIES)} entries.\n{hexfile}")

if __name__=='__main__':
    try: main()
    except (RuntimeError,OSError,subprocess.CalledProcessError) as e: raise SystemExit(str(e))
