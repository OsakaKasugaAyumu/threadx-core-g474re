#!/usr/bin/env python3
"""Fresh C3 builds; never call a baseline build, programmer or network service.
py -B tools/build_core_c3.py core-c3-01  (new directory under build/ only)
Oracles: C2 source hashes/CPP fingerprints and the two existing verified builds.
"""
import argparse
from concurrent.futures import ThreadPoolExecutor
import hashlib
import json
from pathlib import Path
import re
import subprocess
import sys
from threading import Lock
from build_profile import ROOT, SRC, BUILD as GREF, CPU, CFLAGS, ENTRIES, LOCK214, check, env, sha, writej
from build_nucleo_g474re import START, SYSTEM, LINK, LOCK, PREFIX

CORE=ROOT/'threadx_core'; BREF=ROOT/'build/nucleo-g474re'; C2=ROOT/'reports/core-c2'
PROFILE='49c9bebd295564446576cc6f4a75fa5d7ef0128f16280adb8800ddd890734404'
GENSTART={'cortexm4_crt0.s','cortexm4_vectors.S','tx_initialize_low_level.S'}
DIRECT=GENSTART|{'demo_threadx.c'}
def J(p): return json.loads(p.read_bytes())
def H(b): return hashlib.sha256(b).hexdigest()
def dump(p,v): writej(p,v)
def guardroot(name):
    check(re.fullmatch(r'core-c3-[a-z0-9-]+',name),'Provide a fresh core-c3-* directory name')
    d=ROOT/'build'/name; check(d.resolve()==d and d.parent.resolve()==ROOT/'build','Unexpected run directory'); return d

def snapshot():
    m=J(C2/'manifest.json'); e=J(C2/'evidence.json'); locks=e['locks']; vg=J(GREF/'verified.json'); vb=J(BREF/'verified.json'); gdb=J(GREF/'compile_commands.json'); bdb=J(BREF/'compile_commands.json')
    check(len(m['files'])==215 and m['profile_sha256']==locks['autoconf_sha256']==vg['autoconf_sha256']==vb['autoconf_sha256']==PROFILE and m['source_lock']==vg['source_lock']==vb['source_lock']==locks['source_lock']==LOCK214,'Wrong C2/ELF baseline')
    for root,side in [(SRC,'before'),(CORE,'after')]:
        expected={r['name']:r[side]['sha256'] for r in m['files']}; check({p.name:sha(p) for p in root.iterdir() if p.is_file()}==expected,'Source hashes differ: '+str(root))
    for k,v in locks['preexisting_artifacts'].items(): check(sha(ROOT/k)==v,'Baseline drift: '+k)
    for k,p in [('generic_commands_sha256',GREF/'compile_commands.json'),('board_commands_sha256',BREF/'compile_commands.json'),('linker_cpp_command_sha256',GREF/'linker-cpp.json')]: check(sha(p)==locks[k],'Commands changed: '+str(p))
    check(len(gdb)==199 and len(bdb)==202 and bdb[:199]==gdb,'Unexpected command sets')
    gcc=Path(gdb[0]['arguments'][0]); cube=Path(bdb[199]['file']).parents[len(START.parts)-1]; glue=ROOT/'boards/nucleo-g474re/board_threadx.c'
    check([Path(x['file']) for x in bdb[199:]]==[cube/START,cube/SYSTEM,glue],'Unexpected ST/app inputs')
    for p,h in LOCK.items(): check(H((cube/p).read_bytes().replace(b'\r\n',b'\n'))==h==vb['package_inputs'][p.as_posix()],'Bad ST input: '+str(p))
    check(sha(glue)==vb['board_glue_sha256'] and sha(GREF/'libthreadx.a')==vb['kernel_archive_sha256'],'Glue/archive baseline mismatch')
    check(sha(GREF/'demo_threadx.elf')==vg['elf_sha256'] and sha(GREF/'demo_threadx.map')==vg['map_sha256'] and all(sha(BREF/k)==h for k,h in vb['artifacts'].items()),'Unverified existing ELF/map/image')
    oracle={(c['kind'],c['file']):c for c in e['contexts']}; check(len(oracle)==203,'Missing C2 CPP evidence')
    return gcc,cube,glue,vg,vb,oracle,[Path(x['file']).name for x in gdb]

def deps(path,cwd):
    s=path.read_text(encoding='utf8').replace('\\\n',' '); m=re.search(r':\s+',s); check(m,'Malformed dependency file: '+str(path)); out=[]
    for word in re.findall(r'(?:\\[ \t]|[^\s])+',s[m.end():]):
        q=Path(word.replace('\\ ', ' ').replace('\\#','#').replace('$$','$')); out.append((cwd/q).resolve(strict=True))
    return out

def response(path,items):
    path.write_bytes((''.join('"'+str(i).replace('\\','/').replace('"','\\"')+'"\n' for i in items)).encode())

class Target:
    def __init__(self,runroot,board,setup,jobs):
        self.board=board; self.gcc,self.cube,self.glue,self.vg,self.vb,self.oracle,self.names=setup; self.jobs=jobs
        self.out=runroot/('nucleo-g474re' if board else 'gnu-m4f'); self.obj=self.out/'obj'; self.dep=self.out/'deps'; self.base=PREFIX if board else 'demo_threadx'; self.obj.mkdir(parents=True); self.dep.mkdir(); self.lock=Lock(); self.warnings=[]
        self.inc=self.out/'misra-thumb-types.inc'; self.mapper=f'-fmacro-prefix-map={CORE}={SRC}'; self.tc=self.gcc.parent.parent.resolve(); self.meta={}
    def command(self,*args):
        args=list(map(str,args)); p=subprocess.run(args,cwd=self.out,env=env(),stdout=subprocess.PIPE,stderr=subprocess.PIPE); err=p.stderr.decode('utf8','replace')
        with self.lock:
            with (self.out/'commands.jsonl').open('a',encoding='utf8') as f: f.write(json.dumps(dict(argv=args,returncode=p.returncode,stderr=err))+'\n')
            if err.strip(): self.warnings.append(err.strip())
        check(not p.returncode,subprocess.list2cmdline(args)+'\n'+err+p.stdout.decode('utf8','replace')[:600]); return p.stdout.decode('utf8','strict').replace('\r\n','\n')
    def tool(self,name): return self.gcc.with_name('arm-none-eabi-'+name+'.exe')
    def known(self,kind,p,flags):
        s=self.command(self.gcc,*flags,'-E','-P','-dD',p); v=H('\n'.join(l for l in s.splitlines() if l.strip()).encode()); c=self.oracle[kind,p.name]; check(v==c['matching_cpp_with_active_definitions_sha256'],'CPP/active macros differ: '+p.name); return v
    def provenance(self,dep,p,kind):
        paths=deps(dep,self.out); allowed=[CORE,self.tc,self.cube/'Drivers/CMSIS']; isolated={self.glue,self.cube/START,self.cube/SYSTEM}
        check(p in paths and all(z in isolated or any(root in z.parents for root in allowed) for z in paths),'Unexpected dependency for '+p.name+': '+str(paths))
        if kind:
            wanted={CORE/n for n in self.oracle[kind,p.name]['entered_project_headers']}|({p} if p.parent==CORE else set()); actual={z for z in paths if CORE in z.parents}; check(actual==wanted,'Header set differs: '+p.name+': '+str(actual^wanted))
        return [str(z) for z in paths]
    def prepare_aliases(self):
        src=CORE/'tx_misra.S'; raw=self.out/'misra.raw.o'; dep=self.dep/'misra-raw.d'; flags=[*CPU,'-I',CORE,'-g','-x','assembler-with-cpp',self.mapper]; self.command(self.gcc,*flags,'-MD','-MF',dep,'-MT','c3','-c',src,'-o',raw); self.provenance(dep,src,'generic')
        def symbols(obj): return {n:(int(a,16),typ,sec) for a,typ,sec,n in re.findall(r'^\s*\d+:\s+([\da-fA-F]+)\s+\d+\s+(\w+)\s+GLOBAL\s+DEFAULT\s+(\d+)\s+(_tx_misra_\w+)$',self.command(self.tool('readelf'),'-Ws',obj),re.M)}
        orig=symbols(raw); anchor=orig['_tx_misra_pointer_to_ulong_convert']; aliases=sorted(n for n,(_,typ,_) in orig.items() if typ=='NOTYPE'); check(aliases==self.vg['thumb_aliases'] and anchor[1]=='FUNC' and anchor[0]&1,'Unexpected GNU Thumb aliases')
        check(all(orig[n]==(anchor[0]&~1,'NOTYPE',anchor[2]) for n in aliases),'Not aliases of one Thumb function')
        self.inc.write_text(''.join('.type '+n+', %function\n' for n in aliases),encoding='ascii'); return orig,symbols
    def compile(self,p):
        project=p.parent==CORE; plain=p==self.cube/START; kind='generic' if project else 'board'; dep=self.dep/(p.name+'.d'); out=(self.obj if project else self.out)/(p.name+'.o')
        if plain: flags=[*CPU,'-g','-x','assembler']; extra=['-Xassembler','--MD','-Xassembler',dep]; fingerprint=None
        else:
            if project: flags=[*CPU,'-I',CORE,* (CFLAGS if p.suffix=='.c' else ['-g','-x','assembler-with-cpp'])]
            else: flags=[*CPU,*CFLAGS,'-DSTM32G474xx','-DUSER_VECT_TAB_ADDRESS','-I',CORE,'-I',self.cube/'Drivers/CMSIS/Include','-I',self.cube/'Drivers/CMSIS/Device/ST/STM32G4xx/Include']
            if p.name=='tx_misra.S': flags+=['-Xassembler',self.inc]
            flags+=[self.mapper]; fingerprint=self.known(kind,p,flags); extra=['-MD','-MF',dep,'-MT','c3']
        args=list(map(str,[self.gcc,*flags,*extra,'-c',p,'-o',out])); self.command(*args); actual=self.provenance(dep,p,None if plain else kind)
        return dict(directory=str(self.out),arguments=args,file=str(p),output=str(out)),dict(file=str(p),object_sha256=sha(out),deps=actual,cpp_sha256=fingerprint,explicit_gas_metadata=str(self.inc) if p.name=='tx_misra.S' else None)
    def build(self):
        check(self.command(self.gcc,'--version').splitlines()[0]==self.vg['compiler'] and self.command(self.gcc,'-dumpmachine').strip()=='arm-none-eabi','Wrong toolchain')
        orig,symread=self.prepare_aliases(); ps=[CORE/n for n in self.names if not self.board or n not in GENSTART];
        if self.board: ps.extend([self.cube/START,self.cube/SYSTEM,self.glue]);
        with ThreadPoolExecutor(max_workers=self.jobs) as pool: allrows=list(pool.map(self.compile,ps))
        db,details=map(list,zip(*allrows)); dump(self.out/'compile_commands.json',db);dump(self.out/'dependencies.json',details)
        def obj(n): return self.obj/(n+'.o')
        final=symread(obj('tx_misra.S')); a=orig['_tx_misra_pointer_to_ulong_convert']; aliases=self.vg['thumb_aliases']; check(final=={n:a if n in aliases else v for n,v in orig.items()},'Bad Thumb symbol metadata')
        for section,expected in self.vg['unchanged_sections'].items():
            for prefix,o in [('raw',self.out/'misra.raw.o'),('final',obj('tx_misra.S'))]:
                dest=self.out/('misra.'+prefix+section+'.bin'); self.command(self.tool('objcopy'),'-j',section,'-O','binary',o,dest); check(sha(dest)==expected,'Unexpected MISRA code/data: '+section)
        members=[n+'.o' for n in self.names if n not in DIRECT|{'tx_misra.c'}]; archive=self.out/'libthreadx.a'; response(self.out/'archive.rsp',[self.obj/n for n in members]); self.command(self.tool('ar'),'rcs',archive,'@archive.rsp'); actual=self.command(self.tool('ar'),'t',archive).splitlines(); check(len(members)==194 and actual==members,'Archive contents/order differ')
        name=self.base; elf=self.out/(name+'.elf'); mp=self.out/(name+'.map'); baseargs=[*CPU,'-nostartfiles','-nostdlib']; ld=None; extras={}
        if not self.board:
            ld=self.out/'demo_threadx.preprocessed.ld'; source=CORE/'demo_threadx.crt0.ld'; f=[*CPU,'-I',CORE,'-x','assembler-with-cpp',self.mapper]; extras[source.name]=self.known('generic',source,f); dep=self.dep/'linker.d'; self.command(self.gcc,*f,'-E','-P','-MD','-MF',dep,'-MT','c3',source,'-o',ld); self.provenance(dep,source,'generic'); check(ld.read_bytes()==(GREF/ld.name).read_bytes(),'Generic linker script output differs'); t=CORE/'tx_user_sample.h'; extras[t.name]=self.known('template',t,[*CPU,'-I',CORE,*CFLAGS,self.mapper,'-x','c']); args=[*baseargs,'-T',ld,'-Wl,-e,reset_handler','-Wl,--gc-sections','-Wl,--no-undefined','-Wl,-Map='+mp.name,*[obj(n) for n in sorted(DIRECT)],'-Wl,--start-group',archive,'-lc','-lgcc','-Wl,--end-group','-o',elf]; response(self.out/'link.rsp',args); self.command(self.gcc,'@link.rsp')
        else:
            def crt(n):
                path=Path(self.command(self.gcc,*CPU,'-print-file-name='+n).strip()); check(self.tc in path.resolve(strict=True).parents,'Wrong toolchain CRT'); return path
            ld=self.cube/LINK; args=[*baseargs,'-T',ld,'-Wl,--gc-sections,--no-undefined','-Wl,-Map='+str(mp),crt('crti.o'),*[self.out/(f.name+'.o') for f in [self.cube/START,self.cube/SYSTEM,self.glue]],obj('demo_threadx.c'),'-Wl,--start-group',archive,'-lc','-lgcc','-Wl,--end-group',crt('crtn.o'),'-o',elf]; response(self.out/'link.rsp',args); self.command(self.gcc,*args)
        for fmt,ext in [('binary','bin'),('ihex','hex')]: self.command(self.tool('objcopy'),'-O',fmt,elf,self.out/(name+'.'+ext))
        for tool,arg,filename in [('nm',['-n','--defined-only'],'symbols.txt'),('objdump',['-d'],'disassembly.txt'),('readelf',['-lW'],'segments.txt'),('readelf',['-A'],'attributes.txt')]: (self.out/filename).write_bytes(self.command(self.tool(tool),*arg,elf).encode())
        check(not self.command(self.tool('nm'),'-u',elf).strip(),'Undefined symbols in linked image')
        dump(self.out/(self.out.name+'.code-workspace'),{'folders':[{'path':str(CORE)}],'settings':{'C_Cpp.default.compilerPath':str(self.gcc),'C_Cpp.default.compilerArgs':CPU,'C_Cpp.default.compileCommands':str(self.out/'compile_commands.json'),'C_Cpp.default.includePath':[str(CORE)],'C_Cpp.default.browse.path':[str(CORE)]}})
        dump(self.out/'built.json',dict(stage='built, pending image comparison',compiler=self.vg['compiler'],objects=len(db),compiler_database_sha256=sha(self.out/'compile_commands.json'),dependencies_sha256=sha(self.out/'dependencies.json'),core_root=str(CORE),linker=str(ld),linker_sha256=sha(ld),extra_cpp=extras,archive_members=members,archive_sha256=sha(archive),thumb_aliases=len(aliases),artifacts={p.name:sha(p) for p in [elf,mp,self.out/(name+'.bin'),self.out/(name+'.hex')]},warnings=list(dict.fromkeys(self.warnings))))

def main():
    ap=argparse.ArgumentParser(description=__doc__); ap.add_argument('run');ap.add_argument('-j',type=int,default=7); args=ap.parse_args(); out=guardroot(args.run); setup=snapshot(); out.mkdir(exist_ok=False); (out/'.gitignore').write_bytes(b'*\n');
    for board in [False,True]:
        t=Target(out,board,setup,max(1,min(args.j,12)));print('Fresh compile/link:',t.out,flush=True);t.build();print('Built:',t.base, flush=True)
    snapshot(); print('Both built; compare the two sets of artifacts before accepting C3.',flush=True)

if __name__=='__main__':
    try: main()
    except (RuntimeError,OSError,ValueError) as exc: raise SystemExit(str(exc))
