import os
import re
import json
import sys
from pathlib import Path

# --- Configuration & Setup ---
# Defined based on script location to avoid NameError and dependency on CWD
AI_DIR = Path(__file__).parent
ROOT_DIR = AI_DIR.parent

def load_config():
    cfg_path = AI_DIR / "ai_pack_config.json"
    default_cfg = {
        "source_root": ".",
        "include_ext": [".h", ".hpp", ".cpp", ".inl"],
        "ignore_dirs": ["bin", "build", ".git", "ThirdParty"],
        "max_snippet_lines": 25,
        "index_exclude_ext": [".cpp", ".c", ".cc"],
        "content_exclude_ext": [".cpp", ".c", ".cc"],
        "entry_keywords": [
            "Entry", "Entry.cpp", "WinMain", "main(",
            "Game::Launch", "RenderEngine::Tick", "RenderEngine::Create",
            "SceneManager::Update", "SceneManager::Create"
        ],
        "project_goal": "Build a modern rendering engine with GPU-driven rendering, scalable resources, and tooling support.",
        "digest_guidance": [],
        "subsystem_notes": {},
        "subsystems": {}
    }
    if cfg_path.exists():
        try:
            with open(cfg_path, 'r', encoding='utf-8') as f:
                user_cfg = json.load(f)
                default_cfg.update(user_cfg)
        except Exception as e:
            print(f"[WARN] Failed to load config: {e}")
    return default_cfg

CONFIG = load_config()

# --- Helper Functions ---

def is_ignored(path: Path):
    rel_str = path.as_posix().lower()
    parts = [p.lower() for p in path.parts]
    for ignore in CONFIG["ignore_dirs"]:
        ign = str(ignore).lower()
        if "/" in ign or "\\" in ign:
            ign_norm = ign.replace("\\", "/")
            if ign_norm in rel_str:
                return True
        else:
            if ign in parts:
                return True
    return False

def collect_files():
    files = []
    src_root = ROOT_DIR / CONFIG["source_root"]
    exts = set(CONFIG["include_ext"])
    
    if not src_root.exists():
         print(f"[WARN] Source root not found: {src_root}")
         return []

    for p in src_root.rglob("*"):
        if p.is_dir(): continue
        if p.suffix.lower() not in exts: continue
        try:
            if is_ignored(p.relative_to(ROOT_DIR)): continue
        except ValueError:
            continue
        files.append(p)
    return files

def calculate_relevance(file_path: Path, content: str, keywords: list):
    """Score a file based on filename matches and content keyword density."""
    score = 0
    # Use relative path string for matching to allow folder-scoped keywords
    try:
        rel_str = file_path.relative_to(ROOT_DIR).as_posix().lower()
    except ValueError:
        rel_str = file_path.name.lower()
        
    fname = file_path.name.lower()
    content_lower = content.lower()
    
    for kw in keywords:
        kw_lower = kw.lower()
        
        # 1. Filename match (Highest Specificity)
        if kw_lower in fname:
            score += 20
        # 2. Path match (Folder containment)
        elif kw_lower in rel_str:
            score += 10
            
        # 3. Content match
        # Improve performance: check only first 5k chars for fast filter, or count all
        if len(kw_lower) >= 4:
            count = content_lower.count(kw_lower)
            if count > 0:
                score += min(count, 5) # Cap contribution per keyword
        
    # Boost headers slightly as they define structure
    if file_path.suffix.lower() in ['.h', '.hpp']:
        score += 2

    # Boost likely entry points to recover lifecycle and dataflow
    for ek in CONFIG.get("entry_keywords", []):
        ek_lower = ek.lower()
        if ek_lower in rel_str:
            score += 8
        elif ek_lower in content_lower:
            score += 4
        
    return score

def extract_smart_snippets(content: str, keywords: list, max_lines=20, file_suffix=""):
    """Extracts relevant code blocks (structs, classes, key functions)."""
    lines = content.splitlines()
    snippets = []
    
    suffix = file_suffix.lower()

    # Regex for interesting definition starts by file type
    if suffix in [".py"]:
        def_pattern = re.compile(r'^\s*(class|def)\s+\w+', re.MULTILINE)
    elif suffix in [".lua"]:
        def_pattern = re.compile(r'^\s*(local\s+)?function\s+[\w\.:]+', re.MULTILINE)
    elif suffix in [".ps1"]:
        def_pattern = re.compile(r'^\s*function\s+\w+|^\s*(msbuild|python|premake|cmake|dotnet)\b', re.IGNORECASE | re.MULTILINE)
    elif suffix in [".bat", ".cmd"]:
        def_pattern = re.compile(r'^\s*(call|set|msbuild|python|premake5?\.exe|cmake|git|dotnet)\b', re.IGNORECASE | re.MULTILINE)
    else:
        # C++/HLSL default
        def_pattern = re.compile(
            r'^\s*((static|inline|virtual|constexpr|friend)\s+)?(class|struct|enum|namespace|cbuffer)\s+\w+'
            r'|^\s*(template\s*<[^>]+>\s*)?((static|inline|virtual|constexpr)\s+)?[\w:<>~]+\s+[\w:<>~]+\s*\([^;{]*\)\s*(const)?\s*(override|final)?\s*(;|\{)'
            r'|^\s*[\w:<>~]+\s*\([^;{]*\)\s*(const)?\s*(override|final)?\s*(;|\{)'
            r'|^\s*(Texture2D|TextureCube|SamplerState)\s*[<>\w]*\s+\w+',
            re.MULTILINE,
        )
    
    # Regex for user manual hints
    hint_pattern = re.compile(r'^\s*(//|#|--|::|rem)\s*@arch', re.IGNORECASE)

    relevant_indices = []
    
    # 1. Find explicit @arch tags (Highest priority)
    for i, line in enumerate(lines):
        if hint_pattern.search(line):
            relevant_indices.append((i, 100)) # High priority score

    # 2. Find keyword hits in definitions (Medium priority)
    keyword_roots = [k.lower() for k in keywords]
    for i, line in enumerate(lines):
        if def_pattern.match(line):
            lower_line = line.lower()
            if any(k in lower_line for k in keyword_roots):
                relevant_indices.append((i, 10))

    # 3. Fallback: extract function declarations in headers if few hits
    if len(relevant_indices) < 3:
        for i, line in enumerate(lines):
            if def_pattern.match(line):
                relevant_indices.append((i, 5))
        if not relevant_indices and suffix in [".bat", ".cmd", ".ps1", ".py", ".lua"]:
            relevant_indices.append((0, 1))

    # Sort by priority and position
    relevant_indices.sort(key=lambda x: x[1], reverse=True)
    
    # Merge overlapping regions
    merged_regions = []
    used_lines = set()
    
    for idx, score in relevant_indices:
        if idx in used_lines: continue
        
        # Determine context window
        start = max(0, idx - 2)
        end = min(len(lines), idx + max_lines)
        
        # Simple heuristic: stop at '}' if it's a short struct/func
        brace_balance = 0
        found_brace = False
        actual_end = end
        
        for k in range(idx, min(len(lines), idx + 50)): # Look ahead up to 50 lines
            line = lines[k]
            brace_balance += line.count('{') - line.count('}')
            if '{' in line: found_brace = True
            if found_brace and brace_balance <= 0:
                actual_end = k + 1
                break
        
        # Don't capture massive functions, clamp to max_lines unless it was a definition
        if actual_end - start > max_lines:
            actual_end = start + max_lines
            
        region = (start, actual_end)
        
        # Check overlap
        is_overlapping = False
        for r_start, r_end in merged_regions:
            if not (actual_end < r_start or start > r_end):
                is_overlapping = True
                break
        
        if not is_overlapping and len(merged_regions) < 5: # Limit to 5 snippets per file
            merged_regions.append(region)
            for k in range(start, actual_end): used_lines.add(k)

    # Extract text
    for start, end in sorted(merged_regions):
        code_block = "\n".join(lines[start:end])
        snippets.append(code_block)
        
    return snippets

def generate_digests():
    # Note: AI_DIR is assumed to exist as the script is running from inside it
    
    print(f"Scanning files from {ROOT_DIR}...")
    all_files = collect_files()
    print(f"Found {len(all_files)} source files.")
    
    # Pre-read files to avoid re-reading for every subsystem
    file_contents = {}
    for f in all_files:
        try:
            file_contents[f] = f.read_text(encoding='utf-8', errors='ignore')
        except:
            pass

    subsystems = CONFIG["subsystems"]
    index_exclude_ext = set(ext.lower() for ext in CONFIG.get("index_exclude_ext", []))
    content_exclude_ext = set(ext.lower() for ext in CONFIG.get("content_exclude_ext", []))
    
    for sys_name, keywords in subsystems.items():
        print(f"Generating digest for {sys_name}...")
        
        # 1. Rank files
        ranked_files = []
        for f, content in file_contents.items():
            score = calculate_relevance(f, content, keywords)
            if score > 0:
                ranked_files.append((score, f))
        
        ranked_files.sort(key=lambda x: x[0], reverse=True)
        
        # Files to list in index (structure view) - show more files for context
        index_files = []
        for score, f in ranked_files:
            if f.suffix.lower() in index_exclude_ext:
                continue
            index_files.append((score, f))
            if len(index_files) >= 60:
                break
        
        # Files to extract content from (deep dive)
        content_files = ranked_files[:20] 
        
        # 2. Build Markdown
        out_lines = []
        out_lines.append(f"# Architecture Digest: {sys_name}")
        out_lines.append(f"> Auto-generated. Focus: {', '.join(keywords)}")

        project_goal = CONFIG.get("project_goal")
        if project_goal:
            out_lines.append("\n## Project Intent")
            out_lines.append(project_goal)

        guidance = CONFIG.get("digest_guidance", [])
        if guidance:
            out_lines.append("\n## Digest Guidance")
            for note in guidance:
                out_lines.append(f"- {note}")

        notes = CONFIG.get("subsystem_notes", {}).get(sys_name, [])
        if notes:
            out_lines.append("\n## Understanding Notes")
            for note in notes:
                out_lines.append(f"- {note}")
        out_lines.append("\n## Key Files Index")
        
        for score, f in index_files:
            rel_path = f.relative_to(ROOT_DIR).as_posix()
            mark = " *(Content Included)*" if any(cf[1] == f for cf in content_files) else ""
            out_lines.append(f"- `[{score}]` **{rel_path}**{mark}")
            
        out_lines.append("\n## Evidence & Implementation Details")
        
        included_snippets = 0
        for score, f in content_files:
            # Skip snippets for excluded types to focus on interfaces/definitions.
            # If too few snippets are found, allow excluded types as fallback.
            if f.suffix.lower() in content_exclude_ext and included_snippets >= 3:
                continue

            rel_path = f.relative_to(ROOT_DIR).as_posix()
            content = file_contents[f]
            snippets = extract_smart_snippets(content, keywords, CONFIG["max_snippet_lines"], f.suffix)
            
            if snippets:
                out_lines.append(f"\n### File: `{rel_path}`")
                for i, snip in enumerate(snippets):
                    suffix = f.suffix.lower()
                    if suffix in [".hlsl", ".hlsli"]:
                        lang = "hlsl"
                    elif suffix in [".py"]:
                        lang = "python"
                    elif suffix in [".ps1"]:
                        lang = "powershell"
                    elif suffix in [".bat", ".cmd"]:
                        lang = "bat"
                    elif suffix in [".lua"]:
                        lang = "lua"
                    else:
                        lang = "cpp"
                    out_lines.append(f"```{lang}\n{snip}\n```")
                    if i < len(snippets) - 1:
                        out_lines.append("...")
                included_snippets += 1

        # 3. Write Output
        out_path = AI_DIR / f"DIGEST_{sys_name}.md"
        with open(out_path, 'w', encoding='utf-8') as f:
            f.write("\n".join(out_lines))

    print(f"Digests generated in {AI_DIR} folder.")

if __name__ == "__main__":
    generate_digests()
