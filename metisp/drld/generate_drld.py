#!/usr/bin/env python
"""
Render DRLD cards from the pymetis catalogue: one data-item card per registered, fully
resolved `DataItem` class, one recipe card per registered `Recipe`, one QC-parameter card
per parameter the recipes declare -- and, with --document, the generated chapters of the
DRLD assembled into one file (a fragment, or a compilable document with --standalone).

Everything on a card comes from the code -- the item classes themselves (name,
description, OCA keywords, HDU structure, kind) and the recipes' `InputSet`s,
`ProductSet`s, `Qc` sets and parameters. The Jinja2 templates `dataitem.tex` and
`recipe.tex`, `qc.tex` and `document.tex` use LaTeX-friendly delimiters: `(* expression *)`, `(% block %)` and
`(# comment #)`.

Run from an environment where pymetis is importable, e.g.

    python drld/generate_drld.py --list
    python drld/generate_drld.py MASTER_IMG_FLAT_LAMP_LM metis_lm_img_flat
    python drld/generate_drld.py --all --output build/drld
    python drld/generate_drld.py --document build/drld.tex --standalone
    TEXINPUTS=/path/to/drld//: latexmk -pdf build/drld.tex
"""

import argparse
import datetime
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
class QcCard:
    """ One QC-parameter card of the DRLD chapter "QC Parameters". """
    name: str                       # placeholders lowercase without braces, as the DRLD writes them
    label: str
    type: str                       # double / int / string, the DRLD's spelling
    format: str                     # the DRLD's "Value" row: %.3f, %d, %s
    unit: str
    default: str
    description: str
    comment: str
    created_by: list[str] = field(default_factory=list)


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
    description: str = ''           # the author's long text, for the section before the card


@dataclass
class Group:
    """ A titled group of cards: a DRLD (sub)section. """
    title: str
    items: list = field(default_factory=list)
    kinds: list = field(default_factory=list)      # item families: sub-groups by kind (raw / calibration / product)
    recipes: list = field(default_factory=list)
    qcs: list = field(default_factory=list)


@dataclass
class Document:
    """ Everything `document.tex` needs to assemble the generated chapters. """
    item_families: list[Group]
    recipe_families: list[Group]
    qc_families: list[Group]
    standalone: bool
    version: str
    date: str


# How the DRLD groups things: recipe families in the order of chapter "Pipeline Recipes",
# keyed by the recipe subpackage; item families by band and module.
RECIPE_FAMILIES = [
    ('Detector calibrations', ('',)),
    ('LM imaging', ('lm_img',)),
    ('N imaging', ('n_img',)),
    ('LM long-slit spectroscopy', ('lm_lss',)),
    ('N long-slit spectroscopy', ('n_lss',)),
    ('IFU', ('ifu',)),
    ('High-contrast imaging', ('hci',)),
    ('Technical', ('cal', 'instrument')),
]
ITEM_KINDS = [('Raw data', 'RAW'), ('Calibrations and intermediate products', 'CALIB'), ('Final products', 'FINAL')]
QC_TYPES = {'float': 'double', 'int': 'int', 'str': 'string', 'bool': 'int'}
QC_FORMATS = {'float': '%.3f', 'int': '%d', 'str': '%s', 'bool': '%d'}


def latex(text: str) -> str:
    """ Escape free text for LaTeX; never apply it to tags, which go verbatim into \\PROD{} and friends. """
    return ''.join(LATEX_SPECIALS.get(char, char) for char in str(text))


def fits_keywords(keywords) -> str:
    if isinstance(keywords, str):
        keywords = [keywords]
    return ', '.join(rf'\FITS{{{keyword}}}' for keyword in keywords)


def template_pattern(template: str, tag_values: dict[str, set[str]]) -> re.Pattern:
    """
    A regex matching every resolved tag a (partial) template can stand for. Each
    placeholder admits only the values its tag keyword takes in the catalogue, so
    `IFU_{target}_RAW` matches `IFU_SCI_RAW` but not `IFU_RSRF_RAW`.
    """
    def alternatives(match: re.Match) -> str:
        values = sorted(tag_values.get(match.group(1), ()), key=len, reverse=True)
        return '(?:' + '|'.join(map(re.escape, values)) + ')' if values else '[A-Z0-9]+'

    return re.compile('^' + re.sub(r'\\\{(\w+)\\\}', alternatives, re.escape(template)) + '$')


class Catalogue:
    """ The registered, fully resolved data items and the recipes that create and consume them. """

    def __init__(self):
        self.items: dict[str, type[DataItem]] = {
            tag: item for tag, item in sorted(DataItem._registry.items()) if '{' not in tag
        }
        self.recipes: dict[str, type[Recipe]] = dict(sorted(Recipe._registry.items()))
        self.created_by: dict[str, set[str]] = {tag: set() for tag in self.items}
        self.input_for: dict[str, set[str]] = {tag: set() for tag in self.items}

        # The values each tag keyword takes anywhere in the catalogue, e.g. target -> SCI, STD, SKY.
        self.tag_values: dict[str, set[str]] = {}
        for item in self.items.values():
            for key, value in item.tag_parameters().items():
                self.tag_values.setdefault(key, set()).add(str(value))

        # A recipe consumes exactly the items whose class the input's Item covers (what
        # `PipelineInput` matches at run time); a product template with placeholders left
        # for the data (e.g. `{target}`) stands for each catalogue value of that tag.
        for name, recipe in self.recipes.items():
            for _, product in recipe._list_products():
                for tag in self.expand(product.name()):
                    self.created_by[tag].add(name)
            for _, input_class in recipe._list_inputs():
                for tag, item in self.items.items():
                    if issubclass(item, input_class.Item):
                        self.input_for[tag].add(name)

    @staticmethod
    def input_tag(recipe: type[Recipe], input_class) -> str:
        return partial_format(input_class.Item.name(), **recipe.Impl.tag_parameters())

    def expand(self, template: str) -> list[str]:
        """ The catalogue tags a (possibly partial) template denotes. """
        if '{' not in template:
            return [template] if template in self.items else []
        pattern = template_pattern(template, self.tag_values)
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
        """
        `\\PROD{TAG}` and friends. A tag with placeholders left for the data is written as
        the DRLD writes it, as the alternatives the catalogue offers joined by "or"
        (`\\RAW{LM_FLAT_LAMP_RAW} or \\RAW{LM_FLAT_TWILIGHT_RAW}`); with no catalogue
        entry to expand to, the placeholders are shown as `<name>`.
        """
        alternatives = self.expand(tag) if '{' in tag else []
        if alternatives:
            return ' or '.join(rf'\{self.macro_of(self.items[t], t)}{{{t}}}' for t in alternatives)
        shown = re.sub(r'\{(\w+)\}', r'<\1>', tag)
        return rf'\{self.macro_of(item, tag)}{{{shown}}}'

    @staticmethod
    def qc_shown(name: str) -> str:
        """ A QC name as the DRLD writes it: run-time placeholders lowercase, no braces. """
        return re.sub(r'\{(\w+)\}', lambda m: m.group(1).lower(), name)

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
            qc_parameters=[rf'\QC{{{self.qc_shown(qc.name())}}}' for _, qc in recipe._list_qc_parameters()],
        )


    # --- QC parameters ---

    def qc_cards(self) -> dict[str, QcCard]:
        """
        One card per distinct QC parameter name over all recipes' Qc sets (the sets as
        specialized with the recipe's own tags; a placeholder left for the data or an index
        stays, lowercase without braces, as the DRLD writes generic cards). "Created by" is
        every recipe whose set declares the name.
        """
        cards: dict[str, QcCard] = {}
        for name, recipe in self.recipes.items():
            for _, klass in recipe._list_qc_parameters():
                shown = self.qc_shown(klass.name())
                if shown in cards:
                    if name not in cards[shown].created_by:
                        cards[shown].created_by.append(name)
                    continue
                type_name = getattr(klass._type, '__name__', str(klass._type))
                cards[shown] = QcCard(
                    name=shown,
                    label=re.sub(r'[^a-z0-9]+', '_', shown.lower()).strip('_'),
                    type=QC_TYPES.get(type_name, type_name),
                    format=QC_FORMATS.get(type_name, '%s'),
                    unit='None' if klass._unit is None else str(klass._unit),
                    default='None' if klass._default is None else str(klass._default),
                    description=re.sub(r'\{(\w+)\}', lambda m: m.group(1).lower(), klass.description()),
                    comment=klass._comment or '',
                    created_by=[name],
                )
        return cards

    # --- the whole thing ---

    @staticmethod
    def recipe_family(recipe: type[Recipe]) -> str:
        parts = recipe.__module__.replace('pymetis.instruments.metis.recipes', '').strip('.').split('.')
        return parts[0] if len(parts) > 1 else ''

    @staticmethod
    def item_family(item: type[DataItem]) -> str:
        module = item.__module__.replace('pymetis.instruments.metis.dataitems.', '')
        if module.startswith('hci'):
            return 'High-contrast imaging'
        if item.tag_parameters().get('band') == 'IFU' or module.split('.')[0] in ('ifu', 'wavecal', 'rsrf'):
            return 'IFU'
        if module.split('.')[0] in ('lss', 'adc', 'synth', 'molecfit'):
            return 'Long-slit spectroscopy'
        if module.split('.')[0] in ('raw', 'masterdark', 'linearity', 'gainmap', 'badpixmap', 'common'):
            return 'Detector and common calibrations'
        return 'Imaging'

    def item_kind(self, item: type[DataItem]) -> str:
        if item.frame_group() == cpl.ui.Frame.FrameGroup.RAW:
            return 'RAW'
        if item.frame_level() == cpl.ui.Frame.FrameLevel.FINAL and item.frame_group() == cpl.ui.Frame.FrameGroup.PRODUCT:
            return 'FINAL'
        return 'CALIB'

    def document(self, standalone: bool) -> Document:
        families = ['Detector and common calibrations', 'Imaging', 'Long-slit spectroscopy', 'IFU', 'High-contrast imaging']
        item_families = []
        for title in families:
            members = [tag for tag, item in self.items.items() if self.item_family(item) == title]
            kinds = [Group(title=kind_title, items=[self.item_card(t) for t in members if self.item_kind(self.items[t]) == kind])
                     for kind_title, kind in ITEM_KINDS]
            kinds = [k for k in kinds if k.items]
            if kinds:
                item_families.append(Group(title=title, kinds=kinds))

        recipe_families, qc_families = [], []
        qc_cards = self.qc_cards()
        for title, subpackages in RECIPE_FAMILIES:
            names = [n for n, r in self.recipes.items() if self.recipe_family(r) in subpackages]
            if not names:
                continue
            cards = [self.recipe_card(n) for n in names]
            for card in cards:
                card.description = self.recipes[card.name]._long_description or ''
            recipe_families.append(Group(title=title, recipes=cards))
            qcs = [c for c in qc_cards.values() if c.created_by[0] in names]
            if qcs:
                qc_families.append(Group(title=title, qcs=qcs))

        try:
            from importlib.metadata import version
            pymetis_version = version('pymetis')
        except Exception:   # noqa: BLE001  -- an editable checkout without metadata
            pymetis_version = 'development'
        return Document(item_families=item_families, recipe_families=recipe_families, qc_families=qc_families,
                        standalone=standalone, version=pymetis_version, date=datetime.date.today().isoformat())


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
                        help='directory to write into: items/<TAG>.tex, recipes/<name>.tex, qc/<NAME>.tex (default: stdout)')
    parser.add_argument('--document', '-d', type=Path, metavar='FILE',
                        help='assemble the generated DRLD chapters (data items, recipes, QC parameters) into FILE')
    parser.add_argument('--standalone', action='store_true',
                        help='with --document: a compilable document instead of a fragment; compile with the DRLD '
                             'sources on TEXINPUTS, e.g. TEXINPUTS=/path/to/drld//: latexmk -pdf FILE')
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

    if args.document is not None:
        env = environment()
        args.document.parent.mkdir(parents=True, exist_ok=True)
        args.document.write_text(env.get_template('document.tex').render(doc=catalogue.document(args.standalone)))
        print(f"generated DRLD chapters written to {args.document}"
              + (" (standalone)" if args.standalone else " (fragment)"))
        return

    if not args.all and not args.names:
        parser.error("give data item tags or recipe names, or --all, --document, or --list")

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
    if args.all:
        rendered += [('qc', card.label, env.get_template('qc.tex').render(qc=card))
                     for card in catalogue.qc_cards().values()]

    if args.output is None:
        for _, _, text in rendered:
            sys.stdout.write(text)
    else:
        for kind, name, text in rendered:
            (args.output / kind).mkdir(parents=True, exist_ok=True)
            (args.output / kind / f"{name}.tex").write_text(text)
        counts = {kind: sum(1 for k, _, _ in rendered if k == kind) for kind in ('items', 'recipes', 'qc')}
        print(f"{counts['items']} item cards, {counts['recipes']} recipe cards and {counts['qc']} QC cards written to {args.output}")


if __name__ == '__main__':
    main()
