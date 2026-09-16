#!/usr/bin/env python
"""
Regenerate the type stubs for the compiled `cpl` and `hdrl` extension modules.

pycpl is a single pybind11 extension without `.pyi` files, so IDEs and type checkers see
none of its members and autocomplete falls back to guessing. The stubs generated here
(`pybind11-stubgen`, a dev dependency) make the real API visible; pyright picks them up
from `[tool.pyright] stubPath` in pyproject.toml.

Run from `metisp/pymetis` with the development venv whenever pycpl is bumped:

    .venv/bin/python typings/regenerate.py

A few signatures that pybind11-stubgen extracts from docstrings are not valid Python;
`PATCHES` repairs them. If a patch no longer applies after a pycpl upgrade, this script
stops and says which one, so the stubs never silently regress.
"""

import re
import subprocess
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent

# (file, old, new): exact substitutions applied after generation; each must match exactly once.
PATCHES = [
    # A parameter without a default after one with a default.
    ('cpl/core.pyi',
     'def filter(self, kernel: ..., filter: Filter, border: Border = ..., dtype: Type) -> Image:',
     'def filter(self, kernel: ..., filter: Filter, border: Border = ..., dtype: Type = ...) -> Image:'),
    ('cpl/hdrl/func.pyi',
     'stat_mask: cpl.core.Mask = None, collapse_params: Collapse) -> None:',
     'stat_mask: cpl.core.Mask = None, collapse_params: Collapse = ...) -> None:'),
]

# Parameters the bindings call `from`, which is a keyword in Python; renamed everywhere.
KEYWORD_PARAMETER = re.compile(r'(\(|, )from: ')

# C enum type names that leak from the C++ signatures -> the Python enums that wrap them.
LEAKED_TYPES = {
    'cpl/ui.pyi': {'_cpl_type_': 'cpl.core.Type'},
    'cpl/core.pyi': {'_cpl_filter_mode_': 'Filter', '_cpl_border_mode_': 'Border'},
}

# The `hdrl` package on the Python side is a thin re-export of `cpl.hdrl`; give its
# submodules stubs of their own so `from hdrl.core import Image` resolves.
HDRL_SUBMODULES = ('core', 'func', 'debug')


def generate(module: str) -> None:
    subprocess.run([sys.executable, '-m', 'pybind11_stubgen', module, '--output-dir', str(HERE)],
                   check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)


def patch() -> None:
    for relative, old, new in PATCHES:
        path = HERE / relative
        text = path.read_text()
        if text.count(old) != 1:
            sys.exit(f"{relative}: patch target found {text.count(old)} times, expected once -- "
                     f"pycpl changed; update PATCHES:\n  {old}")
        path.write_text(text.replace(old, new))
    for relative, replacements in LEAKED_TYPES.items():
        path = HERE / relative
        text = path.read_text()
        for leaked, python in replacements.items():
            text = re.sub(rf'\b{leaked}\b', python, text)
        if 'cpl.core.' in text and '\nimport cpl.core\n' not in text:
            text = text.replace('\nimport typing\n', '\nimport typing\nimport cpl.core\n', 1)
        path.write_text(text)
    for path in HERE.rglob('*.pyi'):
        text = path.read_text()
        if KEYWORD_PARAMETER.search(text):
            path.write_text(KEYWORD_PARAMETER.sub(r'\1from_: ', text))


def hdrl_submodules() -> None:
    for name in HDRL_SUBMODULES:
        (HERE / 'hdrl' / f'{name}.pyi').write_text(f'from cpl.hdrl.{name} import *\n')


def check() -> None:
    import ast
    for path in HERE.rglob('*.pyi'):
        try:
            ast.parse(path.read_text())
        except SyntaxError as e:
            sys.exit(f"{path.relative_to(HERE)}:{e.lineno}: {e.msg}")


if __name__ == '__main__':
    for module in ('cpl', 'hdrl'):
        generate(module)
    patch()
    hdrl_submodules()
    check()
    print(f"stubs regenerated under {HERE}: {sum(1 for _ in HERE.rglob('*.pyi'))} files")
