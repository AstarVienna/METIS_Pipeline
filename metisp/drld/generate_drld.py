#!/usr/bin/env python
"""
Render DRLD cards from the pymetis catalogue: one data-item card per registered, fully
resolved `DataItem` class and one recipe card per registered `Recipe`.

Everything on a card comes from the code -- the item classes themselves (name,
description, OCA keywords, HDU structure, kind) and the recipes' `InputSet`s,
`ProductSet`s, `Qc` sets and parameters. The Jinja2 templates `dataitem.tex` and
`recipe.tex` use LaTeX-friendly delimiters: `(* expression *)`, `(% block %)` and
`(# comment #)`.

Run from an environment where pymetis is importable, e.g.

    python drld/generate_drld.py --list
    python drld/generate_drld.py MASTER_IMG_FLAT_LAMP_LM metis_lm_img_flat
    python drld/generate_drld.py --all --output build/drld
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
    """ Everything the data-item template needs for one item. """
    name: str
    macro: str                      # RAW, PROD, EXTCALIB or STATCALIB, as used in the DRLD paragraphs
    description: str
    oca_keywords: list[str]
    created_by: list[str] = field(default_factory=list)
    input_for: list[str] = field(default_factory=list)
    structure: list[tuple[str, str]] = field(default_factory=list)   # (C type, comment)

    @property
    def is_raw(self) -> bool:
        return self.macro == 'RAW'


@dataclass
class RecipeCard:
    """ Everything the recipe template needs for one recipe; rows are ready-made LaTeX. """
    name: str
    synopsis: str
    inputs: list[str]
    matched_keywords: list[str]
    parameters: list[str]
    algorithm: list[str]
    outputs: list[str]
    qc_parameters: list[str]


def latex(text: str) -> str:
    """ Escape free text for LaTeX. Tags inside \\PROD{} and friends are left alone. """
    return ''.join(LATEX_SPECIALS.get(char, char) for char in str(text))


def fits_keywords(keywords) -> str:
    if isinstance(keywords, str):
        keywords = [keywords]
    return ', '.join(rf'\FITS{{{keyword}}}' for keyword in keywords)


def template_pattern(template: str) -> re.Pattern:
    """ A regex matching every resolved tag a (partial) template can stand for. """
    escaped = re.escape(template)
    return re.compile('^' + re.sub(r'\\\{\w+\\\}', '[A-Z0-9]+', escaped) + '$')


class Catalogue:
    """ The registered, fully resolved data items and the recipes that create and consume them. """

    def __init__(self):
        self.items: dict[str, type[DataItem]] = {
            tag: item for tag, item in sorted(DataItem._registry.items()) if '{' not in tag
        }
        self.recipes: dict[str, type[Recipe]] = dict(sorted(Recipe._registry.items()))
        self.created_by: dict[str, set[str]] = {tag: set() for tag in self.items}
        self.input_for: dict[str, set[str]] = {tag: set() for tag in self.items}

        # A recipe's products are already specialized to its own tags; an input's item is
        # specialized here the same way the man page does it, and whatever placeholders the
        # data would fill at run time (e.g. `{target}`) stand for every matching tag.
        for name, recipe in self.recipes.items():
            for _, product in recipe._list_products():
                for tag in self.expand(product.name()):
                    self.created_by[tag].add(name)
            for _, input_class in recipe._list_inputs():
                for tag in self.expand(self.input_tag(recipe, input_class)):
                    self.input_for[tag].add(name)

    @staticmethod
    def input_tag(recipe: type[Recipe], input_class) -> str:
        return partial_format(input_class.Item.name(), **recipe.Impl.tag_parameters())

    def expand(self, template: str) -> list[str]:
        """ The catalogue tags a (possibly partial) template denotes. """
        if '{' not in template:
            return [template] if template in self.items else []
        pattern = template_pattern(template)
        return [tag for tag in self.items if pattern.match(tag)]

    def macro_of(self, item: type[DataItem], tag: str | None = None) -> str:
        """
        The DRLD macro of an item: static calibrations are `\\STATCALIB` whether or not a
        recipe can regenerate them; otherwise the frame group decides, a calibration being
        a product if some recipe creates it (any resolution of `tag`) and external otherwise.
        """
        if item.is_static():
            return 'STATCALIB'
        match item.frame_group():
            case cpl.ui.Frame.FrameGroup.RAW:
                return 'RAW'
            case cpl.ui.Frame.FrameGroup.PRODUCT:
                return 'PROD'
            case _:
                created = any(self.created_by[t] for t in self.expand(tag or item.name()))
                return 'PROD' if created else 'EXTCALIB'

    def reference(self, item: type[DataItem], tag: str) -> str:
        """ `\\PROD{TAG}` and friends, with placeholders shown as `<name>`. """
        shown = re.sub(r'\{(\w+)\}', r'<\1>', tag)
        return rf'\{self.macro_of(item, tag)}{{{shown}}}'

    # --- data items ---

    def item_card(self, tag: str) -> Card:
        item = self.items[tag]
        return Card(
            name=tag,
            macro=self.macro_of(item),
            description=item.description(),
            oca_keywords=sorted(item.oca_keywords()),
            created_by=sorted(self.created_by[tag]),
            input_for=sorted(self.input_for[tag]),
            structure=self.structure_of(item),
        )

    @staticmethod
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

    # --- recipes ---

    def recipe_card(self, name: str) -> RecipeCard:
        recipe = self.recipes[name]

        # Raw data first, as in the DRLD, then the calibrations alphabetically.
        inputs = []
        for _, input_class in sorted(recipe._list_inputs(),
                                     key=lambda entry: (entry[1]._group != cpl.ui.Frame.FrameGroup.RAW,
                                                        self.input_tag(recipe, entry[1]))):
            tag = self.input_tag(recipe, input_class)
            row = self.reference(input_class.Item, tag)
            if input_class.multiplicity() == 'N':
                row += ' (one or more)'
            if not input_class.required():
                row += ' (optional)'
            inputs.append(row)

        parameters = []
        for parameter in recipe.parameters:
            row = rf'\CODE{{{latex(parameter.name)}}}: {latex(parameter.description)}'
            if (alternatives := getattr(parameter, 'alternatives', None)) is not None:
                row += ' (' + ', '.join(rf'\texttt{{{latex(a)}}}' for a in alternatives) + ')'
            row += rf', default \texttt{{{latex(parameter.default)}}}'
            parameters.append(row)

        # The algorithm is free text with `code` spans; LaTeX-escape it and typeset the spans.
        algorithm = [re.sub(r'`([^`]+)`', r'\\texttt{\1}', latex(line.strip()))
                     for line in recipe._algorithm.splitlines() if line.strip()]

        return RecipeCard(
            name=name,
            synopsis=recipe._synopsis,
            inputs=inputs,
            matched_keywords=sorted(recipe._matched_keywords or ()),
            parameters=parameters,
            algorithm=algorithm,
            outputs=[self.reference(product, product.name()) for _, product in recipe._list_products()],
            qc_parameters=[rf'\QC{{{qc.name()}}}' for _, qc in recipe._list_qc_parameters()],
        )


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
        description='Render DRLD data-item and recipe cards from the pymetis catalogue.',
        formatter_class=argparse.RawTextHelpFormatter,
    )
    parser.add_argument('names', nargs='*', metavar='NAME',
                        help='data item tags and/or recipe names to render (default: none; see --all)')
    parser.add_argument('--all', '-a', action='store_true',
                        help='render every registered data item and recipe')
    parser.add_argument('--list', '-l', action='store_true',
                        help='list the catalogue tags and recipe names and exit')
    parser.add_argument('--output', '-o', type=Path,
                        help='directory to write into: items/<TAG>.tex and recipes/<name>.tex (default: stdout)')
    parser.add_argument('--debug', action='store_true',
                        help='enable debug mode (sets CPL Msg level to DEBUG)')
    args = parser.parse_args()

    if args.debug:
        Msg.set_level(Msg.Level.DEBUG)

    catalogue = Catalogue()

    if args.list:
        for tag, item in catalogue.items.items():
            print(f"{tag:<40} {item.__module__}.{item.__qualname__}")
        for name, recipe in catalogue.recipes.items():
            print(f"{name:<40} {recipe.__module__}.{recipe.__qualname__}")
        return

    if not args.all and not args.names:
        parser.error("give data item tags or recipe names, or --all, or --list")

    if args.all:
        tags, names = list(catalogue.items), list(catalogue.recipes)
    else:
        tags = [n for n in args.names if n in catalogue.items]
        names = [n for n in args.names if n in catalogue.recipes]
        if unknown := [n for n in args.names if n not in catalogue.items and n not in catalogue.recipes]:
            raise SystemExit(f"Neither a registered, fully resolved data item nor a recipe: {', '.join(unknown)}")

    env = environment()
    rendered = [('items', tag, env.get_template('dataitem.tex').render(item=catalogue.item_card(tag)))
                for tag in tags]
    rendered += [('recipes', name, env.get_template('recipe.tex').render(recipe=catalogue.recipe_card(name)))
                 for name in names]

    if args.output is None:
        for _, _, text in rendered:
            sys.stdout.write(text)
    else:
        for kind, name, text in rendered:
            (args.output / kind).mkdir(parents=True, exist_ok=True)
            (args.output / kind / f"{name}.tex").write_text(text)
        print(f"{len(tags)} item cards and {len(names)} recipe cards written to {args.output}")


if __name__ == '__main__':
    main()
