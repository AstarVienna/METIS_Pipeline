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
    python drld/generate_drld.py --all                      # cards into drld/build/cards/
    python drld/generate_drld.py --document --standalone --pdf   # drld/build/drld.pdf

Everything generated goes under drld/build/ (ignored by git); --pdf runs latexmk there with
the DRLD sources on TEXINPUTS (--drld DIR, default $METIS_DRLD or the `drld` checkout next
to the pipeline checkout).
"""

import argparse
import datetime
import os
import subprocess
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
BUILD = HERE / 'build'            # every generated file lands here; the directory is not version-controlled

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
    templates: list[str] = field(default_factory=list)
    has_flowchart: bool = False     # `_steps` declared: the document shows the generated flowchart after the card


@dataclass
class DprRow:
    """ One line of the DRLD DPR keywords table (`tab:dpr_keywords`). """
    catg: str
    tech: str
    type: str
    tag: str
    recipes: list[str]              # recipes taking the item on a RAW-role input


@dataclass
class ChartNode:
    key: str
    reference: str                  # \RAW{...} and friends, alternatives joined by "or"
    style: str                      # a node style of recipe_config.tex
    optional: bool = False
    step: str = ''                  # the step the box belongs to (consumed by / produced by)
    above: str = 'input'            # the node the connection sits below
    below: str = 'stop-t'           # the node the connection sits above
    fraction: float = 0.5           # position between `above` and `below`


@dataclass
class ChartStep:
    key: str
    label: str
    style: str = 'redstep'
    gap: float = 2.0                # cm below the previous step


@dataclass
class FlowChart:
    """ One per-recipe flowchart (`tikz/metis_<recipe>.tex`). """
    recipe: str
    templates: list[str]
    raw_inputs: list[str]
    calibrations: list[ChartNode]
    steps: list[ChartStep]
    products: list[ChartNode]
    first_step_gap: float
    stop_gap: float
    findings: list[str] = field(default_factory=list)


CHART_ALTERNATIVES = 2   # a flowchart box lists at most this many "or" alternatives, else the placeholders
STEP_PITCH = 1.2        # cm of vertical room per calibration or product box hanging off a connection
PLACEHOLDER_STEP = 'algorithm steps:\\ not declared'


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
    assomaps: list = field(default_factory=list)       # AssoMap objects, rendered to assomap_<mode>.tex next to the document
    dpr_rows: list[DprRow] = field(default_factory=list)
    dpr_findings: list[str] = field(default_factory=list)
    keyword_rows: list = field(default_factory=list)  # KeywordRow objects (workflows.py)
    keyword_findings: list[str] = field(default_factory=list)


# The workflows whose association maps the document shows, and which of them are drawn in two parts.
DOCUMENT_WORKFLOWS = [('metis.metis_lm_img_wkf', False), ('metis.metis_n_img_wkf', False), ('metis.metis_ifu_wkf', False),
                      ('metis.metis_lm_lss_wkf', True), ('metis.metis_n_lss_wkf', True)]
# The workflows the matched-keywords table reads: everything, as the DRLD table lists every recipe.
KEYWORD_WORKFLOWS = ['metis.metis_wkf', 'metis.metis_lm_app_wkf', 'metis.metis_lm_ravc_wkf']


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

    def reference(self, item: type[DataItem], tag: str, max_alternatives: int | None = None) -> str:
        """
        `\\PROD{TAG}` and friends. A tag with placeholders left for the data is written as
        the DRLD writes it, as the alternatives the catalogue offers joined by "or"
        (`\\RAW{LM_FLAT_LAMP_RAW} or \\RAW{LM_FLAT_TWILIGHT_RAW}`); with no catalogue
        entry to expand to, the placeholders are shown as `<name>`.
        """
        alternatives = self.expand(tag) if '{' in tag else []
        if max_alternatives is not None and len(alternatives) > max_alternatives:
            alternatives = []           # the flowcharts write <band>_<cgrph>_SCI_CENTRED rather than five items
        if alternatives:
            return ' or '.join(rf'\{self.macro_of(self.items[t], t)}{{{t}}}' for t in alternatives)
        shown = re.sub(r'\{(\w+)\}', r'<\1>', tag)
        return rf'\{self.macro_of(item, tag)}{{{shown}}}'

    @staticmethod
    def qc_shown(name: str) -> str:
        """ A QC name as the DRLD writes it: run-time placeholders lowercase, no braces. """
        return re.sub(r'\{(\w+)\}', lambda m: m.group(1).lower(), name)

    # --- flowcharts ---

    def flowchart(self, name: str) -> FlowChart:
        """
        The flowchart of a recipe (`tikz/metis_<recipe>.tex`): raw input(s) at the top, the
        steps of `Recipe._steps` down the middle, each calibration hanging off the left above
        the step consuming it, each product off the right below the step producing it, the
        frame and label. Inputs no step names hang above the first step, products no step
        names below the last; without `_steps` a single placeholder step stands in.
        """
        from pymetis.engine.recipes import Step
        recipe = self.recipes[name]
        chart = FlowChart(recipe=name, templates=list(recipe._templates), raw_inputs=[], calibrations=[],
                          steps=[], products=[], first_step_gap=0, stop_gap=0)
        declared = list(recipe._steps) or [Step(PLACEHOLDER_STEP)]
        if not recipe._steps:
            chart.findings.append(f"{name} declares no _steps; a placeholder step stands in for the algorithm")
        keys = []
        for step in declared:
            key = re.sub(r'[^a-z0-9]', '', step.label.lower())[:24] or 'step'
            while key in keys:
                key += 'x'
            keys.append(key)
            chart.steps.append(ChartStep(key=key, label=latex(step.label).replace('\n', r'\\ ')))
        consumed_at = {attr: keys[i] for i, step in enumerate(declared) for attr in step.inputs}
        produced_at = {attr: keys[i] for i, step in enumerate(declared) for attr in step.products}

        inputs = sorted(recipe._list_inputs(),
                        key=lambda e: (e[1]._group != cpl.ui.Frame.FrameGroup.RAW, self.input_tag(recipe, e[1])))
        left: dict[str, list[ChartNode]] = {key: [] for key in keys}
        for attr, inp in inputs:
            tag = self.input_tag(recipe, inp)
            ref = self.reference(inp.Item, tag, max_alternatives=CHART_ALTERNATIVES)
            if inp.multiplicity() == 'N':
                ref = r'\textsl{N} ' + ref.replace(' or ', r' \\ or \textsl{N} ')
            if inp._group == cpl.ui.Frame.FrameGroup.RAW:
                chart.raw_inputs.append(ref)
                continue
            macro = self.macro_of(inp.Item, tag)
            left[consumed_at.get(attr, keys[0])].append(ChartNode(
                key=re.sub(r'[^a-z0-9]', '', attr.lower()), reference=ref,
                style='calproduct' if macro == 'PROD' else 'external', optional=not inp.required()))
        for attr in consumed_at:
            if attr not in dict(inputs):
                chart.findings.append(f"{name}: _steps consumes `{attr}`, which the InputSet does not have")

        right: dict[str, list[ChartNode]] = {key: [] for key in keys}
        products = dict(recipe._list_products())
        for attr, product in products.items():
            tag = product.name()
            ref = self.reference(product, tag, max_alternatives=CHART_ALTERNATIVES).replace(' or ', r'\\ or ')
            science = product.frame_group() == cpl.ui.Frame.FrameGroup.PRODUCT
            right[produced_at.get(attr, keys[-1])].append(ChartNode(
                key=re.sub(r'[^a-z0-9]', '', attr.lower()), reference=ref,
                style='sciproduct' if science else 'calproduct'))
        for attr in produced_at:
            if attr not in products:
                chart.findings.append(f"{name}: _steps produces `{attr}`, which the ProductSet does not have")

        # Vertical room between consecutive anchors: enough for the boxes hanging off either side.
        above = 'input'
        for i, key in enumerate(keys):
            attached = left[key]
            outgoing = right[keys[i - 1]] if i else []
            gap = max(2.0, STEP_PITCH * (max(len(attached), len(outgoing)) + 1))
            if i == 0:
                chart.first_step_gap = gap
            else:
                chart.steps[i].gap = gap
            for j, node in enumerate(attached):
                node.step, node.above, node.below = key, above, f"step_{key}"
                node.fraction = round((j + 1) / (len(attached) + 1), 3)
                chart.calibrations.append(node)
            for j, node in enumerate(outgoing):
                node.step, node.above, node.below = keys[i - 1], above, f"step_{key}"
                node.fraction = round((j + 1) / (len(outgoing) + 1), 3)
                chart.products.append(node)
            above = f"step_{key}"
        last = right[keys[-1]]
        chart.stop_gap = max(2.5, STEP_PITCH * (len(last) + 1))
        for j, node in enumerate(last):
            node.step, node.above, node.below = keys[-1], f"step_{keys[-1]}", 'stop-t'
            node.fraction = round((j + 1) / (len(last) + 1), 3)
            chart.products.append(node)
        return chart

    # --- data items ---

    def raw_consumers(self, tag: str) -> list[str]:
        """ The recipes that take `tag` on a RAW-role input (the DPR table's Recipes column). """
        item = self.items[tag]
        return sorted(name for name in self.input_for[tag]
                      if any(inp._group == cpl.ui.Frame.FrameGroup.RAW and issubclass(item, inp.Item)
                             for _, inp in self.recipes[name]._list_inputs()))

    def dpr_rows(self, findings: list[str] | None = None) -> list[DprRow]:
        """ The DPR classification of every raw item, ordered as the DRLD table (CATG, TECH, TYPE, tag). """
        rows = []
        for tag, item in self.items.items():
            if item.frame_group() != cpl.ui.Frame.FrameGroup.RAW:
                continue
            dpr = item.dpr()
            if dpr is None:
                if findings is not None:
                    findings.append(f"{tag} declares no DPR triple")
                continue
            rows.append(DprRow(*dpr, tag=tag, recipes=self.raw_consumers(tag)))
        return sorted(rows, key=lambda r: (r.catg, r.tech, r.type, r.tag))

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
            templates=list(recipe._templates), has_flowchart=bool(recipe._steps),
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
    env.filters['raw'] = lambda tag: rf'\RAW{{{tag}}}'
    env.filters['tpl'] = lambda name: rf'\TPL{{{name}}}'
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
    parser.add_argument('--output', '-o', type=Path, metavar='DIR',
                        help='directory to write the cards into: items/<TAG>.tex, recipes/<name>.tex, qc/<NAME>.tex '
                             f'(default: {BUILD / "cards"} with --all, stdout for named cards)')
    parser.add_argument('--document', '-d', type=Path, metavar='FILE', nargs='?', const=BUILD / 'drld.tex',
                        help='assemble the generated DRLD chapters (data items, recipes, QC parameters) into FILE '
                             f'(default: {BUILD / "drld.tex"})')
    parser.add_argument('--standalone', action='store_true',
                        help='with --document: a compilable document instead of a fragment to \\input')
    parser.add_argument('--pdf', action='store_true',
                        help='with --document --standalone: run latexmk on it, into the same directory, with the DRLD '
                             'sources on TEXINPUTS')
    parser.add_argument('--assomap', nargs='+', metavar='MODULE',
                        help='render the association map(s) of EDPS workflow module(s), e.g. metis.metis_lm_img_wkf, '
                             f'into {BUILD}/assomap_<mode>.tex (needs the metis workflow package: $METIS_WORKFLOWS '
                             'or the workflows directory next to pymetis)')
    parser.add_argument('--split', action='store_true',
                        help='with --assomap: two figures per workflow, cut at the AIT/daily separator')
    parser.add_argument('--flowchart', nargs='+', metavar='RECIPE',
                        help=f'render the per-recipe flowchart skeleton(s) into {BUILD}/flowchart_<recipe>.tex '
                             '(steps from Recipe._steps where declared, else a placeholder)')
    parser.add_argument('--tables', action='store_true',
                        help=f'render the DPR keywords table and the matched-keywords summary into {BUILD}')
    parser.add_argument('--lint-flowcharts', action='store_true',
                        help='check the tags drawn in the DRLD per-recipe flowcharts (tikz/metis_*.tex under --drld) '
                             'against the recipes\' inputs and products')
    parser.add_argument('--drld', type=Path, metavar='DIR',
                        default=Path(os.environ.get('METIS_DRLD', HERE.parents[2] / 'drld')),
                        help='the DRLD sources (normal_style.tex, styles_data.tex, acronyms.tex) for --pdf '
                             '(default: $METIS_DRLD, else the drld checkout next to the pipeline checkout)')
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

    if args.assomap:
        from workflows import association_maps
        env = environment()
        BUILD.mkdir(parents=True, exist_ok=True)
        for module in args.assomap:
            for amap in association_maps(catalogue, module, split=args.split):
                path = BUILD / f"assomap_{amap.mode}.tex"
                path.write_text(env.get_template('assomap.tex').render(map=amap))
                print(f"{path}: {len(amap.columns)} tasks, {len(amap.rows)} rows, {len(amap.findings)} findings")
        return

    if args.flowchart:
        env = environment()
        BUILD.mkdir(parents=True, exist_ok=True)
        for name in args.flowchart:
            if name not in catalogue.recipes:
                parser.error(f"no recipe {name}")
            chart = catalogue.flowchart(name)
            path = BUILD / f"flowchart_{name}.tex"
            path.write_text(env.get_template('flowchart.tex').render(chart=chart))
            print(f"{path}: {len(chart.calibrations)} calibrations, {len(chart.steps)} steps, "
                  f"{len(chart.products)} products, {len(chart.findings)} findings")
        return

    if args.tables:
        from workflows import matched_keywords
        env = environment()
        BUILD.mkdir(parents=True, exist_ok=True)
        doc = Document(item_families=[], recipe_families=[], qc_families=[], standalone=False, version='', date='')
        doc.dpr_rows = catalogue.dpr_rows(doc.dpr_findings)
        (BUILD / 'dpr.tex').write_text(env.get_template('dpr.tex').render(doc=doc))
        print(f"{BUILD / 'dpr.tex'}: {len(doc.dpr_rows)} rows, {len(doc.dpr_findings)} findings")
        doc.keyword_rows = matched_keywords(catalogue, KEYWORD_WORKFLOWS, doc.keyword_findings)
        (BUILD / 'matched_keywords.tex').write_text(env.get_template('matched_keywords.tex').render(doc=doc))
        print(f"{BUILD / 'matched_keywords.tex'}: {len(doc.keyword_rows)} rows, {len(doc.keyword_findings)} findings")
        return

    if args.lint_flowcharts:
        from workflows import lint_flowchart
        charts = sorted((args.drld / 'tikz').glob('metis_*.tex'))
        if not charts:
            parser.error(f"no flowcharts found in {args.drld / 'tikz'}; give --drld DIR or set METIS_DRLD")
        problems = [line for chart in charts for line in lint_flowchart(catalogue, chart)]
        print('\n'.join(problems) if problems else "every drawn tag is an input or a product of its recipe")
        print(f"{len(charts)} flowcharts, {len(problems)} findings")
        return

    if args.document is not None:
        env = environment()
        args.document.parent.mkdir(parents=True, exist_ok=True)
        doc = catalogue.document(args.standalone)
        doc.dpr_rows = catalogue.dpr_rows(doc.dpr_findings)
        try:
            from workflows import association_maps, matched_keywords
            for module, split in DOCUMENT_WORKFLOWS:
                doc.assomaps += association_maps(catalogue, module, split=split)
            doc.keyword_rows = matched_keywords(catalogue, KEYWORD_WORKFLOWS, doc.keyword_findings)
        except (ImportError, FileNotFoundError) as e:
            print(f"association maps and matched keywords skipped: {e}")
        for amap in doc.assomaps:
            (args.document.parent / f"assomap_{amap.mode}.tex").write_text(env.get_template('assomap.tex').render(map=amap))
        (args.document.parent / 'dpr.tex').write_text(env.get_template('dpr.tex').render(doc=doc))
        for name, recipe in catalogue.recipes.items():
            if recipe._steps:
                (args.document.parent / f"flowchart_{name}.tex").write_text(
                    env.get_template('flowchart.tex').render(chart=catalogue.flowchart(name)))
        if doc.keyword_rows:
            (args.document.parent / 'matched_keywords.tex').write_text(env.get_template('matched_keywords.tex').render(doc=doc))
        args.document.write_text(env.get_template('document.tex').render(doc=doc))
        print(f"generated DRLD chapters written to {args.document}"
              + (" (standalone)" if args.standalone else " (fragment)"))
        if args.pdf:
            if not args.standalone:
                parser.error("--pdf needs --standalone: a fragment does not compile on its own")
            if not (args.drld / 'styles_data.tex').exists():
                parser.error(f"DRLD sources not found in {args.drld}; give --drld DIR or set METIS_DRLD")
            build_dir = args.document.parent
            result = subprocess.run(
                ['latexmk', '-pdf', '-interaction=nonstopmode', f'-output-directory={build_dir}', args.document.name],
                cwd=build_dir, env=os.environ | {'TEXINPUTS': f'{args.drld}//:'},
                capture_output=True, text=True)
            if result.returncode != 0:
                errors = [line for line in result.stdout.splitlines() if line.startswith('!')]
                raise SystemExit("latexmk failed" + (":\n  " + "\n  ".join(errors[:10]) if errors else f"; see {build_dir}"))
            print(f"compiled to {args.document.with_suffix('.pdf')}")
        return

    if not args.all and not args.names:
        parser.error("give data item tags or recipe names, or --all, --document, --assomap, or --list")

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

    if args.output is None and args.all:
        args.output = BUILD / 'cards'
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
