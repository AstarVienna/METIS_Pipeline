"""
This file is part of the METIS Pipeline.
Copyright (C) 2024 European Southern Observatory

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

Which keywords a recipe's implementation refers to, by reading its source: the identifiers
`kw.NAME` in the module of its Impl. Shared by the tests (every keyword is used or declared
only) and the DRLD generator (the "Read by" cross-reference of a keyword card).
"""
import inspect
import re
import sys

from .keyword import Keyword

USED = re.compile(r'\bkw\.([A-Z][A-Z0-9_]*)\b')
# The declarations themselves are not reads: strip `_matched_keywords = frozenset({...})` and the OCA sets.
DECLARATION = re.compile(r'_(?:matched|oca)_keywords\s*(?::[^=\n]*)?=\s*[^\n]*?frozenset\(\{[^}]*\}\)', re.S)


def keywords_read_in(recipe, vocabulary) -> set[Keyword]:
    """ The keywords `recipe.Impl`'s module names as `kw.NAME`, looked up in `vocabulary` (a module). """
    module = sys.modules.get(recipe.Impl.__module__)
    if module is None:
        return set()
    try:
        source = inspect.getsource(module)
    except (OSError, TypeError):
        return set()
    out = set()
    for name in set(USED.findall(DECLARATION.sub('', source))):
        keyword = getattr(vocabulary, name, None)
        if isinstance(keyword, Keyword):
            out.add(keyword)
    return out
