"""
Association maps of the DRLD from the EDPS workflows.

An association map is the DRLD's per-mode figure (`fig:IMG_LM_Assomap` and friends): a TikZ
matrix with one column per workflow task, headed by the raw data that triggers it and the
recipe it runs; one row per data item that flows between tasks; an arrow from a task down
to its products; a horizontal line from a product to a dot in the column of every task that
consumes it, dashed where EDPS treats the input as optional. The workflow decides the
structure; the catalogue (`generate_drld.Catalogue`) decides how an item is named, styled
and referenced; every disagreement between a task and its recipe (`pymetis.engine.workflows
.bind`) is written into the figure as a `% FINDING:` comment.
"""
import re
from dataclasses import dataclass, field

import cpl

from pymetis.engine import workflows as wf
from pymetis.engine.dataitems import DataItem


@dataclass
class Column:
    key: str
    task: str
    recipe: str
    main_inputs: list[str]          # raw tags in the header box (main input, plus associated raws)
    fed_by_task: bool               # main input is an upstream task: no raw in the header
    main_row: str | None            # row key of the main-input product, for the arrow into the column
    infrequent: bool
    products: list[str] = field(default_factory=list)


@dataclass
class Cell:
    col: str
    style: str                      # a node style of assomap_common.tex, 'connection' or 'empty'
    content: str = ''


@dataclass
class Row:
    key: str
    tag: str
    style: str
    producer: str | None            # column key, or None for an external / static file (column 'ext')
    cells: list[Cell]
    consumers: dict[str, bool]      # column key -> optional


@dataclass
class Link:
    src: str
    dst: str
    dashed: bool = False


@dataclass
class Separator:
    left: str                       # last infrequent column key
    right: str                      # first daily column key


@dataclass
class AssoMap:
    module: str
    mode: str                       # e.g. 'lm_img', 'lm_lss_calib'
    title: str
    columns: list[Column]
    rows: list[Row]
    arrows: list[Link]
    matches: list[Link]
    separator: Separator | None
    findings: list[str]

    @property
    def column_keys(self) -> list[str]:
        return ['ext'] + [c.key for c in self.columns]


def key_of(name: str) -> str:
    """ TikZ node name part: lowercase alphanumerics only. """
    return re.sub(r'[^a-z0-9]', '', name.lower().removeprefix('metis_'))


def mode_of(module_name: str) -> str:
    return re.sub(r'^metis_|_wkf$', '', module_name.rsplit('.', 1)[-1])


class MapBuilder:
    """ Builds the association map(s) of one workflow module. """

    def __init__(self, catalogue, module_name: str):
        self.catalogue = catalogue
        self.module = module_name
        self.workflow = wf.load_workflow(module_name)
        self.tasks = wf.tasks(self.workflow)
        self.findings: list[str] = []

    # --- styling through the catalogue ---

    def item(self, tag: str) -> type[DataItem] | None:
        return DataItem.find(tag)

    def style(self, tag: str) -> str:
        item = self.item(tag)
        if item is None:
            return 'extcalfile'
        macro = self.catalogue.macro_of(item, tag)
        if macro == 'STATCALIB':
            return 'statcalfile'
        if macro == 'EXTCALIB':
            return 'extcalfile'
        if macro == 'RAW':
            return 'rawinput'
        science = item.frame_group() == cpl.ui.Frame.FrameGroup.PRODUCT or item.tag_parameters().get('target') == 'SCI'
        return 'scienceproduct' if science else 'calibproduct'

    def reference(self, tag: str) -> str:
        item = self.item(tag)
        if item is None:
            return rf'\EXTCALIB{{{tag}}}'
        return self.catalogue.reference(item, tag)

    def is_raw(self, tag: str) -> bool:
        item = self.item(tag)
        return item is not None and item.frame_group() == cpl.ui.Frame.FrameGroup.RAW

    # --- the map ---

    def build(self, split: bool = False) -> list[AssoMap]:
        infrequent = [t for t in self.tasks if wf.is_infrequent(t)]
        daily = [t for t in self.tasks if not wf.is_infrequent(t)]
        ordered = infrequent + daily
        for task in ordered:
            binding = wf.bind(task, self.tasks, self.module)
            self.findings += [f"{task.name} ({task.command}) {p}" for p in binding.problems]

        columns = {t.name: self.column(t) for t in ordered}
        for task in ordered:
            columns[task.name].products = wf.requested_products(task, self.tasks) or self.fallback_products(task)
            for tag in columns[task.name].products:
                if tag not in self.catalogue.items and self.item(tag) is None:
                    self.findings.append(f"{task.name}: product {tag} is not a catalogue item")

        rows = self.rows(ordered, columns)
        separator = None
        if infrequent and daily:
            separator = Separator(left=columns[infrequent[-1].name].key, right=columns[daily[0].name].key)

        if not split or separator is None:
            return [self.assemble(mode_of(self.module), [columns[t.name] for t in ordered], rows, separator)]
        return [self.assemble(f"{mode_of(self.module)}_calib", [columns[t.name] for t in infrequent], rows, None),
                self.assemble(f"{mode_of(self.module)}_science", [columns[t.name] for t in daily], rows, None)]

    def column(self, task) -> Column:
        main = wf.main_tags(task)
        fed_by_task = not wf.is_data_source(task.main_input)
        raws = [] if fed_by_task else list(main)
        for assoc in task.flatten_associated_inputs():
            if wf.is_data_source(assoc.input_task):
                raws += [t for t in wf.assoc_tags(assoc) if self.is_raw(t) and t not in raws]
        return Column(key=key_of(task.name), task=task.name, recipe=task.command, main_inputs=raws,
                      fed_by_task=fed_by_task, main_row=key_of(main[0]) if fed_by_task and main else None,
                      infrequent=wf.is_infrequent(task))

    def fallback_products(self, task) -> list[str]:
        """ A terminal task: nobody asks for its products, so take the recipe's declaration, resolved with the main input's tags. """
        from pymetis.engine.recipes import Recipe
        recipe = Recipe._registry.get(task.command)
        if recipe is None:
            return []
        tags: dict[str, str] = {}
        for tag in wf.main_tags(task):
            item = self.item(tag)
            if item is not None:
                tags |= {k: v for k, v in item.tag_parameters().items() if isinstance(v, str)}
        out: list[str] = []
        for _, product in recipe._list_products():
            name = product.name()
            for key, value in tags.items():
                name = name.replace('{' + key + '}', value)
            out += [t for t in self.catalogue.expand(name) if t not in out]
        return out

    def rows(self, ordered, columns: dict) -> list[Row]:
        """ External files first (in order of first use), then every column's products in column order. """
        specs: list[tuple[str, str | None]] = []            # (tag, producer key or None)
        consumers: dict[str, dict[str, bool]] = {}
        for task in ordered:
            col = columns[task.name].key
            for assoc in task.flatten_associated_inputs():
                optional = wf.is_optional(assoc)
                for tag in wf.assoc_tags(assoc):
                    if wf.is_data_source(assoc.input_task) and self.is_raw(tag):
                        continue                              # shown in the header box
                    if wf.is_data_source(assoc.input_task) and (tag, None) not in specs:
                        specs.append((tag, None))
                    consumers.setdefault(tag, {})[col] = optional
            if columns[task.name].fed_by_task:
                for tag in wf.main_tags(task):
                    consumers.setdefault(tag, {})[col] = False
        for task in ordered:
            for tag in columns[task.name].products:
                if not any(s[0] == tag for s in specs):
                    specs.append((tag, columns[task.name].key))
                else:
                    self.findings.append(f"{task.name}: product {tag} is also an external input or another task's product")

        rows = []
        for tag, producer in specs:
            style = self.style(tag)
            cells = []
            for col in ['ext'] + [columns[t.name].key for t in ordered]:
                if (producer is None and col == 'ext') or col == producer:
                    cells.append(Cell(col, style, self.reference(tag)))
                elif col in consumers.get(tag, {}):
                    cells.append(Cell(col, 'connection'))
                else:
                    cells.append(Cell(col, 'empty'))
            rows.append(Row(key=key_of(tag), tag=tag, style=style, producer=producer, cells=cells,
                            consumers=consumers.get(tag, {})))
        return rows

    def assemble(self, mode: str, columns: list[Column], all_rows: list[Row], separator: Separator | None) -> AssoMap:
        keys = ['ext'] + [c.key for c in columns]
        rows = []
        for row in all_rows:
            cells = [c for c in row.cells if c.col in keys]
            if any(c.style != 'empty' for c in cells if c.col != 'ext') or (row.producer is None and any(c.style == 'connection' for c in cells)):
                rows.append(Row(row.key, row.tag, row.style, row.producer, cells, row.consumers))
        # drop external rows nobody in this part consumes
        rows = [r for r in rows if r.producer is not None or any(c.style == 'connection' for c in r.cells)]
        row_keys = {r.key for r in rows}

        arrows: list[Link] = []
        for col in columns:
            chain = [r.key for r in rows if r.producer == col.key]
            if not chain:
                continue
            start = f"{col.key}_{col.main_row}" if col.fed_by_task and col.main_row in row_keys else f"{col.key}_raw"
            for nxt in chain:
                arrows.append(Link(start, f"{col.key}_{nxt}"))
                start = f"{col.key}_{nxt}"

        matches: list[Link] = []
        for row in rows:
            marked = [(c.col, c.style) for c in row.cells if c.style != 'empty']
            for (a, _), (b, _) in zip(marked, marked[1:]):
                matches.append(Link(f"{a}_{row.key}", f"{b}_{row.key}", dashed=row.consumers.get(b, False)))

        return AssoMap(module=self.module, mode=mode, title=mode.replace('_', ' ').upper(),
                       columns=columns, rows=rows, arrows=arrows, matches=matches, separator=separator,
                       findings=sorted(set(self.findings)))


def association_maps(catalogue, module_name: str, split: bool = False) -> list[AssoMap]:
    return MapBuilder(catalogue, module_name).build(split=split)
