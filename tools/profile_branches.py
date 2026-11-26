#!/usr/bin/env python3
"""Replay the verified compile database and show WHICH preprocessor arms are selected.
Only --write-site refreshes the checked-in report; no source file is modified.
"""
import argparse
from collections import Counter
from concurrent.futures import ThreadPoolExecutor
from html import escape
import json
from pathlib import Path
import re
from tempfile import TemporaryDirectory
from build_profile import BUILD, SRC, ROOT, HEADER, LOCK214, inputs, sha, run, check, writej

DIRECTIVE=re.compile(r'^\s*#\s*(ifdef|ifndef|if|elif|else|endif)\b(.*)')
LEX=re.compile(r'/\*|//|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'')
OPEN={'if','ifdef','ifndef'}
MARK=re.compile(r'^TXLEARN_BRANCH_PROBE_(open|arm)_(\d+)_(\d+)$')

def instrument(p, uid):
    # Preserve logical directives (backslash-newline, comments, quotes) and original __LINE__.
    lines=p.read_text(encoding='utf-8-sig').splitlines(); stack=[]; groups=[]; i=0; block=False
    fileliteral=json.dumps(str(p),ensure_ascii=False); out=['#line 1 '+fileliteral]
    while i<len(lines):
        first=i+1; part=lines[i]; i+=1
        while re.search(r'\\[ \t]*$',part) and i<len(lines):
            part=re.sub(r'\\[ \t]*$','',part)+lines[i]; i+=1
        j=0; clean=''
        while j<len(part):
            if block:
                k=part.find('*/',j)
                if k<0: break
                j=k+2; block=False
            m=LEX.search(part,j)
            if not m: clean+=part[j:]; break
            clean+=part[j:m.start()]; j=m.end(); token=m[0]
            if token=='//': break
            if token=='/*': clean+=' '; block=True
            else: clean+=token
        hit=DIRECTIVE.match(clean); op=hit[1] if hit else None
        if op in OPEN:
            stack.append([]); groups.append(stack[-1]); out.extend([f'TXLEARN_BRANCH_PROBE_open_{uid}_{first}',f'#line {first} '+fileliteral])
        out.extend(lines[first-1:i])
        if hit:
            check(not block,f'Cannot insert marker inside multiline comment at {p.name}:{first}')
            if op=='endif': stack.pop()
            else:
                stack[-1].append((first,op,' '.join(hit[2].split()))); out.append(f'TXLEARN_BRANCH_PROBE_arm_{uid}_{first}')
            out.append(f'#line {i+1} '+fileliteral)
    check(not block and not stack,'Unbalanced directives/comments: '+p.name)
    return '\n'.join(out)+'\n', groups

def audit():
    ps=inputs()+[HEADER]; site=ROOT/'demo_threadx'; evidence=json.loads((BUILD/'verified.json').read_text(encoding='utf-8'))
    check(evidence['source_lock']==LOCK214 and evidence['autoconf_sha256']==sha(HEADER)
        and evidence['compile_db_sha256']==sha(BUILD/'compile_commands.json') and evidence['elf_sha256']==sha(BUILD/'demo_threadx.elf'),
        'Stale build evidence: run build_profile.py first')
    db=json.loads((BUILD/'compile_commands.json').read_text(encoding='utf-8')); groups={}; total=Counter()
    with TemporaryDirectory(prefix='cpp-replay-',dir=BUILD) as scratch:
        mirror=Path(scratch)
        for uid,p in enumerate(ps):
            txt,groups[p.name]=instrument(p,uid); (mirror/p.name).write_bytes(txt.encode('utf8'))
        def replay(entry):
            p=Path(entry['file']); args=entry['arguments']; flags=args[:args.index('-c') if '-c' in args else args.index('-E')]
            # GAS metadata is not an input to CPP. The original explicit includes and ABI stay intact.
            real=run([*flags,'-E','-P',p],BUILD)
            flags=[str(mirror) if a==str(SRC) else str(mirror/HEADER.name) if a==str(HEADER) else a for a in flags]
            sampled=run([*flags,'-E','-P',mirror/p.name],BUILD); kept=[]; counts=Counter()
            for line in sampled.splitlines():
                m=MARK.fullmatch(line.strip())
                if m: counts[(m[1],int(m[2]),int(m[3]))]+=1
                elif line.strip(): kept.append(line.strip())
            expected=[line.strip() for line in real.splitlines() if line.strip()]
            if kept!=expected:
                where=next((i for i,(x,y) in enumerate(zip(kept,expected)) if x!=y),min(len(kept),len(expected)))
                raise RuntimeError(f'CPP output divergence at {p.name}, row {where}: {kept[where:where+1]} != {expected[where:where+1]}')
            check(counts['arm',len(ps)-1,groups[HEADER.name][0][0][0]]==1,'No explicit include observed for '+p.name)
            return counts
        active=db+[json.loads((BUILD/'linker-cpp.json').read_text(encoding='utf8'))]; check(len(active)==200,'Unexpected CPP input count')
        with ThreadPoolExecutor(max_workers=6) as pool:
            for counts in pool.map(replay,active): total.update(counts)
    summary={}; outcomes=Counter()
    for uid,p in enumerate(ps):
        result=[]
        for group in groups[p.name]:
            remaining=total['open',uid,group[0][0]]
            for line,op,expr in group:
                chosen=total['arm',uid,line]; check(remaining>=chosen>=0,'Invalid branch counts')
                state=('not-reached' if not remaining else 'taken' if chosen==remaining else 'mixed' if chosen else 'skipped')
                result.append(dict(line=line,op=op,expr=expr,reached=remaining,selected=chosen,state=state)); remaining-=chosen
                if op=='else': check(remaining==0,'Unaccounted #else')
                if p!=HEADER and op!='else': outcomes[state]+=1
        summary[p.name]=sorted(result,key=lambda r:r['line'])
    check(sum(outcomes.values())==935 and sum(bool(summary[p.name]) for p in ps[:-1])==148,'Original condition scope changed')
    for f,sym in (('tx_misra.S','__ARM_FP'),('tx_port.h','TX_THREAD_INIT')):
        matches=[r for r in summary[f] if r['expr']==sym]; check(matches and all(r['selected'] for r in matches), 'Lost positive control: '+f+' / '+sym)
    return evidence,summary,dict(outcomes),site

LABEL={'not-reached':'未求值','taken':'选中','mixed':'随包含现场而不同','skipped':'未选中'}

def page(data,counts,info):
    seq=sorted(data,key=lambda n:(n!='autoconf.h',n)); cards=[]
    for name in seq:
        rows=data[name]; tests=sum(r['op']!='else' for r in rows); body=''
        if not rows: body='<p>此文件没有直接的 C 预处理条件。'+('它通过 GNU CPP 显式读取项目配置；本文件自身没有条件分支。' if name.endswith(('.s','.ld')) else '它仍可能使用所包含头文件选定的宏。')+'</p>'
        else:
            body='<div class="branch-scroll"><table><thead><tr><th>源码行</th><th>条件 / 分支</th><th>本配置的选择</th><th>选中 / 到达</th></tr></thead><tbody>'
            for r in rows:
                line=r['line']; state=r['state']; body+=f'<tr id="L-{name}-{line}"><td>L{line}</td><td><code>#{r["op"]} {escape(r["expr"])}</code></td><td>{LABEL[state]}</td><td>{r["selected"]} / {r["reached"]}</td></tr>'
            body+='</tbody></table></div>'
        cards.append(f'<details class="branch-file" id="file-{name}"><summary><code>{name}</code> · {tests} 处测试</summary><p><a href="../main/{name}">本仓库阅读副本</a></p>{body}</details>')
    totals=' · '.join(f'{LABEL[k]} {counts.get(k,0)}' for k in ('taken','skipped','mixed','not-reached'))
    return '''<!doctype html><html lang="zh-CN"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>#215 实际分支 · ThreadX</title><link rel="stylesheet" href="assets/reader.css"><link rel="stylesheet" href="assets/profile.css"></head><body><a class="skip" href="#main">跳到正文</a><header id="top"><div class="bar"><a class="brand" href="index.html">ThreadX / 源码阅读</a><span class="mark">GNU · 第 215 份</span></div></header><main id="main"><section class="hero"><p class="eyebrow">配置证据 / 条件分支</p><h1>这一行，到底走哪边？</h1><p class="lead">选择文件，按原行号看 #215 的实际配置。这里统计预处理分支，不是板上覆盖率，也不是每个对象都进入了 ELF。</p><nav class="quick"><a href="autoconf.html">先读第 215 份导读</a><a href="macro-index.html">查宏是什么</a><a href="index.html">回总目录</a></nav></section><section class="how"><h2>怎样读结果</h2><p>原 214 份共 935 处测试：'''+totals+'''。<code>#else</code> 另列不重复计数；新增头的检查也单列。0 / 0 不是“条件为假”，而是外层没进入、前一支已选，或这份头从未包含。重复包含的头保护、每个 .c 自己的初始化标记，本来就可以出现不同结果。</p><p>按 199 条 C / 汇编命令及 1 条通用链接脚本预处理命令重放 GCC <code>-E -P</code>：在临时副本插入无副作用的分支标记，保留原文件/行号；去掉标记与空行后，逐行比对未插标记的输出。表中是重放次数，不是翻译单元数；头保护优化可能省去的重复测试，在探针中也可见。没有以此重放的内容参与链接。</p><p>工具链：'''+escape(info['compiler'])+'''。<a href="#file-autoconf.h">项目头</a> · <a href="#file-tx_misra.c">提前选支的 C 文件</a> · <a href="#file-tx_port.h">端口派生</a> · <a href="#file-tx_misra.S">汇编实现</a> · <a href="#file-tx_user_sample.h">未包含的模板</a></p></section><section aria-label="逐文件分支">'''+ '\n'.join(cards)+'''</section></main><footer><a href="#top">回到顶部 ↑</a></footer><script>function reveal(){const t=document.getElementById(location.hash.slice(1));if(t){const d=t.closest('details');if(d)d.open=true}}addEventListener('hashchange',reveal);reveal()</script></body></html>\n'''

def main():
    ap=argparse.ArgumentParser(description=__doc__); ap.add_argument('--write-site',action='store_true'); args=ap.parse_args()
    ev,data,counts,site=audit(); writej(BUILD/'branches.json',dict(source_lock=LOCK214,profile=ev['autoconf_sha256'],original_tests=counts,files=data))
    html=page(data,counts,ev); dest=(site if args.write_site else BUILD)/'profile-branches.html'; dest.write_bytes(html.encode('utf-8'))
    print('Verified 200 CPP replays; original tests:',counts,'; files:',len(data),'\n',dest)

if __name__=='__main__':
    try: main()
    except (RuntimeError,OSError) as e: raise SystemExit(str(e))
