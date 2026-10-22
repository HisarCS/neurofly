#!/usr/bin/env python3
"""Verify that relative href/src targets in docs/**/*.html exist on disk.

Skips absolute URLs (http, https, mailto, tel, data, javascript), pure
fragments and protocol-relative links. Query strings and fragments are
stripped. Root-relative links ("/x") resolve against docs/. Exit 1 on any
broken link.
"""
import sys
from html.parser import HTMLParser
from pathlib import Path
from urllib.parse import unquote, urlsplit

DOCS = Path(__file__).resolve().parent.parent / "docs"
SKIP = ("http:", "https:", "mailto:", "tel:", "data:", "javascript:", "//", "#")


class Links(HTMLParser):
    def __init__(self):
        super().__init__()
        self.links = []

    def handle_starttag(self, tag, attrs):
        for k, v in attrs:
            if k in ("href", "src") and v:
                self.links.append((self.getpos()[0], v.strip()))


def main():
    broken = 0
    for page in sorted(DOCS.rglob("*.html")):
        p = Links()
        p.feed(page.read_text(encoding="utf-8", errors="replace"))
        for line, link in p.links:
            if link.lower().startswith(SKIP) or not link:
                continue
            path = unquote(urlsplit(link).path)
            if not path:
                continue
            target = DOCS / path.lstrip("/") if path.startswith("/") else page.parent / path
            if target.is_dir():
                target = target / "index.html"
            if not target.exists():
                print(f"BROKEN {page.relative_to(DOCS.parent)}:{line}: {link}")
                broken += 1
    print(f"{broken} broken link(s)" if broken else "all relative links ok")
    return 1 if broken else 0


if __name__ == "__main__":
    sys.exit(main())
