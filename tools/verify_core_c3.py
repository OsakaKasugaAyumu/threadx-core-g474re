#!/usr/bin/env python3
"""Check fresh C3 outputs vs saved, hash-locked reference ELFs; never relink a reference."""
import argparse
from collections import Counter
from html import escape
import re
import struct
import subprocess
from pathlib import Path
from build_core_c3 import (CORE, ROOT, SRC, GREF, BREF, C2, ENTRIES, START, SYSTEM, PREFIX, J, H, dump, sha, check, env, guardroot, snapshot, deps)

class ELF32:
    def __init__(self,data):
        self.raw=data; self.h=struct.unpack_from('<16sHHIIIIIHHHHHH',data); h=self.h
        check(h[0][:6]==b'\x7fELF\x01\x01' and h[2]==40 and h[11]==40,'Expected ELF32 little-endian ARM')
        sections=[struct.unpack_from('<10I',data,h[6]+i*h[11]) for i in range(h[12])]; self.sections=sections
        names=self.blob(sections[h[13]]); self.names=[self.string(names,s[0]) for s in sections]; self.sec=dict(zip(self.names,sections))
        self.ph=[struct.unpack_from('<8I',data,h[5]+i*h[9]) for i in range(h[10])]
    @staticmethod
    def string(tab,n): return tab[n:tab.index(b'\0',n)].decode()
    def blob(self,s): return b'' if s[1]==8 else self.raw[s[4]:s[4]+s[5]]
    def runtime(self):
        return [(n,s[1:4],s[5],s[8:],self.blob(s)) for n,s in zip(self.names,self.sections) if s[2]&2]
    def relocations(self):  # Compare relocation meaning, not ELF string-table or symbol indices.
        out=[]
        for s in self.sections:
            if s[1] not in (4,9) or not self.sections[s[7]][2]&2: continue
            tbl=self.sections[s[6]]; st=self.blob(self.sections[tbl[6]]); sy=self.blob(tbl); payload=self.blob(s)
            for i in range(0,len(payload),s[9]):
                off,info=struct.unpack_from('<II',payload,i); name,value,sz,flags,other,ndx=struct.unpack_from('<IIIBBH',sy,(info>>8)*tbl[9]); out.append((self.names[s[7]],off,info&255,self.string(st,name),value,sz,flags,other,self.names[ndx] if 0<ndx<len(self.names) else ndx,struct.unpack_from('<i',payload,i+8)[0] if s[1]==4 else None))
        return out
    def image(self):
        image={}; explicit={}
        for t,o,v,p,fs,ms,flags,al in self.ph:
            if t!=1: continue
            for i,c in enumerate(self.raw[o:o+fs]): check(p+i not in image,'Overlapping flash load segments'); image[p+i]=c
        for s in self.sections:
            if not(s[2]&2 and s[1]!=8 and s[5]): continue
            matches=[ph for ph in self.ph if ph[0]==1 and ph[1]<=s[4] and s[4]+s[5]<=ph[1]+ph[4] and s[3]-ph[2]==s[4]-ph[1]]
            check(len(matches)==1,'Unmapped loadable section'); ph=matches[0]; p=ph[3]+s[4]-ph[1]
            for i,c in enumerate(self.blob(s)): check(p+i not in explicit and image[p+i]==c,'Wrong section LMA'); explicit[p+i]=c
        return image,explicit

def archive(path):
    raw=path.read_bytes(); check(raw[:8]==b'!<arch>\n','Expected ordinary (not thin) GNU archive'); i=8; out={}; names=b''
    while i<len(raw):
        hd=raw[i:i+60];check(hd[58:]==b'`\n','Invalid ar header'); name=hd[:16].decode('ascii').rstrip(); size=int(hd[48:58]); b=raw[i+60:i+60+size]; i+=60+size+(size&1)
        if name=='//': names=b; continue
        if name in ('/','/SYM64/'):continue
        if name.startswith('/'): off=int(name[1:]);name=names[off:names.index(b'/\n',off)].decode()
        else:name=name.rstrip('/')
        check(name not in out and len(b)==size,'Unexpected duplicate/truncated archive member');out[name]=b
    return out

def ihex(file):
    memory={};start=None;bank=0;eof=False
    for text in file.read_text('ascii').splitlines():
        check(not eof and text.startswith(':'),'Malformed HEX'); b=bytes.fromhex(text[1:]); count=b[0]; addr=int.from_bytes(b[1:3],'big'); typ=b[3];data=b[4:-1];check(count==len(data) and sum(b)%256==0,'HEX checksum/length failed')
        if typ==0:
            for i,c in enumerate(data): check(bank+addr+i not in memory,'Overlapping HEX');memory[bank+addr+i]=c
        elif typ in (2,4):check(count==2 and addr==0,'Bad HEX address');bank=int.from_bytes(data,'big')<<(4 if typ==2 else 16)
        elif typ in (3,5):
            check(start is None and count==4 and addr==0,'Bad/multiple entry records');start=(int.from_bytes(data[:2],'big')*16+int.from_bytes(data[2:],'big') if typ==3 else int.from_bytes(data,'big'))
        elif typ==1:check(count==addr==0,'Bad HEX EOF');eof=True
        else:raise ValueError('Unknown HEX record')
    check(eof and memory and start is not None,'Incomplete HEX');return memory,start

def map_normal(text,dirs):
    s=text.replace('\\','/')
    for d in dirs:s=s.replace(d.as_posix()+'/','${OUT}/')
    check('Memory Configuration' in s and 'Linker script and memory map' in s,'Incomplete GNU map'); pre,tail=s.split('Linker script and memory map',1); keep=[];debug=False
    for line in tail.splitlines():
        if line.startswith('.'):debug=line.startswith('.debug_')
        elif line.startswith(('LOAD ','OUTPUT(','START GROUP','END GROUP')):debug=False
        if not debug:keep.append(line)
    return pre+'Linker script and memory map'+'\n'.join(keep)

def compare(out,board,setup):
    gcc,cube,glue,vg,vb,oracle,srcs=setup; name=PREFIX if board else 'demo_threadx'; ref=BREF if board else GREF; here=out/ref.name; built=J(here/'built.json'); refelf=ref/(name+'.elf'); elf=here/(name+'.elf'); tools=gcc.parent.parent.resolve(); diag=here/'comparison';diag.mkdir(exist_ok=True)
    for n,h in built['artifacts'].items():check(sha(here/n)==h,'Artifact changed after link: '+n)
    def run(tool,*args):
        cmd=[str(gcc.with_name('arm-none-eabi-'+tool+'.exe')),*map(str,args)]; res=subprocess.run(cmd,cwd=here,env=env(),stdout=subprocess.PIPE,stderr=subprocess.PIPE); check(res.returncode==0,'Failed '+str(cmd)+' '+res.stderr.decode('utf8','replace')); return res.stdout.decode('utf8').replace('\r\n','\n')
    db=J(here/'compile_commands.json');dps=J(here/'dependencies.json'); check(len(db)==len(dps)==199 and sha(here/'dependencies.json')==built['dependencies_sha256'] and sha(here/'compile_commands.json')==built['compiler_database_sha256'],'Incomplete database')
    for c,d in zip(db,dps):
        p=Path(c['file']);obj=Path(c['output']); check(p==Path(d['file']) and obj.is_relative_to(here) and sha(obj)==d['object_sha256'] and p in [CORE/p.name,cube/START,cube/SYSTEM,glue],'Nonlocal object/source')
        paths=set(deps(here/'deps'/(p.name+'.d'),here)); check(paths==set(map(Path,d['deps'])) and all(z in [glue,p] or z.is_relative_to(CORE) or z.is_relative_to(tools) or z.is_relative_to(cube/'Drivers/CMSIS') for z in paths),'Unverified input');
        if p!=cube/START:
            k='generic' if p.parent==CORE else 'board'; check(d['cpp_sha256']==oracle[k,p.name]['matching_cpp_with_active_definitions_sha256'],'Missing TU comparison')
    lib=here/'libthreadx.a'; fresh=archive(lib); historic=archive(GREF/'libthreadx.a'); check(list(fresh)==list(historic)==built['archive_members'] and len(fresh)==194 and sha(lib)==built['archive_sha256'],'Library layout differs')
    for n,raw in fresh.items():
        check(raw==(here/'obj'/n).read_bytes(),'Archive member not local: '+n); src=ELF32(historic[n]);dst=ELF32(raw);check(src.runtime()==dst.runtime() and src.relocations()==dst.relocations(),'Library allocated sections/relocations differ: '+n)
    a,b=[ELF32(p.read_bytes()) for p in [refelf,elf]];check(a.h[1]==2 and a.h[:6]+a.h[7:]==b.h[:6]+b.h[7:] and a.names==b.names and a.runtime()==b.runtime(),'ELF header or allocated section differences');changed=[]
    for n in a.names:
        s,t=a.sec[n],b.sec[n]
        if s[:4]+s[5:]!=t[:4]+t[5:] or a.blob(s)!=b.blob(t): changed.append(n)
    check(all(n.startswith('.debug_') for n in changed),'Unexpected non-DWARF differences: '+repr(changed))
    check(a.ph==b.ph and all(a.raw[x[1]:x[1]+x[4]]==b.raw[x[1]:x[1]+x[4]] for x in a.ph),'ELF program headers/LOAD data differ')
    dbg=[]
    for prefix,path in [('reference',refelf),('candidate',elf)]:
        dst=diag/(prefix+'.no-debug.elf');run('objcopy','--strip-debug',path,dst);dbg.append(dst)
    check(dbg[0].read_bytes()==dbg[1].read_bytes(),'ELFs still differ after stripping debugging data')
    text=(here/(name+'.map')).read_text('utf8');base=(ref/(name+'.map')).read_text('utf8'); canonical=map_normal(text,[here]);check(canonical==map_normal(base,[ref,GREF]),'Non-debug linker map differs after explicit build-path remapping')
    load=[Path(v).resolve(strict=True) for v in re.findall(r'^LOAD (.+)$',text,re.M) if v.strip()!='linker stubs']; allowed=( {here/(n+'.o') for n in ('startup_stm32g474retx.s','system_stm32g4xx.c','board_threadx.c')}|{here/'obj/demo_threadx.c.o'} if board else {here/'obj'/(n+'.o') for n in ['cortexm4_crt0.s','cortexm4_vectors.S','demo_threadx.c','tx_initialize_low_level.S']})|{lib}
    check(set(z for z in load if not z.is_relative_to(tools))==allowed and all(z.name in {'crti.o','crtn.o','libc.a','libgcc.a','libm.a'} for z in load if z.is_relative_to(tools)),'Unapproved linker input')
    extracted=re.findall(r'^[^\s][^\n]*?libthreadx\.a\(([^)]+)\)\s*$',text.split('Discarded input sections',1)[0],re.M);check(len(extracted)==len(set(extracted)) and 'tx_misra.S.o' in extracted and 'tx_misra.c.o' not in extracted,'Wrong MISRA provider')
    words,symbols=run('nm','-n','--defined-only',elf),run('nm','-n','--defined-only',refelf); check(words==symbols and not run('nm','-u',elf).strip() and not run('nm','-u',refelf).strip(),'Undefined/symbol-address mismatch'); syms={n:int(k,16) for k,n in re.findall(r'^(\w+)\s+\S\s+(\S+)$',words,re.M)}
    attrs=run('readelf','-A',elf); check(attrs==run('readelf','-A',refelf) and all(k in attrs for k in ['v7E-M','VFPv4-D16','VFP registers']),'Hard-float ABI mismatch')
    image,explicit=b.image(); check((image,explicit)==a.image(),'ELF flash address/value mismatch'); lo,hi=min(image),max(image); check(lo==(0x08000000 if board else 0) and len(image)==hi-lo+1,'Invalid flash extent'); expected=bytes(image[i] for i in range(lo,hi+1))
    for fmt,ext in [('binary','bin'),('ihex','hex')]:run('objcopy','-O',fmt,refelf,diag/('reference.'+ext))
    bn=here/(name+'.bin');hx=here/(name+'.hex'); check(bn.read_bytes()==expected==(diag/'reference.bin').read_bytes() and ihex(hx)==ihex(diag/'reference.hex')==(explicit,b.h[4]),'Flash payload, addresses or entry differ')
    if board:check((ref/bn.name).read_bytes()==expected and ihex(ref/hx.name)==(explicit,b.h[4]) and (ref/hx.name).read_bytes()==hx.read_bytes(),'Historical board HEX/BIN differs')
    else:check((GREF/'after-explicit-include.bin').read_bytes()==expected,'Historical generic BIN differs')
    vector=b.blob(b.sec['.isr_vector' if board else '.vectors']); v=struct.unpack('<'+'I'*(len(vector)//4),vector);check(v[1]==b.h[4]==(syms['Reset_Handler' if board else 'reset_handler']|1) and v[14]==(syms['__tx_PendSVHandler']|1) and v[15]==(syms['SysTick_Handler']|1),'Wrong reset or exception vectors')
    if board:
        names=re.findall(r'^\s*\.word\s+(\w+)',(cube/START).read_text('utf8').split('g_pfnVectors:',1)[1].split('.weak',1)[0],re.M);check(len(v)==len(names)==118 and all(w==(0 if n=='0' else syms[n] if i==0 else syms[n]|1) for i,(w,n) in enumerate(zip(v,names))) and v[0]==0x20020000,'Device vectors differ from ST source')
        check(hex(v[1])==vb['reset'] and hex(v[14])==vb['pendsv'] and hex(v[15])==vb['systick'] and syms['_ebss']-syms['_sdata']==vb['sram_data_bss_bytes'],'Board memory map mismatch')
    entries={n:hex(syms[n]) for n in ENTRIES}; check(all(lo<=syms[n]<=hi for n in ENTRIES) and all(0x20000000<=syms['thread_'+str(i)+suffix]<0x20020000 for suffix in ('','_counter') for i in range(8)),'Missing Demo objects/entry')
    dis=(here/'disassembly.txt').read_text('utf8'); edges={}
    for caller,callee in [('tx_application_define','_txe_thread_create'),('_txe_thread_create','_tx_thread_create'),('_tx_thread_create','_tx_misra_memset'),('SysTick_Handler','_tx_timer_interrupt')]:
        body=re.search(r'^0*'+format(syms[caller],'x')+r' <[^>]+>:\n(.*?)(?:\n\n|\Z)',dis,re.S|re.M);calls=len(re.findall(r'\bblx?(?:\.w)?\s+[\da-f]+ <'+callee+r'>',body[1])) if body else 0;check(calls>=1,'Missing call '+caller+' -> '+callee);edges[caller+' -> '+callee]=calls
    # Static source lookup from the candidate itself (not a VS Code F12 or a board session).
    focus={'tx_application_define':'demo_threadx.c','_txe_thread_create':'txe_thread_create.c','_tx_thread_create':'tx_thread_create.c','_tx_misra_memset':'tx_misra.S','__tx_PendSVHandler':'tx_thread_schedule.S','SysTick_Handler':('board_threadx.c' if board else 'tx_initialize_low_level.S')}; dwarf={}
    for symbol,filename in focus.items():
        loc=run('addr2line','-e',elf,hex(syms[symbol])).strip();path,lineno=loc.rsplit(':',1);target=glue if filename=='board_threadx.c' else CORE/filename
        check(Path(path).resolve()==target and lineno.isdigit() and 0<int(lineno)<=len(target.read_bytes().splitlines()),'Wrong DWARF source: '+symbol+' '+loc);dwarf[symbol]=[str(target),int(lineno)]
    # Attribute active public #defines to their actual input and line, rather than grep other versions.
    import json
    demo=next(c for c in db if Path(c['file']).name=='demo_threadx.c');args=demo['arguments'][:demo['arguments'].index('-MD')]
    pp=subprocess.run([*args,'-E','-dD',demo['file']],cwd=here,env=env(),check=True,stdout=subprocess.PIPE,stderr=subprocess.PIPE).stdout.decode('utf8'); sources={}; filename='';lineno=0
    for line in pp.splitlines():
        marker=re.match(r'^#\s+(\d+)\s+("(?:\\.|[^"\\])*")',line)
        if marker: filename=json.loads(marker[2]);lineno=int(marker[1]);continue
        define=re.match(r'^#define\s+(tx_thread_create|TX_TIMER_TICKS_PER_SECOND)\b',line)
        if define:
            key=define[1]; target=CORE/('tx_api.h' if key=='tx_thread_create' else 'autoconf.h');check(Path(filename).resolve()==target and key in target.read_text('utf8').splitlines()[lineno-1], 'Wrong active macro location: '+key)
            if key=='tx_thread_create': check('_txe_thread_create' in line,'Wrong checked API mapping')
            sources[key]=[str(target),lineno]
        lineno+=1
    check(set(sources)=={'tx_thread_create','TX_TIMER_TICKS_PER_SECOND'},'Missing macro source lookup')
    return dict(status='PASS',target=here.name,objects=len(db),archive_members=len(fresh),kernel_members_in_map=extracted,declared_link_inputs=[str(z) for z in load],map_runtime_projection_sha256=H(canonical.encode()),
        entry=hex(b.h[4]),vectors=len(v),initial_sp=hex(v[0]),vector14_pendsv=hex(v[14]),vector15_systick=hex(v[15]),application_threads=8,entries=entries,undefined=0,call_sites=edges,dwarf_source_locations=dwarf,cpp_macro_source_locations=sources,
        elf_sha256=sha(elf),reference_elf_sha256=sha(refelf),map_sha256=sha(here/(name+'.map')),reference_map_sha256=sha(ref/(name+'.map')),different_elf_section_payload_or_metadata=changed,
        no_debug_elf_sha256=sha(dbg[1]),program_headers=b.ph,alloc_section_names=[n for n,s in b.sec.items() if s[2]&2],checked_archive_machine_sections_and_relocations=len(fresh),
        flash_first=f'0x{lo:08X}',flash_last=f'0x{hi:08X}',binary_bytes=len(expected),hex_explicit_bytes=len(explicit),bin_padding_bytes=len(expected)-len(explicit),bin_sha256=sha(bn),hex_sha256=sha(hx),raw_hex_matches_objcopy_of_verified_reference=(diag/'reference.hex').read_bytes()==hx.read_bytes(),
        comparison='All ELF program headers and payloads, all non-DWARF sections, execution symbols, linker contributions, all physical Flash addresses and bytes match; no undefined symbols.',execution='not flashed; not hardware tested',compile_warnings=built['warnings'])



def report_html(results):
    cards=[]
    for result in results:
        n=result['target']; base=PREFIX if n=='nucleo-g474re' else 'demo_threadx'; label='NUCLEO-G474RE 板级' if n=='nucleo-g474re' else '通用 M4F（不是 G474RE 板级镜像）'
        links=' · '.join(f'<a href="{n}/{base}.{ext}">{ext.upper()}</a>' for ext in ['elf','map','hex','bin'])
        cards.append(f'''<section><h2>{label}</h2><p>{links} · <a href="{n}/compile_commands.json">实际编译命令</a> · <a href="{n}/dependencies.json">实际源码依赖</a></p><dl>
<dt>Flash 地址（首尾均含）</dt><dd><code>{result['flash_first']}–{result['flash_last']}</code></dd><dt>BIN / HEX 明确携带的数据</dt><dd>{result['binary_bytes']:,} / {result['hex_explicit_bytes']:,} 字节；差额是 BIN 的对齐填充</dd><dt>BIN SHA-256 · 与基线一致</dt><dd><code>{result['bin_sha256']}</code></dd>
<dt>全库 / map 实际抽取</dt><dd>{result['archive_members']} / {len(result['kernel_members_in_map'])} 个成员；MISRA 实现由 tx_misra.S 提供</dd><dt>入口与异常向量</dt><dd>应用 8 个线程、6 个入口；向量表 {result['vectors']} 项，初始 MSP {result['initial_sp']}，复位 {result['entry']}，PendSV {result['vector14_pendsv']}，SysTick {result['vector15_systick']}；未解析符号 0</dd></dl></section>''')
    return '''<!doctype html><html lang="zh-CN"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>C3 · 固定配置固件等价性验收</title><style>body{background:#f6f8f5;color:#203237;font:16px/1.65 system-ui,"Microsoft YaHei",sans-serif;margin:0}main{max-width:880px;margin:auto;padding:1.4rem 1rem 3rem}section{background:white;border:1px solid #cddad6;border-radius:.4rem;padding:0 1.25rem;margin:1.2rem 0}a{color:#00645a}dt{font-weight:650}dd{margin:0 0 .65rem}code{overflow-wrap:anywhere}h1,h2{line-height:1.3}</style></head><body><main><p>ThreadX · C3 · 本地构建验收</p><h1>两个目标的固件均与各自基线等价</h1><p>两次从裁剪源码独立编译、归档与链接，未复用旧内核对象。已比较整个归档的机器段和重定位、ELF 装载段及符号、map 的非调试部分、按地址解码后的 HEX 和 BIN。原始 ELF/map 不是逐字节相同；去除调试信息后 ELF 逐字节相同。未烧录、未验证硬件运行、未发布。</p>'''+''.join(cards)+'''<p><a href="verified.json">完整比对与哈希（JSON）</a></p></main></body></html>\n'''

if __name__=='__main__':
    try:
        ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('run');args=ap.parse_args();out=guardroot(args.run); setup=snapshot(); results=[]
        for board in [False,True]:results.append(compare(out,board,setup));print('Equivalent:',results[-1]['target'],results[-1]['binary_bytes'],results[-1]['bin_sha256'],flush=True)
        snapshot();dump(out/'verified.json',dict(stage='C3',status='PASS',compiler=setup[3]['compiler'],configuration_sha256=setup[3]['autoconf_sha256'],c2_manifest_sha256=sha(C2/'manifest.json'),c2_evidence_sha256=sha(C2/'evidence.json'),source_root=str(CORE),baselines_read_only=True,hardware='not connected, flashed or tested',published=False,results=results));(out/'index.html').write_bytes(report_html(results).encode('utf8'))
    except (RuntimeError,OSError,ValueError) as exc:raise SystemExit(str(exc))
