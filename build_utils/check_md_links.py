#!/usr/bin/env python3
"""Check the relative links in the tracked Markdown files.

Usage: python3 build_utils/check_md_links.py

Every [text](target) link whose target is not a URL (scheme:...) or an
in-page anchor (#...) must point to an existing file or directory, relative
to the Markdown file. Fenced code blocks are skipped, and anchors in file
links are not checked. External URLs are not checked here (they are slow
and flaky in CI). Exits with 1 if a link is broken.
"""

import os
import re
import subprocess
import sys

def main():
  repo = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
  files = subprocess.run(["git", "-C", repo, "ls-files", "*.md"],
                         capture_output=True, text=True,
                         check=True).stdout.split()
  missing = 0
  links = 0
  for f in files:
    with open(os.path.join(repo, f), encoding="utf-8") as md:
      text = md.read()
    text = re.sub(r"```.*?```", "", text, flags=re.S)
    for m in re.finditer(r"\]\(([^)\s]+)\)", text):
      target = m.group(1)
      if re.match(r"[a-zA-Z][a-zA-Z0-9+.-]*:", target) or target.startswith("#"):
        continue
      links += 1
      path = target.split("#")[0]
      full = os.path.normpath(os.path.join(repo, os.path.dirname(f), path))
      if not os.path.exists(full):
        missing += 1
        print(f"MISSING {f}: {target}")
  print(f"{len(files)} Markdown files, {links} relative links, {missing} missing")
  return 1 if missing else 0

if __name__ == "__main__":
  sys.exit(main())
