# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 The APP Authors
"""Independent self-check of the planning documents (planning session, phase 3).

Run from the repository root:  python docs/planning-session/checks/check_plan.py
Prints one line per check (PASS/FAIL/INFO) and exits 1 if any check fails.
"""

import csv
import re
import sys
from collections import defaultdict
from pathlib import Path

ROOT = Path( __file__ ).resolve().parents[3]
SPEC = ROOT / "docs/spec/cad-functional-requirements.md"
CHECKLIST = ROOT / "docs/IMPLEMENTATION_CHECKLIST.md"
TRACE = ROOT / "docs/traceability.csv"
FOLDERS = ROOT / "docs/FOLDER_STRUCTURE.md"
STAGES = ROOT / "docs/stages"
RISKS = ROOT / "docs/RISKS.md"
ADRS = ROOT / "docs/adr"

ID_RE = r"(?:CR|P|A|B|L|F|U|T|X|M-[A-Z0-9]+|E-[A-Z0-9]+)-\d{2}"
STEP_RE = r"S\d{1,2}\.\d{2}\.\d+[a-z]?"
FIELDS = [ "Requirements", "Modules", "Depends on", "Deliverables", "Tests", "Done when" ]

failures = []


def report( ok, name, detail = "" ):
    status = "PASS" if ok else "FAIL"
    if not ok:
        failures.append( name )
    print( f"[{status}] {name}" + ( f": {detail}" if detail else "" ) )


def info( name, detail ):
    print( f"[INFO] {name}: {detail}" )


def read( path ):
    return path.read_text( encoding = "utf-8" )


# ---------------------------------------------------------------- spec IDs
spec = read( SPEC )
spec_ids = []
spec_accept = set()
for line in spec.splitlines():
    m = re.match( rf"^- \*\*({ID_RE})\b", line ) or re.match( rf"^#{{3,5}} ({ID_RE})\b", line ) \
        or re.match( rf"^\| ({ID_RE}) \|", line )
    if m and m.group( 1 ) not in spec_ids:
        # skip the section-9 / section-15 tables, whose first column is a stage or a source ID
        spec_ids.append( m.group( 1 ) )
# Accept lines: requirement bullets contain "*Accept:*" inline; headings carry it in a following paragraph.
current = None
for line in spec.splitlines():
    m = re.match( rf"^- \*\*({ID_RE})\b", line ) or re.match( rf"^#{{3,5}} ({ID_RE})\b", line )
    if m:
        current = m.group( 1 )
    elif re.match( r"^#{1,5} ", line ):
        current = None
    if current and re.search( r"\*?Accept:\*?", line ):
        spec_accept.add( current )
info( "spec", f"{len( spec_ids )} requirement IDs, {len( spec_accept )} with an Accept line" )
report( len( spec_ids ) == 160, "spec defines 160 requirement IDs", str( len( spec_ids ) ) )

# ---------------------------------------------------------------- checklist steps
lines = read( CHECKLIST ).splitlines()
steps = []          # dicts in document order
stage = None
wp = None
stage_wps = defaultdict( list )
stage_order = []
in_plan = True
for i, line in enumerate( lines ):
    if line.startswith( "## " ):
        m = re.match( r"^## Stage (S\d+):", line )
        stage = m.group( 1 ) if m else None
        if stage:
            stage_order.append( stage )
        wp = None
        continue
    m = re.match( r"^### (S\d+\.\d{2}) (.+)$", line )
    if m and stage:
        wp = m.group( 1 )
        stage_wps[stage].append( ( wp, m.group( 2 ).strip() ) )
        continue
    m = re.match( rf"^- \[([ x~])\] \*\*({STEP_RE}): (.+?)\*\*\s*$", line )
    if m:
        step = { "id": m.group( 2 ), "title": m.group( 3 ), "line": i + 1, "stage": stage, "wp": wp,
                 "fields": {}, "order": [] }
        j = i + 1
        while j < len( lines ) and lines[j].startswith( "  " ):
            fm = re.match( r"^  - ([A-Za-z ]+?): (.*)$", lines[j] )
            if fm:
                step["fields"][fm.group( 1 )] = fm.group( 2 )
                step["order"].append( fm.group( 1 ) )
            j += 1
        steps.append( step )
    elif re.match( r"^- \[.\] \*\*S\d", line ):
        report( False, "step heading parse", f"line {i + 1}: {line[:80]}" )

info( "checklist", f"{len( steps )} steps in {len( stage_order )} stages" )
report( stage_order == [ f"S{n}" for n in range( 11 ) ], "stages S0..S10 in order", ", ".join( stage_order ) )

ids = [ s["id"] for s in steps ]
dups = sorted( { x for x in ids if ids.count( x ) > 1 } )
report( not dups, "step IDs unique", ", ".join( dups ) )

bad_prefix = [ s["id"] for s in steps if not s["wp"] or not s["id"].startswith( s["wp"] + "." )
               or not s["id"].startswith( s["stage"] + "." ) ]
report( not bad_prefix, "step IDs match their stage and work package", ", ".join( bad_prefix[:10] ) )

missing = [ f"{s['id']} ({', '.join( f for f in FIELDS if f not in s['fields'] )})" for s in steps
            if any( f not in s["fields"] or not s["fields"][f].strip() for f in FIELDS ) ]
report( not missing, "every step has the six fields", "; ".join( missing[:10] ) )
misordered = [ s["id"] for s in steps if [ f for f in s["order"] if f in FIELDS ] != FIELDS ]
report( not misordered, "fields in rule-080 order", ", ".join( misordered[:10] ) )
extra = sorted( { f for s in steps for f in s["order"] if f not in FIELDS } )
if extra:
    info( "extra field names", ", ".join( extra ) )

# ---------------------------------------------------------------- dependencies
pos = { s["id"]: k for k, s in enumerate( steps ) }
dep_bad = []
for k, s in enumerate( steps ):
    dep = s["fields"].get( "Depends on", "" )
    refs = re.findall( STEP_RE, dep )
    if not refs and dep.strip().lower() != "none":
        dep_bad.append( f"{s['id']}: unparsed '{dep}'" )
    for r in refs:
        if r not in pos:
            dep_bad.append( f"{s['id']} -> {r} (unknown)" )
        elif pos[r] >= k:
            dep_bad.append( f"{s['id']} -> {r} (not earlier)" )
report( not dep_bad, "no step depends on a later or unknown step", "; ".join( dep_bad[:10] ) )

# ---------------------------------------------------------------- stage gates
gate_bad = []
for st in stage_order:
    wps = stage_wps[st]
    st_steps = [ s for s in steps if s["stage"] == st ]
    if not wps or wps[-1][0] != f"{st}.99" or wps[-1][1] != "Stage gate":
        gate_bad.append( f"{st}: last work package is {wps[-1] if wps else None}" )
        continue
    gate = [ s for s in st_steps if s["wp"] == f"{st}.99" ]
    titles = " | ".join( s["title"].lower() for s in gate )
    text = " ".join( s["title"] + " " + " ".join( s["fields"].values() ) for s in gate ).lower()
    if st_steps[-1]["title"] != "Human review checkpoint: stop and wait for approval":
        gate_bad.append( f"{st}: last step is not the human review checkpoint" )
    for need, words in ( ( "full test suite", [ "test suite" ] ), ( "benchmarks", [ "benchmark" ] ),
                         ( "traceability", [ "traceability" ] ), ( "docs", [ "documentation", "docs" ] ) ):
        if not any( w in text for w in words ):
            gate_bad.append( f"{st}: gate lacks '{need}'" )
report( not gate_bad, "every stage ends with Sx.99 gate incl. tests, benchmarks, traceability, docs, review",
        "; ".join( gate_bad ) )

# ---------------------------------------------------------------- coverage
req_steps = defaultdict( list )
unknown_req = []
for s in steps:
    for r in re.findall( ID_RE, s["fields"].get( "Requirements", "" ) ):
        if r not in spec_ids:
            unknown_req.append( f"{s['id']}:{r}" )
        if s["id"] not in req_steps[r]:
            req_steps[r].append( s["id"] )
uncovered = [ r for r in spec_ids if r not in req_steps ]
report( not uncovered, "every spec ID in at least one step's Requirements", ", ".join( uncovered ) )
report( not unknown_req, "no step cites an ID that is not in the spec", ", ".join( unknown_req[:10] ) )

no_ids = [ s["id"] for s in steps if not re.findall( ID_RE, s["fields"].get( "Requirements", "" ) ) ]
if no_ids:
    info( "steps with no requirement ID (prose only)", f"{len( no_ids )}: " + ", ".join( no_ids[:12] ) )

accept_tags = defaultdict( list )
for s in steps:
    for tag in re.findall( r"\(Accept: ([^)]+)\)", s["fields"].get( "Tests", "" ) ):
        for t in re.split( r"[,;]\s*", tag ):
            m = re.match( rf"({ID_RE})([a-z]?)$", t.strip() )
            if m:
                accept_tags[m.group( 1 )].append( ( m.group( 2 ), s["id"] ) )
no_accept = [ r for r in sorted( spec_accept ) if r not in accept_tags ]
report( not no_accept, "every requirement with an Accept line has a tagged test", ", ".join( no_accept ) )
stray = [ r for r in accept_tags if r not in spec_accept ]
report( not stray, "no Accept tag on a requirement without an Accept line", ", ".join( stray ) )
info( "accept tags", f"{sum( len( v ) for v in accept_tags.values() )} tags over {len( accept_tags )} requirements" )

# ---------------------------------------------------------------- traceability.csv
with TRACE.open( encoding = "utf-8", newline = "" ) as fh:
    reader = csv.DictReader( fh )
    cols = reader.fieldnames
    rows = list( reader )
report( cols == [ "id", "title", "source", "stage", "status", "steps", "tests", "notes" ], "CSV columns", str( cols ) )
csv_ids = [ r["id"] for r in rows ]
report( sorted( csv_ids ) == sorted( spec_ids ) and len( csv_ids ) == len( set( csv_ids ) ),
        "one CSV row per spec ID",
        f"missing {sorted( set( spec_ids ) - set( csv_ids ) )}, extra {sorted( set( csv_ids ) - set( spec_ids ) )}" )
report( all( r["status"] == "not-started" for r in rows ), "all CSV statuses not-started" )
csv_bad = []
csv_extend = {}
for r in rows:
    listed = [ x.strip() for x in r["steps"].split( ";" ) if x.strip() ]
    if any( x not in pos for x in listed ):
        csv_bad.append( f"{r['id']}: unknown step" )
    if sorted( listed ) != sorted( req_steps.get( r["id"], [] ) ):
        csv_bad.append( f"{r['id']}: steps differ from checklist" )
    stages_csv = set( x.strip() for x in r["stage"].split( ";" ) )
    stages_steps = { steps[pos[x]]["stage"] for x in listed if x in pos }
    # "stage" names the delivering stage(s); every one of them must have a step citing the ID.
    if not stages_csv <= stages_steps:
        csv_bad.append( f"{r['id']}: stage '{r['stage']}' has no step in {sorted( stages_csv - stages_steps )}" )
    if stages_steps - stages_csv:
        csv_extend[r["id"]] = sorted( stages_steps - stages_csv, key = lambda x: int( x[1:] ) )
report( not csv_bad, "CSV steps match checklist; every CSV stage has a step citing the ID", "; ".join( csv_bad[:12] ) )
info( "IDs also cited by steps outside their CSV stage (regression or extension steps)",
      "; ".join( f"{k}: {','.join( v )}" for k, v in csv_extend.items() ) )

# ---------------------------------------------------------------- coverage table in checklist
text = read( CHECKLIST )
cov = text.split( "<!-- coverage:start -->" )[1].split( "<!-- coverage:end -->" )[0]
cov_bad = []
cov_rows = {}
for line in cov.splitlines():
    m = re.match( rf"^\| ({ID_RE}) \| [^|]* \| ([^|]*) \|", line )
    if m:
        cov_rows[m.group( 1 )] = [ x.strip() for x in m.group( 2 ).split( "," ) if x.strip() ]
for r in spec_ids:
    if r not in cov_rows:
        cov_bad.append( f"{r}: no row" )
    elif sorted( cov_rows[r] ) != sorted( req_steps.get( r, [] ) ):
        cov_bad.append( f"{r}: steps differ" )
report( not cov_bad, "coverage table matches the steps", "; ".join( cov_bad[:12] ) )
report( "**Uncovered requirements:** none." in cov, "coverage table's uncovered list is empty" )

# ---------------------------------------------------------------- step and ADR cross-references
adr_files = { p.name[:4]: p for p in ADRS.glob( "[0-9][0-9][0-9][0-9]-*.md" ) }
xref_bad = []
docs_to_scan = [ CHECKLIST, TRACE, RISKS, FOLDERS ] + sorted( STAGES.glob( "S*.md" ) ) + sorted( ADRS.glob( "*.md" ) )
for p in docs_to_scan:
    body = read( p )
    for ref in set( re.findall( rf"\b({STEP_RE})\b", body ) ):
        # S2.03.4 / S2.03.4a are the format examples in the prompt and in rule 080
        if ref not in pos and not ( ref.startswith( "S2.03.4" ) and "letter suffix" in body ):
            xref_bad.append( f"{p.name}: step {ref}" )
    for ref in set( re.findall( r"\bADRs? (\d{4})\b", body ) ) | set( re.findall( r"\bADR (\d{4})", body ) ):
        if ref not in adr_files and not ( "0034" <= ref <= "0041" ):
            xref_bad.append( f"{p.name}: ADR {ref}" )
    for ref in set( re.findall( r"docs/adr/(\d{4})-([a-z0-9-]+)\.md", body ) ):
        if ref[0] in adr_files and adr_files[ref[0]].stem != f"{ref[0]}-{ref[1]}" and ref[0] < "0034":
            xref_bad.append( f"{p.name}: adr path {ref[0]}-{ref[1]}.md" )
report( not xref_bad, "every step ID and ADR number cited in the plan docs exists", "; ".join( sorted( xref_bad )[:15] ) )

# ---------------------------------------------------------------- stage plans (4.4)
stage_bad = []
spec_exit = {}
for line in spec.splitlines():
    m = re.match( r"^\| \*\*(S\d+)\*\* \| ([^|]+) \| ([^|]+) \| (.+) \|\s*$", line )
    if m:
        spec_exit[m.group( 1 )] = m.group( 4 ).strip()
for st in stage_order:
    p = STAGES / f"{st}.md"
    if not p.exists():
        stage_bad.append( f"{st}: missing" )
        continue
    body = read( p )
    for sec in [ "## Goal", "## Requirements delivered", "## Work packages", "## Entry criteria",
                 "## Exit criteria", "## Key risks", "## Golden test data" ]:
        if sec not in body:
            stage_bad.append( f"{st}: no '{sec}'" )
    wp_body = body.split( "## Work packages" )[1].split( "\n## " )[0] if "## Work packages" in body else ""
    listed = re.findall( r"^- (S\d+\.\d{2}) (.+)$", wp_body, re.M )
    if [ ( a, b.strip() ) for a, b in listed ] != stage_wps[st]:
        stage_bad.append( f"{st}: work packages differ from checklist" )
    exit_body = body.split( "## Exit criteria" )[1].split( "\n## " )[0] if "## Exit criteria" in body else ""
    want = re.sub( r"\*\*", "", spec_exit.get( st, "" ) )
    have = re.sub( r"\*\*", "", " ".join( l[2:] for l in exit_body.splitlines() if l.startswith( "> " ) ) )
    if not want or want.strip() != have.strip():
        stage_bad.append( f"{st}: exit criteria not copied verbatim from spec section 9" )
    goal = body.split( "## Goal" )[1].split( "\n## " )[0].strip() if "## Goal" in body else ""
    sentences = len( re.findall( r"[.!?](\s|$)", goal ) )
    if sentences > 2 or sentences == 0:
        stage_bad.append( f"{st}: goal has {sentences} sentences" )
    count = len( [ s for s in steps if s["stage"] == st ] )
    m = re.search( r"\((\d+) steps", body )
    if not m or int( m.group( 1 ) ) != count:
        stage_bad.append( f"{st}: header step count {m.group( 1 ) if m else '?'} vs {count}" )
report( not stage_bad, "stage plans meet section 4.4", "; ".join( stage_bad ) )

# ---------------------------------------------------------------- risks (4.6)
risk_text = read( RISKS )
header_ok = "| ID | Risk | Likelihood | Impact | Mitigation | Owner |" in risk_text
required = { "topological naming": "topological naming", "OCCT fillet/shell": "fillet and shell",
             "sketch solver convergence": "sketch solver convergence", "mate solver performance": "mate solver performance",
             "push-and-shove router": "push-and-shove", "polygon pour": "polygon pour",
             "large-assembly memory": "large-assembly memory", "C++26 support gaps": "c++26 compiler support",
             "Qt + vcpkg build times": "qt + vcpkg build times", "ngspice and Chrono": "ngspice and chrono",
             "file-format evolution": "file-format evolution" }
missing_risk = [ k for k, v in required.items() if v not in risk_text.lower() ]
risk_rows = [ l for l in risk_text.splitlines() if re.match( r"^\| R-\d+ ", l ) ]
bad_rows = [ l[:8] for l in risk_rows if l.count( "|" ) != 7 ]
report( header_ok and not missing_risk and not bad_rows, "RISKS.md meets section 4.6",
        f"header {header_ok}, missing {missing_risk}, malformed {bad_rows}" )
info( "risks", f"{len( risk_rows )} rows" )

# ---------------------------------------------------------------- deliverable paths vs FOLDER_STRUCTURE.md
tree = read( FOLDERS ).split( "```" )[1]
folders = set()
stack = []
for line in tree.splitlines():
    m = re.match( r"^((?:│   |    )*)[├└]── (\S+)", line )
    if not m:
        continue
    depth = len( m.group( 1 ) ) // 4
    name = m.group( 2 )
    stack = stack[:depth] + [ name ]
    if name.endswith( "/" ):
        folders.add( "".join( stack ) )
# "tests/core/ … cli/" stands for one folder per module; expand it.
modules = [ "core", "platform", "doc", "geom", "sketch", "part", "asm", "drw", "ecad/lib", "ecad/sch", "ecad/pcb",
            "ecad/out", "link", "sim", "io", "render", "ui", "cli" ]
folders |= { f"tests/{m}/" for m in modules } | { "tests/ecad/" }
# "tests/core/ … cli/<module>/spikes/" stands for a spikes/ folder in any module test folder.
if any( f.endswith( "<module>/spikes/" ) for f in folders ):
    folders |= { f"tests/{m}/spikes/" for m in modules }
repo_roots = { "src/", "tests/", "docs/", "scripts/", "libraries/", "build/", "bench/", "third_party/" }
# Fixture and generated-data folders whose sub-folders are data, not structure.
data_dirs = ( "tests/data/expected/", "tests/data/format/", "tests/data/snapshots/", "tests/data/import/",
              "tests/data/scripts/", "docs/format/schemas/", "docs/commands/" )
path_bad = []
unlisted = defaultdict( list )
skipped = set()
for s in steps:
    for field in ( "Deliverables", "Tests" ):
        for p in re.findall( r"`([^`\s]+)`", s["fields"].get( field, "" ) ):
            p = p.split( "{" )[0].split( "<" )[0].rstrip( "*" )
            if "/" not in p or not re.match( r"^[a-z_.]+/", p ):
                continue
            first = p.split( "/" )[0] + "/"
            if first not in repo_roots:
                skipped.add( p )
                continue
            parts = p.split( "/" )
            n = len( parts ) if p.endswith( "/" ) else len( parts ) - 1
            dirs = [ "/".join( parts[:k] ) + "/" for k in range( 1, n + 1 ) ]
            dirs = [ d for d in dirs if d != "/" and "//" not in d ]
            for d in dirs:
                if d in folders or d in repo_roots:
                    continue
                if d.startswith( data_dirs ):
                    break
                if d.rsplit( "/", 2 )[0] + "/" in folders or d.count( "/" ) > 2:
                    unlisted[d].append( s["id"] )
                else:
                    path_bad.append( f"{s['id']}: {p} ({d} not in tree)" )
                break
report( not path_bad, "deliverable paths fall inside top-level folders listed in FOLDER_STRUCTURE.md",
        "; ".join( path_bad[:20] ) )
info( "skipped project-relative or module-name paths", ", ".join( sorted( skipped ) ) )
report( not unlisted, "every sub-folder used by a step is in the FOLDER_STRUCTURE.md tree",
        f"{len( unlisted )} unlisted" )
for d in sorted( unlisted ):
    first = min( unlisted[d], key = lambda x: pos[x] )
    info( "unlisted", f"{d} [{steps[pos[first]]['stage']}] first {first}: {steps[pos[first]]['title'][:70]}" )

# ---------------------------------------------------------------- per-stage statistics
print()
print( "Stage | steps | work packages | requirement IDs cited" )
for st in stage_order:
    st_steps = [ s for s in steps if s["stage"] == st ]
    st_ids = { r for s in st_steps for r in re.findall( ID_RE, s["fields"].get( "Requirements", "" ) ) }
    print( f"{st} | {len( st_steps )} | {len( stage_wps[st] )} | {len( st_ids )}" )
spikes = [ s["id"] for s in steps if s["title"].lower().startswith( "spike" ) ]
print( f"Total | {len( steps )} | {sum( len( v ) for v in stage_wps.values() )} | {len( req_steps )}" )
print( f"Spike steps: {len( spikes )}: {', '.join( spikes )}" )

print()
print( "check_plan: " + ( "OK" if not failures else f"FAILED ({len( failures )}): " + "; ".join( failures ) ) )
sys.exit( 1 if failures else 0 )
