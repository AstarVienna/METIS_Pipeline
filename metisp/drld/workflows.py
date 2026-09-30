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
from pymetis.engine.keywords import Alias


@dataclass
class Column:
    key: str
    tasks: list[str]                # the workflow tasks running this recipe; drawn as one column, as the DRLD does
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
    head: bool = True               # arrows: an arrowhead at `dst` (a product); False for the plain continuation to a lower dot


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
        return self.is_raw_tag(tag)

    @staticmethod
    def is_raw_tag(tag: str) -> bool:
        item = DataItem.find(tag)
        return item is not None and item.frame_group() == cpl.ui.Frame.FrameGroup.RAW

    # --- the map ---

    def build(self, split: bool = False) -> list[AssoMap]:
        infrequent = [t for t in self.tasks if wf.is_infrequent(t)]
        daily = [t for t in self.tasks if not wf.is_infrequent(t)]
        ordered = infrequent + daily
        for task in ordered:
            binding = wf.bind(task, self.tasks, self.module)
            self.findings += [f"{task.name} ({task.command}) {p}" for p in binding.problems]
            self.findings += [f"{task.name} ({task.command}) not fed (optional): {m}" for m in binding.missing_optional]
            self.findings += [f"{task.name} ({task.command}) recipe also accepts {tag}; no task of this workflow feeds it"
                              for tag in self.unfed_alternatives(task)]

        # One column per recipe: the tasks running the same recipe (basic_reduce on SCI, STD and
        # SKY frames) merge, the header listing every raw they take and the products the union.
        columns: dict[str, Column] = {}
        col_of: dict[str, str] = {}
        for task in ordered:
            key = key_of(task.command)
            if key in columns:
                self.merge(columns[key], task)
            else:
                columns[key] = self.column(task)
            col_of[task.name] = key
        by_name = {t.name: t for t in ordered}
        for col in columns.values():
            products: list[str] = []
            for name in col.tasks:
                task = by_name[name]
                products += [t for t in (wf.requested_products(task, self.tasks) or self.fallback_products(task))
                             if t not in products]
            declared = self.declared_order(by_name[col.tasks[0]])
            col.products = sorted(products, key=lambda t: declared.index(t) if t in declared else len(declared))
            for tag in col.products:
                if tag not in self.catalogue.items and self.item(tag) is None:
                    self.findings.append(f"{col.recipe}: product {tag} is not a catalogue item")

        rows = self.rows(ordered, columns, col_of)
        separator = None
        if infrequent and daily and col_of[infrequent[-1].name] != col_of[daily[0].name]:
            separator = Separator(left=col_of[infrequent[-1].name], right=col_of[daily[0].name])

        if not split:
            return [self.assemble(mode_of(self.module), list(columns.values()), rows, separator)]
        # Two figures, as the LSS overview draws them: the calibration cascade, and the science chain
        # (tasks with the SCIENCE target and everything fed from them).
        science_keys = {col_of[t.name] for t in ordered if self.is_science(t)}
        calib = [c for c in columns.values() if c.key not in science_keys]
        science = [c for c in columns.values() if c.key in science_keys]
        sep_calib = separator if separator and any(c.key == separator.right for c in calib) else None
        return [self.assemble(f"{mode_of(self.module)}_calib", calib, rows, sep_calib),
                self.assemble(f"{mode_of(self.module)}_science", science, rows, None)]

    def is_science(self, task) -> bool:
        if 'science' in task.meta_targets:
            return True
        return (not wf.is_data_source(task.main_input)) and self.is_science(task.main_input)

    def column(self, task) -> Column:
        main = wf.main_tags(task)
        fed_by_task = not wf.is_data_source(task.main_input)
        return Column(key=key_of(task.command), tasks=[task.name], recipe=task.command, main_inputs=self.header_raws(task),
                      fed_by_task=fed_by_task, main_row=key_of(main[0]) if fed_by_task and main else None,
                      infrequent=wf.is_infrequent(task))

    def merge(self, col: Column, task) -> None:
        """ A further task running the column's recipe: its raws join the header, its cadence the column's. """
        col.tasks.append(task.name)
        col.main_inputs += [t for t in self.header_raws(task) if t not in col.main_inputs]
        col.fed_by_task = col.fed_by_task and not wf.is_data_source(task.main_input)
        col.infrequent = col.infrequent or wf.is_infrequent(task)

    def header_raws(self, task) -> list[str]:
        """ The raw tags in a task's header box: its main input when that is a data source, plus associated raw data sources. """
        raws = [] if not wf.is_data_source(task.main_input) else list(wf.main_tags(task))
        for assoc in task.flatten_associated_inputs():
            if wf.is_data_source(assoc.input_task):
                raws += [t for t in wf.assoc_tags(assoc) if self.is_raw(t) and t not in raws]
        return raws

    def unfed_alternatives(self, task) -> list[str]:
        """ Catalogue tags the recipe's RAW-role inputs accept (LM_FLAT_TWILIGHT_RAW next to LM_FLAT_LAMP_RAW) that no task feeds. """
        from pymetis.engine.recipes import Recipe
        recipe = Recipe._registry.get(task.command)
        if recipe is None:
            return []
        fed = set()
        for t in self.tasks:
            fed |= set(wf.main_tags(t))
            for assoc in t.flatten_associated_inputs():
                fed |= set(wf.assoc_tags(assoc))
        out = []
        for _, inp in recipe.Impl.InputSet.list_input_classes():
            if inp._group != cpl.ui.Frame.FrameGroup.RAW:
                continue
            for tag in self.catalogue.expand(inp.Item.name()):
                if tag not in fed and tag not in out:
                    out.append(tag)
        return out

    def declared_order(self, task) -> list[str]:
        """ The recipe's products in declaration order, expanded to catalogue tags (LINEARITY before GAIN_MAP as the ProductSet lists them). """
        from pymetis.engine.recipes import Recipe
        recipe = Recipe._registry.get(task.command)
        if recipe is None:
            return []
        out: list[str] = []
        for _, product in recipe._list_products():
            out += [t for t in self.catalogue.expand(product.name()) if t not in out]
        return out

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

    def rows(self, ordered, columns: dict[str, Column], col_of: dict[str, str]) -> list[Row]:
        """
        Rows grouped by the column that produces them, the groups placed where they are first
        needed: a group sits above the first column consuming any of its rows, as the DRLD
        draws it (the distortion table next to the calibration recipe that reads it, not next
        to the recipe that made it). External files are single-row groups placed the same way,
        after the products a column consumes at the same point. Within a group the recipe's
        product order holds.
        """
        specs: list[tuple[str, str | None]] = []            # (tag, producer key or None)
        consumers: dict[str, dict[str, bool]] = {}
        for task in ordered:
            col = col_of[task.name]
            for assoc in task.flatten_associated_inputs():
                optional = wf.is_optional(assoc)
                for tag in wf.assoc_tags(assoc):
                    if wf.is_data_source(assoc.input_task) and self.is_raw(tag):
                        continue                              # shown in the header box
                    if wf.is_data_source(assoc.input_task) and (tag, None) not in specs:
                        specs.append((tag, None))
                    consumers.setdefault(tag, {})[col] = optional
            if not wf.is_data_source(task.main_input):
                for tag in wf.main_tags(task):
                    consumers.setdefault(tag, {})[col] = False
        for col in columns.values():
            for tag in col.products:
                existing = next((i for i, s in enumerate(specs) if s[0] == tag), None)
                if existing is None:
                    specs.append((tag, col.key))
                elif specs[existing][1] is None:
                    # a data source duplicates a product of this workflow: the product wins the row
                    specs[existing] = (tag, col.key)
                    self.findings.append(f"{col.recipe}: product {tag} is also offered as an external data source")
                else:
                    self.findings.append(f"{col.recipe}: product {tag} is also a product of {specs[existing][1]}")

        col_index = {key: i for i, key in enumerate(['ext'] + list(columns))}

        def first_consumer(tag: str) -> int:
            return min((col_index[c] for c in consumers.get(tag, {})), default=len(col_index))

        # A group is placed at the later of its producer's column and the first column consuming
        # any of its rows (a product fed backwards, the master flat into the distortion task, stays
        # with its producer); at the same place products come before external files, producers in
        # column order, and the more widely used external file first.
        group_first: dict[str, int] = {}
        for tag, producer in specs:
            key = producer if producer is not None else f"ext:{tag}"
            group_first[key] = min(group_first.get(key, len(col_index)), first_consumer(tag))

        def place(spec: tuple[str, str | None]) -> tuple:
            tag, producer = spec
            if producer is None:
                return group_first[f"ext:{tag}"], 1, -len(consumers.get(tag, {}))
            return max(group_first[producer], col_index[producer]), 0, col_index[producer]

        specs.sort(key=place)

        rows = []
        for tag, producer in specs:
            style = self.style(tag)
            cells = []
            for col in ['ext'] + list(columns):
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
            consumed_here = any(c.style == 'connection' for c in cells)
            produced_here = row.producer in keys
            if not (consumed_here or produced_here):
                continue
            producer = row.producer
            if producer is not None and not produced_here:
                # produced by a column of the other part: an external input to this figure
                cells = [Cell('ext', row.style, self.reference(row.tag)) if c.col == 'ext' else c for c in cells]
                producer = None
            rows.append(Row(row.key, row.tag, row.style, producer, cells, row.consumers))

        # One continuous vertical line per column, from the recipe header through every dot the column
        # consumes to its products (arrowhead on each product); a dot below the last product, or a
        # column without products, still gets the plain line down to its lowest dot.
        arrows: list[Link] = []
        for col in columns:
            marked = [r.key for r in rows if any(c.col == col.key and c.style != 'empty' for c in r.cells)]
            products = [r.key for r in rows if r.producer == col.key]
            start = f"{col.key}_raw"
            for nxt in products:
                arrows.append(Link(start, f"{col.key}_{nxt}"))
                start = f"{col.key}_{nxt}"
            if marked and marked[-1] not in products:
                arrows.append(Link(start, f"{col.key}_{marked[-1]}", head=False))

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


# --- the matched-keywords summary (DRLD `tab:fitsmatchedkeywordssummary`) -------------------



@dataclass
class KeywordRow:
    recipe: str
    tasks: list[str]                # workflow tasks running the recipe ([] for a recipe without one)
    main_inputs: list[str]          # LaTeX references
    calibrations: list[str]         # LaTeX references
    fits_keywords: list             # Keyword objects: the aliases expanded to the instrument keywords they resolve to
    aliases: list                   # Keyword objects: `Recipe._matched_keywords` as declared


def matched_keywords(catalogue, module_names, findings: list[str] | None = None) -> list[KeywordRow]:
    """
    One row per recipe, as the DRLD table: the main input(s) and the associated calibrations of
    every workflow task that runs the recipe, in the modules' task order, and the recipe's
    matched keywords. A recipe no task runs is listed from its own InputSet.
    """
    from pymetis.engine.recipes import Recipe
    findings = findings if findings is not None else []
    rows: dict[str, KeywordRow] = {}

    def ref(tag: str) -> str:
        item = DataItem.find(tag)
        return catalogue.reference(item, tag) if item is not None else rf'\EXTCALIB{{{tag}}}'

    def add(row: KeywordRow, attr: str, tags) -> None:
        for tag in tags:
            r = ref(tag)
            if r not in getattr(row, attr):
                getattr(row, attr).append(r)

    for module in module_names:
        for task in wf.tasks(wf.load_workflow(module)):
            recipe = Recipe._registry.get(task.command)
            if recipe is None:
                findings.append(f"{task.name}: recipe {task.command} is not registered")
                continue
            row = rows.setdefault(task.command, KeywordRow(
                recipe=task.command, tasks=[], main_inputs=[], calibrations=[], fits_keywords=[],
                aliases=sorted(recipe._matched_keywords)))
            if task.name not in row.tasks:
                row.tasks.append(task.name)
            add(row, 'main_inputs', wf.main_tags(task))
            for assoc in task.flatten_associated_inputs():
                tags = wf.assoc_tags(assoc)
                raws = [t for t in tags if wf.is_data_source(assoc.input_task) and MapBuilder.is_raw_tag(t)]
                add(row, 'main_inputs', raws)
                add(row, 'calibrations', [t for t in tags if t not in raws])

    for name, recipe in catalogue.recipes.items():
        if name in rows:
            continue
        findings.append(f"{name}: no workflow task runs this recipe; inputs listed from the recipe")
        row = rows[name] = KeywordRow(recipe=name, tasks=[], main_inputs=[], calibrations=[], fits_keywords=[],
                                      aliases=sorted(recipe._matched_keywords))
        for _, inp in recipe._list_inputs():
            tag = catalogue.input_tag(recipe, inp)
            attr = 'main_inputs' if inp._group == cpl.ui.Frame.FrameGroup.RAW else 'calibrations'
            r = catalogue.reference(inp.Item, tag)
            if r not in getattr(row, attr):
                getattr(row, attr).append(r)

    for row in rows.values():
        for alias in row.aliases:
            for keyword in (alias.resolves_to if isinstance(alias, Alias) else (alias,)):
                if keyword not in row.fits_keywords:
                    row.fits_keywords.append(keyword)
    return [rows[name] for name in catalogue.recipes if name in rows]


# --- the hand-drawn per-recipe flowcharts against the recipes ---------------------------------

# How the DRLD flowcharts spell a placeholder inside a tag (`LINEARITY_det`) -> the catalogue's keyword.
FLOWCHART_PLACEHOLDERS = {'det': 'detector', 'band': 'band', 'cgrph': 'cgrph', 'target': 'target', 'source': 'source'}


def lint_flowchart(catalogue, path) -> list[str]:
    """
    The `\\RAW`, `\\PROD`, `\\EXTCALIB` and `\\STATCALIB` tags a flowchart `tikz/metis_<recipe>.tex`
    draws that are neither an input nor a product of that recipe, and the ones that are no
    catalogue item at all. The macro a tag is drawn with is not checked: the flowcharts use
    `\\STATCALIB` for any calibration taken from the database, not for static calibrations
    only. The step chains themselves are prose and are not checked.
    """
    from pymetis.engine.recipes import Recipe
    name = path.stem
    recipe = Recipe._registry.get(name)
    if recipe is None:
        return [f"{path.name}: no recipe {name}"]
    expected: set[str] = set()
    for _, inp in recipe._list_inputs():
        for tag in catalogue.items:
            if issubclass(catalogue.items[tag], inp.Item):
                expected.add(tag)
    for _, product in recipe._list_products():
        expected.update(catalogue.expand(product.name()))

    out = []
    text = path.read_text()
    for macro, drawn in sorted(set(re.findall(r'\\(RAW|PROD|EXTCALIB|STATCALIB)\{([^}]*)\}', text))):
        template = re.sub(r'(?<![A-Z])([a-z]+)(?![A-Z])',
                          lambda m: '{' + FLOWCHART_PLACEHOLDERS.get(m.group(1), m.group(1)) + '}', drawn)
        tags = catalogue.expand(template)
        if not tags:
            out.append(f"{path.name}: \\{macro}{{{drawn}}} is no catalogue item")
        elif not any(t in expected for t in tags):
            out.append(f"{path.name}: \\{macro}{{{drawn}}} is neither an input nor a product of {name}")
    return out
