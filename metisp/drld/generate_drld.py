#!/usr/bin/env python
"""
Render DRLD data-item cards from the pymetis data item catalogue.

Every registered, fully resolved `DataItem` class becomes one LaTeX card, filled
from the class itself (name, description, OCA keywords, HDU structure) and from the
recipes that produce or consume it (derived from their `ProductSet`s and
`InputSet`s). The Jinja2 template `dataitem.tex` uses LaTeX-friendly delimiters:
`(* expression *)`, `(% block %)` and `(# comment #)`.

Run from an environment where pymetis is importable, e.g.

    python drld/generate_drld.py --list
    python drld/generate_drld.py MASTER_IMG_FLAT_LAMP_LM
    python drld/generate_drld.py --all --output build/dataitems
"""

import argparse
import re
import sys
from dataclasses import dataclass, field
from pathlib import Path

import cpl
import jinja2
from cpl.core import Image, ImageList, Msg, Table

import pymetis.instruments.metis.dataitems  # noqa: F401  (registers the catalogue)
import pymetis.instruments.metis.recipes    # noqa: F401  (registers the recipes; some items load with them)
from pymetis.engine.core.functions.format import partial_format
from pymetis.engine.dataitems import DataItem
from pymetis.engine.recipes import Recipe

HERE = Path(__file__).resolve().parent

# What a FITS extension holding each CPL type is called in the DRLD structure lists.
CPL_TYPES = {
    Image: 'hdrl_image',
    ImageList: 'cpl_imagelist',
    Table: 'cpl_table',
}

LATEX_SPECIALS = {
    '\\': r'\textbackslash{}', '&': r'\&', '%': r'\%', '$': r'\$', '#': r'\#',
    '_': r'\_', '{': r'\{', '}': r'\}', '~': r'\textasciitilde{}', '^': r'\textasciicircum{}',
}


@dataclass
class Card:
    """ Everything the template needs for one data item. """
    name: str
    macro: str                      # RAW, PROD or EXTCALIB, as used in the DRLD paragraphs
    description: str
    oca_keywords: list[str]
    created_by: list[str] = field(default_factory=list)
    input_for: list[str] = field(default_factory=list)
    structure: list[tuple[str, str]] = field(default_factory=list)   # (C type, comment)

    @property
    def is_raw(self) -> bool:
        return self.macro == 'RAW'


def latex(text: str) -> str:
    """ Escape free text for LaTeX. Tags inside \\PROD{} and friends are left alone. """
    return ''.join(LATEX_SPECIALS.get(char, char) for char in str(text))


def fits_keywords(keywords: list[str]) -> str:
    return ', '.join(rf'\FITS{{{keyword}}}' for keyword in keywords)


def template_pattern(template: str) -> re.Pattern:
    """ A regex matching every resolved tag a (partial) template can stand for. """
    escaped = re.escape(template)
    return re.compile('^' + re.sub(r'\\\{\w+\\\}', '[A-Z0-9]+', escaped) + '$')


def resolved_items() -> dict[str, type[DataItem]]:
    """ The catalogue: registered data items whose tag carries no placeholder. """
    return {tag: item for tag, item in sorted(DataItem._registry.items()) if '{' not in tag}


def expand(template: str, tags: list[str]) -> list[str]:
    """ The catalogue tags a (possibly partial) template denotes. """
    if '{' not in template:
        return [template] if template in tags else []
    pattern = template_pattern(template)
    return [tag for tag in tags if pattern.match(tag)]


def producers_and_consumers(tags: list[str]) -> tuple[dict[str, set[str]], dict[str, set[str]]]:
    """
    Which recipes create and which consume each tag, from the recipes' declarations.

    A recipe's products are already specialized to its own tags; an input's item is
    specialized here the same way the man page does it, and whatever placeholders the
    data would fill at run time (e.g. `{target}`) stand for every matching tag.
    """
    created_by: dict[str, set[str]] = {tag: set() for tag in tags}
    input_for: dict[str, set[str]] = {tag: set() for tag in tags}

    for recipe in Recipe._registry.values():
        impl = recipe.Impl
        for _, product in impl.ProductSet.list_classes():
            for tag in expand(product.name(), tags):
                created_by[tag].add(recipe._name)
        for _, input_class in impl.InputSet.list_input_classes():
            template = partial_format(input_class.Item.name(), **impl.tag_parameters())
            for tag in expand(template, tags):
                input_for[tag].add(recipe._name)

    return created_by, input_for


def structure_of(item: type[DataItem]) -> list[tuple[str, str]]:
    """ The CPL-level structure of the item's FITS file, from its schema. """
    rows = []
    for extension, klass in item.schema().items():
        if klass is None:
            rows.append(('cpl_propertylist * keywords', f'Primary keywords ({extension})'))
        else:
            rows.append((f'{CPL_TYPES.get(klass, klass.__name__.lower())} * {extension.lower().replace(".", "_")}',
                         f'Extension {extension}'))
    rows.append(('cpl_propertylist * plistarray[]', 'Extension keywords'))
    return rows


def macro_of(item: type[DataItem], created_by: set[str]) -> str:
    match item.frame_group():
        case cpl.ui.Frame.FrameGroup.RAW:
            return 'RAW'
        case cpl.ui.Frame.FrameGroup.PRODUCT:
            return 'PROD'
        case _:
            # A calibration is a product if some recipe creates it, external otherwise.
            # The DRLD also distinguishes static calibrations (\STATCALIB), which the
            # classes do not record yet.
            return 'PROD' if created_by else 'EXTCALIB'


def build_cards(only: list[str] | None = None) -> list[Card]:
    items = resolved_items()
    tags = list(items)
    created_by, input_for = producers_and_consumers(tags)

    selected = tags if only is None else only
    unknown = [tag for tag in selected if tag not in items]
    if unknown:
        raise SystemExit(f"Not a registered, fully resolved data item: {', '.join(unknown)}")

    return [
        Card(
            name=tag,
            macro=macro_of(items[tag], created_by[tag]),
            description=items[tag].description(),
            oca_keywords=sorted(items[tag].oca_keywords()),
            created_by=sorted(created_by[tag]),
            input_for=sorted(input_for[tag]),
            structure=structure_of(items[tag]),
        )
        for tag in selected
    ]


def environment() -> jinja2.Environment:
    env = jinja2.Environment(
        loader=jinja2.FileSystemLoader(HERE),
        variable_start_string='(*', variable_end_string='*)',
        block_start_string='(%', block_end_string='%)',
        comment_start_string='(#', comment_end_string='#)',
        trim_blocks=True, lstrip_blocks=True, keep_trailing_newline=True,
        autoescape=False, undefined=jinja2.StrictUndefined,
    )
    env.filters['latex'] = latex
    env.filters['fits'] = fits_keywords
    return env


def main() -> None:
    parser = argparse.ArgumentParser(
        description='Render DRLD data-item cards from the pymetis catalogue.',
        formatter_class=argparse.RawTextHelpFormatter,
    )
    parser.add_argument('dataitems', nargs='*', metavar='TAG',
                        help='data item tags to render (default: none; see --all)')
    parser.add_argument('--all', '-a', action='store_true',
                        help='render every registered, fully resolved data item')
    parser.add_argument('--list', '-l', action='store_true',
                        help='list the catalogue tags and exit')
    parser.add_argument('--output', '-o', type=Path,
                        help='directory to write one <TAG>.tex per item into (default: stdout)')
    parser.add_argument('--debug', action='store_true',
                        help='enable debug mode (sets CPL Msg level to DEBUG)')
    args = parser.parse_args()

    if args.debug:
        Msg.set_level(Msg.Level.DEBUG)

    if args.list:
        for tag, item in resolved_items().items():
            print(f"{tag:<40} {item.__module__}.{item.__qualname__}")
        return

    if not args.all and not args.dataitems:
        parser.error("give data item tags, or --all, or --list")

    cards = build_cards(None if args.all else args.dataitems)
    template = environment().get_template('dataitem.tex')

    if args.output is None:
        for card in cards:
            sys.stdout.write(template.render(item=card))
    else:
        args.output.mkdir(parents=True, exist_ok=True)
        for card in cards:
            (args.output / f"{card.name}.tex").write_text(template.render(item=card))
        print(f"{len(cards)} cards written to {args.output}")


if __name__ == '__main__':
    main()
