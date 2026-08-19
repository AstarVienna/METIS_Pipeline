# pymetis — developer guide

`pymetis` is the Python implementation of the METIS data-reduction pipeline,
built on [pycpl] and [pyesorex]. This guide explains the package layout, the
one non-obvious mechanism you must understand before reading anything else
(the *tag* system), and the recipes for the two most common tasks: adding a
recipe and adding a data item.

## Package layout

```
src/pymetis/
  engine/        Instrument-agnostic framework: Recipe, RecipeImpl, DataItem,
                 PipelineInput(Set), QcParameter(Set), the tag machinery.
  drl/           Science algorithms as plain functions over numpy arrays.
                 No framework types, no CPL state; individually testable.
                 New algorithmic code belongs here, not in recipe modules.
  instruments/metis/
    mixins/      The five METIS tag axes (band, detector, target, source, cgrph).
    dataitems/   ~300 declarative DataItem classes mirroring the DRLD.
    inputs/      Named PipelineInput aliases shared by recipes.
    qc/          QC parameter declarations, grouped by recipe family.
    recipes/     One module per pyesorex recipe + prefab/ shared bases.
    tests/       Declarative per-recipe tests (see Testing below).
src/tests/       Framework, DRL and data-item tests.
```

## The tag system (read this first)

Almost every class in the pipeline is *parametrizable*
(`engine/core/parametrizable.py`): it carries a dict of **tag parameters**
(e.g. `{'band': 'LM', 'detector': '2RG'}`) and a **name template** — a DRLD
tag with placeholders, e.g. `MASTER_IMG_FLAT_{source}_{band}`. Filling all
placeholders yields the final tag (`MASTER_IMG_FLAT_LAMP_LM`), which is what
appears in `ESO PRO CATG` and in SOF files.

Tags are supplied in two ways, with project-specific vocabulary:

- **Specialization** (static, import time). A class states a tag in its
  definition, almost always by inheriting a mixin:
  `class MetisLmImgFlatImpl(BandLmMixin, ...)` sets `band='LM'`.
  When the recipe class is created, `RecipeImpl.specialize()` gives the Impl
  a private copy of its `ProductSet`/`Qc` containers and formats every
  member's template with the class tags
  (`MASTER_IMG_FLAT_{source}_{band}` → `MASTER_IMG_FLAT_{source}_LM`).
- **Promotion** (dynamic, run time). Tags that depend on the data
  (e.g. `source` is `LAMP` or `TWILIGHT` depending on what was observed)
  are extracted from the matched input frames during `InputSet` validation.
  `RecipeImpl.__init__` then calls `ProductSet.promoted(**tags)` /
  `Qc.promoted(**tags)`, which resolve the remaining placeholders and return
  fully concrete containers. These are assigned to the recipe **instance**
  only — class-level state is never mutated by a run.

The lookup from a resolved tag string to the class that owns it goes through
a **registry** (one per root: `DataItem._registry`, `QcParameter._registry`).
Every non-abstract class registers itself at import time under its (possibly
still partial) template. Two rules are enforced at import time and will fail
your branch fast:

- A tag may be claimed by only one class (subclass refinements of the owner
  are allowed). Two unrelated classes claiming one tag raise `TypeError` —
  this catches copy-pasted mixin lists.
- Tag keywords are validated against the axes declared in
  `instruments/metis/mixins/__init__.py`; a typo like `bnad='LM'` raises.

Worked example — `pyesorex metis_lm_img_flat` from import to product file:

1. `recipes/__init__.py` imports the recipe module; creating
   `MetisLmImgFlat(Recipe)` triggers `Recipe.__init_subclass__`, which builds
   the man page and calls `MetisLmImgFlatImpl.specialize()`.
   The Impl's tags are `{'band': 'LM', 'detector': '2RG'}` (from its mixins);
   its `ProductSet.MasterFlat` template becomes `MASTER_IMG_FLAT_{source}_LM`.
2. At run time the `InputSet` matches the raw frames, whose data item is
   tagged `source='LAMP'`; validation records `tag_matches={'source': 'LAMP'}`.
3. `promoted()` fills the remaining placeholder, looks up
   `MASTER_IMG_FLAT_LAMP_LM` in `DataItem._registry`, and binds the concrete
   class `MasterImgFlatLampLm` as `self.ProductSet.MasterFlat`.
4. `process()` builds and returns the products; `run()` saves them
   (`DataItem.save`) and returns the product frameset to pyesorex.

## How to add a recipe

1. Create `instruments/metis/recipes/<family>/metis_<name>.py`. Start from a
   sibling (e.g. `metis_det_dark.py`, which is commented as a walkthrough).
2. Define the Impl: subclass the relevant `prefab/` base (or
   `MetisRecipeImpl`), add the band/detector/target mixins, and declare:
   - `class InputSet` — one inner class or alias per input
     (see `instruments/metis/inputs/common.py` for shared ones; add
     `OptionalInputMixin` first in the bases for optional inputs);
   - `class ProductSet` — one member per product data item;
   - `class Qc` — one member per QC parameter (shared ones live in
     `instruments/metis/qc/`);
   - `def process(self)` — the algorithm. All pixel manipulation happens
     here; delegate real math to functions in `drl/`. Return the set of
     built products.
3. Define the `Recipe` subclass with the seven pyesorex attributes
   (`_name`, `_version`, `_author`, `_email`, `_copyright`, `_synopsis`,
   `_description`) plus `_matched_keywords`, `_algorithm` and `parameters`.
   Every parameter name must be prefixed `"<recipe_name>."`.
4. Register it: add the import to `instruments/metis/recipes/__init__.py`.
   pyesorex discovers recipes only through that module.
5. Add a test module under `instruments/metis/tests/recipes/<family>/`
   mirroring a sibling, and a SOF file if applicable.
6. Check the man page: `pyesorex --man-page metis_<name>` — the Inputs,
   Outputs and QC sections are generated from your declarations.

### Input attribute names (until this is made explicit)

`InputSet` members are exposed on the instance under an automatically
derived name: the class name minus the trailing `Input`, converted to
snake_case. `MasterDarkInput` → `self.inputset.master_dark`,
`RawInput` → `self.inputset.raw`. This is the one place where a grep will
not connect definition and use — the rule lives in
`engine/inputs/inputset.py`.

## How to add a data item

1. Pick the module under `instruments/metis/dataitems/` matching its family.
2. If a template class for the item kind exists, subclass it with the right
   mixins; otherwise declare the template as `abstract=True` with:
   - `_name_template` — the DRLD tag, with `{placeholders}` for tag axes;
   - `_title_template`, `_description_template` — human-readable text;
   - `_frame_group` / `_frame_level` — CPL classification (a missing group
     causes obscure CPL errors at save time, so it is checked at
     instantiation);
   - `_oca_keywords` — the OCA keywords the item matches on;
   - `_schema` — dict of extension name → `Image` / `Table` / `None`.
3. Declare one concrete leaf per DRLD tag
   (`class MasterDarkGeo(DetectorGeoMixin, MasterDark): pass`).
   The class body stays empty unless the leaf genuinely differs.
4. Make sure the module is reachable from an import: top-level modules load
   via `dataitems/__init__.py`; subpackage modules (e.g. `hci/`) load when a
   registered recipe imports them.
5. The declaration should mirror the DRLD exactly — the intent is that DRLD
   data-item cards are regenerable from these classes.

## Testing

```sh
SOF_DIR=<test-data>/sofFiles uv run python -m pytest src -q -m "not external and not edps"
```

This is the same selection CI runs on every push/PR (`.github/workflows/ci.yaml`),
together with `ruff check src`. Markers are declared in `pyproject.toml`;
`external` needs full-size simulated data, `edps` marks the slow EDPS runs
(exercised by the nightly `run_edps.yaml`).

Per-recipe tests are declarative: subclass `BaseRecipeTest`,
`BaseInputSetTest`, `BaseProductSetTest` from `tests.classes` and point them
at your Recipe/Impl — the base classes contribute the actual test methods.
Project conventions (author format, parameter-name prefixes, mandatory
`_algorithm`/`_matched_keywords`) are enforced by these tests.

### Caveats

- At least one of the saved frames must have `cpl.ui.Frame.FrameLevel.RAW`
  - "RAW" does not mean a raw file, but an original file
- `ESO PRO CATG` must be set, otherwise you get a `DataNotFound` error

[pycpl]: https://www.eso.org/sci/software/pycpl/
[pyesorex]: https://www.eso.org/sci/software/pycpl/
