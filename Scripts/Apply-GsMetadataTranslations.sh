#!/usr/bin/env bash
set -euo pipefail
target_root=${1:?target root required}
target_name=${2:?target name required}
seed="$target_root/$target_name.metadata.tsv"
[[ -f "$seed" ]] || { echo "Metadata translation seed not found: $seed" >&2; exit 1; }
python3 - "$target_root" "$target_name" "$seed" <<'PY'
import pathlib,sys,re
root=pathlib.Path(sys.argv[1]);name=sys.argv[2];seed=pathlib.Path(sys.argv[3])
def esc(v): return v.replace('\\','\\\\').replace('"','\\"').replace('\r','').replace('\n','\\n')
by={}
for line in seed.read_text(encoding='utf-8-sig').splitlines():
    if not line.strip() or line.startswith('#'): continue
    c,s,t=line.split('\t',2);by.setdefault(c,{})[esc(s)]=esc(t)
total=0
for c,m in by.items():
    p=root/c/f'{name}.po'
    if not p.exists(): continue
    lines=p.read_text(encoding='utf-8-sig').splitlines();current=None;changes=0
    for i,line in enumerate(lines):
        q=re.match(r'^msgid "(.*)"$',line)
        if q: current=q.group(1);continue
        if current is not None and re.match(r'^msgstr ".*"$',line):
            if current in m: lines[i]=f'msgstr "{m[current]}"';changes+=1
            current=None
    p.write_text('\n'.join(lines)+'\n',encoding='utf-8',newline='\n');total+=changes;print(f'[GsLocalization] {c} / {changes}')
print(f'[GsLocalization] metadata translations applied: {total}')
PY
