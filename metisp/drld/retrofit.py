"""
Apply the generated parts of the DRLD to a DRLD checkout: replace every card, flowchart,
association map and table that pymetis generates with an `\\input{generated/...}` where it
stood, keep all prose, flag what stays hand-written (orange), and report.

    python retrofit.py --drld ~/astar/drld-regenerated

Idempotent: a block that is already an `\\input{generated/...}` is left alone, fragments are
rewritten. It refuses to run on the approved checkout; it is meant for a copy.
"""
import argparse
import re
import sys
from collections import Counter
from dataclasses import dataclass, field
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))

from generate_drld import Catalogue, environment, write_fragments, latex  # noqa: E402

import pymetis.instruments.metis.recipes  # noqa: E402, F401  (fills the registries)
from pymetis.engine.dataitems import DataItem  # noqa: E402
from pymetis.engine.core.functions.format import partial_format  # noqa: E402

APPROVED = Path('/home/kvik/astar/drld')
ITEM_FILES = {'Detector and common calibrations': 'CalDB_data_items.tex', 'Imaging': 'IMG_data_items.tex',
              'Long-slit spectroscopy': 'LSS_data_items.tex', 'IFU': 'LMS_data_items.tex',
              'High-contrast imaging': 'ADI_data_items.tex'}
RECIPE_FILES = ['Recipes_Detector.tex', 'Recipes_Imaging_LM.tex', 'Recipes_Imaging_N.tex', 'Recipes_LSS_LM.tex',
                'Recipes_LSS_N.tex', 'Recipes_IFU_LM.tex', 'Recipes_ADI.tex', 'Recipes_Technical.tex']
QC_FILES = ['12_0-QC_parameters.tex', 'ADI_QC_items.tex']
BLOCK_END = re.compile(r'^\\(paragraph|subsubsection|subsection|section|input|clearpage)\b')
ITEM_HEADING = re.compile(r'^\\paragraph\{\\(RAW|PROD|EXTCALIB|STATCALIB|PAR)\*?\{([^}]*)\}\}')
QC_HEADING = re.compile(r'^\\subsubsection\{\{?(QC [^}]*)\}?\}')
LABEL = re.compile(r'\\label\{([^}]*)\}')
# A placeholder, however the DRLD or the code spells it: a lowercase word standing alone, `<i>`, `{band}`.
PLACEHOLDER_WORDS = {'det', 'band', 'detector', 'cgrph', 'target', 'source', 'nn', 'n', 'i', 'order'}
TOKEN = re.compile(r'<\w+>|\{\w+\}|[A-Za-z0-9]+')


def tokens(name: str) -> list[tuple[str, bool]]:
    """ (token, is_placeholder) for every word of a tag or QC name; a placeholder glued to a word ('LCOEFF{order}') is split off. """
    out = []
    for token in TOKEN.findall(name):
        # the DRLD writes placeholders lowercase (det, cgrph, nn, i) and literals uppercase (N, LM): case decides
        if token[0] in '<{' or (token.islower() and token in PLACEHOLDER_WORDS):
            out.append((token, True))
        else:
            out.append((token.lower(), False))
    return out


def plain(name: str) -> str:
    """ The literal words only, joined by '_': the normal form a pattern is matched against. """
    return '_'.join(word for word, placeholder in tokens(name) if not placeholder)


def pattern(name: str) -> re.Pattern:
    """ Placeholders become wildcards: 'QC det APP SCI FWHM nn' matches the plain form of 'QC LM APP SCI FWHM {n}'. """
    parts = []
    for word, placeholder in tokens(name):
        parts.append('[a-z0-9]*' if placeholder else re.escape(word))
    return re.compile('_?'.join(parts).replace('_?[a-z0-9]*_?', '_?[a-z0-9]*_?') + '$')


def wildcards(name: str) -> int:
    return sum(1 for _, placeholder in tokens(name) if placeholder)


def consistent(drld_name: str, code_name: str, known: dict[str, str]) -> bool:
    """
    Token by token: a DRLD literal standing where the code has a placeholder is fine only if the code
    knows that specialisation ('QC det APP ...' for 'QC LM APP ...' is; 'QC IFU cgrph ...' for
    'QC {band} {cgrph} ...' is not, there is no IFU coronagraph QC). Different token counts: trust the patterns.
    """
    d, c = tokens(drld_name), tokens(code_name)
    if len(d) != len(c):
        return True
    specialised = code_name
    for (dw, dp), (cw, cp) in zip(d, c):
        if cp and not dp:
            specialised = specialised.replace(cw, dw.upper(), 1)
    if specialised == code_name:
        return True
    want = plain(specialised)
    return any(plain(k) == want and wildcards(k) == wildcards(specialised) for k in known)


def best_match(drld_name: str, candidates: dict[str, str]) -> str | None:
    """
    The code name a DRLD name stands for: an exact plain match first, else the most specific
    candidate whose pattern matches the DRLD name's plain form or whose plain form matches the
    DRLD name's pattern. `candidates` maps code names (with their placeholders) to a value.
    """
    target = plain(drld_name)
    exact = [c for c in candidates if plain(c) == target and wildcards(c) == wildcards(drld_name)]
    if exact:
        return exact[0]
    drld_pattern = pattern(drld_name)
    hits = [c for c in candidates if (pattern(c).fullmatch(target) or drld_pattern.fullmatch(plain(c)))
            and consistent(drld_name, c, candidates)]
    if not hits:
        return None
    return min(hits, key=lambda c: (abs(wildcards(c) - wildcards(drld_name)), wildcards(c), len(c)))


@dataclass
class Report:
    replaced: Counter = field(default_factory=Counter)
    kept: dict[str, list[str]] = field(default_factory=dict)
    added: dict[str, list[str]] = field(default_factory=dict)
    notes: list[str] = field(default_factory=list)

    def keep(self, kind: str, what: str) -> None:
        self.kept.setdefault(kind, []).append(what)

    def add(self, kind: str, what: str) -> None:
        self.added.setdefault(kind, []).append(what)


class Retrofit:
    def __init__(self, drld: Path):
        self.drld = drld
        self.catalogue = Catalogue()
        self.env = environment()
        self.report = Report()
        self.templates_used: list[str] = []
        self.covered_tags: set[str] = set()                 # leaves a template card stands for
        self.matched_tags: set[str] = set()
        self.qc_matched: set[str] = set()
        self.qc_cards = self.catalogue.qc_cards()
        # code QC names with their placeholders in braces -> the generated card's label
        self.qc_labels: dict[str, str] = {card.raw: card.label for card in self.qc_cards.values()}
        self.templates = {t: t for t in DataItem._templates}
        self.labels_before = self.all_labels()

    # --- helpers ---

    def all_labels(self) -> set[str]:
        out = set()
        for path in list(self.drld.glob('*.tex')) + list(self.drld.glob('generated/**/*.tex')):
            out |= set(LABEL.findall(self.strip_comments(path.read_text())))
        return out

    @staticmethod
    def strip_comments(text: str) -> str:
        return '\n'.join(re.sub(r'(?<!\\)%.*', '', line) for line in text.splitlines())

    @staticmethod
    def extra_labels(block: list[str], emitted: set[str]) -> list[str]:
        labels = []
        for line in block:
            if line.lstrip().startswith('%'):
                continue
            for label in LABEL.findall(line):
                if label not in emitted and label not in labels:
                    labels.append(label)
        return labels

    @staticmethod
    def flag_block(block: list[str], heading_lines: int, reason: str, comment: str) -> list[str]:
        """ A kept hand-written block: orange environments, a flag under the heading, a comment above it. """
        body = [re.sub(r'\\(begin|end)\{(recipedef|datastructdef)\}', r'\\\1{hand\2}', line) for line in block]
        return [f"% HANDWRITTEN: {comment}"] + body[:heading_lines] + [f"\\notgenerated{{{latex(reason)}}}"] + body[heading_lines:]

    @staticmethod
    def block_end(lines: list[str], start: int) -> int:
        """ The index after the block starting at `start`: before the next structural command at column 0, trailing comments excluded. """
        end = start + 1
        while end < len(lines) and not BLOCK_END.match(lines[end]):
            end += 1
        while end > start + 1 and (not lines[end - 1].strip() or lines[end - 1].lstrip().startswith('%')):
            end -= 1
        return end

    @staticmethod
    def heading_lines(block: list[str]) -> int:
        n = 1
        while n < len(block) and block[n].strip().startswith('\\label{'):
            n += 1
        return n

    # --- items ---

    def item_target(self, tag: str) -> tuple | None:
        """
        ('leaf', TAG) or ('template', template, partial) for a DRLD card tag, None when pymetis has
        no such item. A DRLD card with a band fixed and the coronagraph open (`LM_cgrph_SCI_THROUGHPUT`)
        is the template `{band}_{cgrph}_SCI_THROUGHPUT` partially specialised with band='LM'.
        """
        if tag in self.catalogue.items:
            return 'leaf', tag
        if not any(placeholder for _, placeholder in tokens(tag)):
            return None
        for template in DataItem._templates:
            names = re.findall(r'\{(\w+)\}', template)
            regex = re.escape(template)
            for name in names:
                regex = regex.replace(re.escape('{' + name + '}'), rf'(?P<{name}>[A-Za-z0-9]+)', 1)
            m = re.fullmatch(regex, tag)
            if not m:
                continue
            partial = {}
            ok = True
            for name, value in m.groupdict().items():
                if value.lower() in PLACEHOLDER_WORDS:
                    continue                                 # the DRLD leaves it open too
                if value in self.catalogue.tag_values.get(name, set()):
                    partial[name] = value
                else:
                    ok = False
            if ok and self.catalogue.expand(partial_format(template, **partial)):
                return 'template', template, partial
        return None

    def retrofit_items(self, path: Path) -> None:
        lines = path.read_text().split('\n')
        out: list[str] = []
        i = 0
        while i < len(lines):
            m = ITEM_HEADING.match(lines[i])
            if not m:
                out.append(lines[i]); i += 1
                continue
            end = self.block_end(lines, i)
            block = lines[i:end]
            tag = m.group(2)
            target = self.item_target(tag)
            if target is None:
                out += self.flag_block(block, self.heading_lines(block), 'no such data item in pymetis', f"{tag}: no such data item in pymetis")
                self.report.keep('items', f"{path.name}: {tag}")
            else:
                kind, name = target[0], target[1]
                if kind == 'leaf':
                    fragment = name
                    self.matched_tags.add(name)
                else:
                    partial = target[2]
                    resolved = partial_format(name, **partial)
                    fragment = self.catalogue.drld_name(resolved)
                    if (name, partial) not in self.templates_used:
                        self.templates_used.append((name, partial))
                    self.covered_tags |= set(self.catalogue.expand(resolved))
                out.append(f"\\input{{generated/items/{fragment}}}")
                out += [f"\\label{{{label}}}" for label in self.extra_labels(block, {f"dataitem:{fragment.lower()}"})]
                out += [line for line in block if line.lstrip().startswith('%') and 'drsstructure' not in line
                        and 'generated by pymetis' not in line]
                self.report.replaced[f"items ({kind} cards)"] += 1
            i = end
        path.write_text('\n'.join(out))

    def append_code_only_items(self) -> None:
        remaining = [t for t in self.catalogue.items if t not in self.matched_tags and t not in self.covered_tags]
        by_file: dict[str, list[str]] = {}
        for tag in remaining:
            by_file.setdefault(ITEM_FILES[self.catalogue.item_family(self.catalogue.items[tag])], []).append(tag)
        marker = '\\subsubsection{Data items defined by the pipeline}'
        for name, tags in by_file.items():
            path = self.drld / name
            text = path.read_text().rstrip('\n')
            if marker in text:
                continue
            text += f"\n\n{marker}\nData items pymetis defines that the DRLD has no card for.\n"
            text += '\n'.join(f"\\input{{generated/items/{tag}}}" for tag in tags) + '\n'
            path.write_text(text)
            for tag in tags:
                self.report.add('items', f"{name}: {tag}")

    # --- recipes ---

    def retrofit_recipes(self, path: Path) -> None:
        lines = path.read_text().split('\n')
        out: list[str] = []
        i = 0
        while i < len(lines):
            line = lines[i]
            if line.strip().startswith('\\begin{recipedef}'):
                end = i + 1
                while end < len(lines) and lines[end].strip() != '\\end{recipedef}':
                    end += 1
                block = lines[i:end + 1]
                m = re.search(r'Name:?\s*&\s*\\REC\*?\{([^}]*)\}', '\n'.join(block))
                name = m.group(1) if m else None
                if name in self.catalogue.recipes:
                    out.append(f"\\input{{generated/recipes/{name}}}")
                    out += [f"\\label{{{label}}}" for label in self.extra_labels(block, set())]
                    self.report.replaced['recipe cards'] += 1
                else:
                    out += self.flag_block(block, 0, f"no such recipe in pymetis ({name or 'unnamed'})",
                                           f"recipe card {name}: no such recipe in pymetis")
                    self.report.keep('recipe cards', f"{path.name}: {name}")
                i = end + 1
                continue
            fm = re.search(r'\\input\{(?:\./)?tikz/(metis_\w+)\}', line)
            if fm and not line.lstrip().startswith('%'):
                name = fm.group(1)
                recipe = self.catalogue.recipes.get(name)
                if recipe is not None and recipe._steps:
                    out.append(line.replace(fm.group(0), f"\\input{{generated/flowcharts/{name}}}"))
                    self.report.replaced['flowcharts'] += 1
                else:
                    out.append(line)
                    out.append("    \\notgenerated{flowchart: " + ("the recipe declares no steps" if recipe else "no such recipe in pymetis") + "}")
                    self.report.keep('flowcharts', f"{path.name}: {name}")
                i += 1
                continue
            gm = re.search(r'\\includegraphics(\[[^\]]*\])?\{(?:figures/)?(metis_\w+)[^}]*\}', line)
            if gm and not line.lstrip().startswith('%'):
                out.append(line)
                out.append("    \\notgenerated{flowchart: hand-drawn; the recipe declares no steps}")
                self.report.keep('flowcharts', f"{path.name}: {gm.group(2)} (figure)")
                i += 1
                continue
            out.append(line); i += 1
        path.write_text('\n'.join(out))

    # --- QC ---

    def retrofit_qc(self, path: Path) -> None:
        lines = path.read_text().split('\n')
        out: list[str] = []
        i = 0
        while i < len(lines):
            m = QC_HEADING.match(lines[i])
            if not m:
                out.append(lines[i]); i += 1
                continue
            end = i + 1
            while end < len(lines) and lines[end].strip() != '\\end{recipedef}' and not QC_HEADING.match(lines[end]) \
                    and not lines[end].startswith(('\\subsection', '\\section', '\\input')):
                end += 1
            if end < len(lines) and lines[end].strip() == '\\end{recipedef}':
                end += 1
            block = lines[i:end]
            heading_lines = self.heading_lines(block)
            raw = best_match(m.group(1), self.qc_labels)
            generated = self.qc_labels[raw] if raw else None
            if generated is None:
                out += self.flag_block(block, heading_lines, 'no such QC parameter in pymetis', f"{m.group(1)}: no such QC parameter in pymetis")
                self.report.keep('qc cards', f"{path.name}: {m.group(1)}")
            else:
                out.append(f"\\input{{generated/qc/{generated}}}")
                out += [f"\\label{{{label}}}" for label in self.extra_labels(block, {f"qc:{generated}"})]
                self.report.replaced['qc cards'] += 1
                self.qc_matched.add(generated)
            i = end
        path.write_text('\n'.join(out))

    def append_code_only_qc(self) -> None:
        path = self.drld / '12_0-QC_parameters.tex'
        text = path.read_text()
        remaining = sorted(card.label for card in self.qc_cards.values() if card.label not in self.qc_matched)
        marker = '\\subsubsection{QC parameters defined by the pipeline}'
        if not remaining or marker in text:
            return
        block = f"{marker}\nQC parameters pymetis defines that the DRLD has no card for.\n" + \
                '\n'.join(f"\\input{{generated/qc/{label}}}" for label in remaining) + '\n\n'
        anchor = '\\input{ADI_QC_items}'
        text = text.replace(anchor, block + anchor, 1) if anchor in text else text + block
        path.write_text(text)
        for label in remaining:
            self.report.add('qc cards', label)

    # --- maps, tables, preamble, build ---

    def retrofit_maps(self) -> None:
        swaps = {
            'Overview_IMG_LM_N.tex': [(r'\\input\{tikz/IMG_LM_assomap_tikz(?:\.tex)?\}', r'\\input{generated/assomap_lm_img}'),
                                      (r'\\input\{tikz/IMG_N_assomap_tikz(?:\.tex)?\}', r'\\input{generated/assomap_n_img}')],
            'Overview_IFU.tex': [(r'\\input\{tikz/IFU_assomap_tikz(?:\.tex)?\}', r'\\input{generated/assomap_ifu}')],
            'Overview_LSS.tex': [(r'\\includegraphics(?:\[[^\]]*\])?\{figures/LM_LSS[^}]*part_1[^}]*\}', r'\\resizebox{\\linewidth}{!}{\\input{generated/assomap_lm_lss_calib}}'),
                                 (r'\\includegraphics(?:\[[^\]]*\])?\{figures/LM_LSS[^}]*part_2[^}]*\}', r'\\resizebox{\\linewidth}{!}{\\input{generated/assomap_lm_lss_science}}'),
                                 (r'\\includegraphics(?:\[[^\]]*\])?\{figures/N_LSS[^}]*part_1[^}]*\}', r'\\resizebox{\\linewidth}{!}{\\input{generated/assomap_n_lss_calib}}'),
                                 (r'\\includegraphics(?:\[[^\]]*\])?\{figures/N_LSS[^}]*part_2[^}]*\}', r'\\resizebox{\\linewidth}{!}{\\input{generated/assomap_n_lss_science}}')],
        }
        for name, pairs in swaps.items():
            path = self.drld / name
            text = path.read_text()
            for pattern, replacement in pairs:
                text, n = re.subn(pattern, replacement, text)
                if n:
                    self.report.replaced['association maps'] += n
                elif replacement.split('/')[-1].rstrip('}') not in text:
                    self.report.notes.append(f"{name}: nothing matched {pattern}")
            path.write_text(text)

    def retrofit_tables(self) -> None:
        path = self.drld / '05_0-Data_Processing_Overview.tex'
        lines = path.read_text().split('\n')
        if not any('generated/matched_keywords' in line for line in lines):
            label = next(i for i, line in enumerate(lines) if 'label{tab:fitsmatchedkeywordssummary}' in line)
            start = max(i for i in range(label) if lines[i].startswith('\\newgeometry'))
            end = next(i for i in range(label, len(lines)) if lines[i].startswith('\\restoregeometry'))
            lines[start:end + 1] = ['\\input{generated/matched_keywords}']
            path.write_text('\n'.join(lines))
            self.report.replaced['tables'] += 1
        app = self.drld / 'App02_FITS_keywords.tex'
        text = app.read_text()
        if '\\input{APP_dpr_keywords}' in text:
            app.write_text(text.replace('\\input{APP_dpr_keywords}', '\\input{generated/dpr}'))
            (self.drld / 'APP_dpr_keywords.tex').unlink(missing_ok=True)
            self.report.replaced['tables'] += 1

    def retrofit_preamble_and_build(self) -> None:
        main = self.drld / 'METIS_DRLD.tex'
        text = main.read_text()
        if 'generated/preamble' not in text:
            anchor = '\\newenvironment{datastructdef}'
            i = text.index(anchor)
            j = text.index('\\end{tcolorbox}}', i) + len('\\end{tcolorbox}}')
            text = text[:j] + '\n\n%% The hand-written-card environments and the flag of the retrofitted document\n\\input{generated/preamble}' + text[j:]
            main.write_text(text)
        makefile = self.drld / 'Makefile'
        text = makefile.read_text()
        if 'generated/' not in text:
            text = text.replace("\t$$(wildcard figures/*.*)\n", "\t$$(wildcard figures/*.*) \\\n\t$$(wildcard generated/*.tex) \\\n\t$$(wildcard generated/*/*.tex)\n", 1)
            text += ('\n# Regenerate every part that comes from the pipeline (a pymetis checkout with its venv next to this one).\n'
                     'PYMETIS ?= ../pipeline/metisp/pymetis/.venv/bin/python\n'
                     'RETROFIT ?= ../pipeline/metisp/drld/retrofit.py\n'
                     'regenerate:\n\t$(PYMETIS) $(RETROFIT) --drld .\n')
            makefile.write_text(text)

    # --- the run ---

    def run(self) -> None:
        already = [name for name in list(ITEM_FILES.values()) + RECIPE_FILES + QC_FILES
                   if 'generated/' in (self.drld / name).read_text()]
        if already:
            raise SystemExit(f"{self.drld} is already retrofitted ({already[0]} inputs generated fragments); "
                             f"reset it to the baseline commit first, the surgery is not repeatable in place")
        for name in ITEM_FILES.values():
            self.retrofit_items(self.drld / name)
        self.append_code_only_items()
        for name in RECIPE_FILES:
            self.retrofit_recipes(self.drld / name)
        for name in QC_FILES:
            self.retrofit_qc(self.drld / name)
        self.append_code_only_qc()
        self.retrofit_maps()
        self.retrofit_tables()
        self.retrofit_preamble_and_build()
        for template, partial in self.templates_used:          # references to a covered leaf point at its template card
            resolved = partial_format(template, **partial)
            for leaf in self.catalogue.expand(resolved):
                if leaf not in self.matched_tags:
                    self.catalogue.alias_targets[leaf] = self.catalogue.drld_name(resolved).lower()
        counts = write_fragments(self.catalogue, self.env, self.drld, templates=self.templates_used)
        self.report.notes.append('fragments: ' + ', '.join(f"{n} {k}" for k, n in counts.items()))
        self.dedupe_labels()
        missing = self.labels_before - self.all_labels()
        if missing:
            self.report.notes.append(f"LABELS LOST ({len(missing)}): {sorted(missing)}")
        self.write_report()

    def dedupe_labels(self) -> None:
        """
        A label re-emitted after an \\input that another fragment or block also defines (the DRLD gave two
        cards the same alias label) would be multiply defined: drop the later standalone copies.
        """
        defined: set[str] = set()
        for path in sorted(self.drld.glob('generated/**/*.tex')):
            defined |= set(LABEL.findall(self.strip_comments(path.read_text())))
        files = list(ITEM_FILES.values()) + RECIPE_FILES + QC_FILES
        for name in files:
            path = self.drld / name
            out = []
            for line in path.read_text().split('\n'):
                m = re.fullmatch(r'\\label\{([^}]*)\}', line.strip())
                if m:
                    if m.group(1) in defined:
                        self.report.notes.append(f"{name}: duplicate label {m.group(1)} dropped")
                        continue
                    defined.add(m.group(1))
                else:
                    defined |= set(LABEL.findall(re.sub(r'(?<!\\)%.*', '', line)))
                out.append(line)
            path.write_text('\n'.join(out))

    def write_report(self) -> None:
        r = self.report
        lines = ['# Retrofit report', '', 'What the pipeline generates replaced in this copy, what stays hand-written, what was added.', '']
        lines += ['## Replaced', ''] + [f"- {k}: {n}" for k, n in sorted(r.replaced.items())] + ['']
        for title, data in (('Kept hand-written (flagged orange in the document)', r.kept), ('Added from the pipeline', r.added)):
            lines += [f"## {title}", '']
            for kind, entries in sorted(data.items()):
                lines += [f"### {kind} ({len(entries)})", ''] + [f"- {e}" for e in entries] + ['']
        lines += ['## Notes', ''] + [f"- {n}" for n in r.notes] + ['']
        (self.drld / 'generated' / 'RETROFIT.md').write_text('\n'.join(lines))
        print('\n'.join(lines[4:4 + len(r.replaced) + 2]))
        print(f"kept: { {k: len(v) for k, v in r.kept.items()} }, added: { {k: len(v) for k, v in r.added.items()} }")
        for note in r.notes:
            print(note[:400])


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('--drld', type=Path, required=True, help='the DRLD copy to retrofit (never the approved checkout)')
    args = parser.parse_args()
    if not (args.drld / 'METIS_DRLD.tex').exists():
        parser.error(f"{args.drld} is no DRLD checkout")
    if args.drld.resolve() == APPROVED.resolve():
        parser.error("refusing to retrofit the approved checkout; copy it first")
    Retrofit(args.drld).run()


if __name__ == '__main__':
    main()
