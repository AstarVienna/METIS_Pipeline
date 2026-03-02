# Architecture Report

## Pedro Beirao, Feb 2026

At the first glance here are my observations and suggestions of pymetis architecture:

## Observations
- The core runtime is built around `MetisRecipe` + `MetisRecipeImpl`, with dynamic promotion and class introspection used to assemble inputs, products, and QC metadata. See `src/pymetis/classes/recipes/recipe.py` and `src/pymetis/classes/recipes/impl.py`.
- Domain types (`DataItem` for example) are tightly coupled to CPL and pyesorex APIs. 
- Recipes are heavy consumers that depend on `classes`, `dataitems`, `qc`, `utils`, and sometimes `functions`, which indicates high coupling.

## Risks & Architectural Friction
- The `classes` and `dataitems` packages depend on each other, which can complicate refactors and tests.
- Coupling to CPL/pyesorex inside domain classes reduces testability and blocks reuse of logic outside the CPL runtime.

### Example of Mutual dependence between `classes` and `dataitems`
1. `dataitems` depend on `classes`
- `src/pymetis/dataitems/gainmap.py` imports from `classes`:
- `from pymetis.classes.dataitems import ImageDataItem`
- `from pymetis.classes.mixins import Detector2rgMixin, DetectorGeoMixin, DetectorIfuMixin, DetectorSpecificMixin`
2. `classes` depend on `dataitems`
- `src/pymetis/classes/inputs/common.py` imports from `dataitems`:
- `from pymetis.dataitems.gainmap import GainMap`
- `from pymetis.dataitems.badpixmap import BadPixMap`
- `from pymetis.dataitems.masterflat import MasterFlat`
- (and several others)

The core framework layer `classes` depends on concrete `dataitems` definitions, while `dataitems` depend back on core mixins and base classes in `classes`. This locks the two packages together and makes it harder to refactor, swap in a different backend, or data model because you can’t isolate the “core” from the “domain types”.

### Example: `DataItem` Depends Directly on CPL Types and Metadata
- `DataItem` imports and uses CPL directly:
- `import cpl`
- `from cpl.core import Msg, Image, Table, PropertyList, Property`
- It stores CPL-specific metadata like `_frame_group`, `_frame_level`, `_frame_type`.
- Its `__init__` signature expects a `cpl.core.PropertyList` and `Hdu` objects, which themselves wrap CPL objects.

This reduces testability unit tests on `DataItem` behavior cannot be performed without a CPL runtime available. Even simple tests of metadata formatting require CPL object creation (e.g., `cpl.core.PropertyList()`). Mocking CPL is possible but cumbersome because the class imports and calls CPL types directly.

## Main Recommendations

### 1. Split the Architecture Into Explicit Layers
- Create a new domain layer (e. g. `pymetis.core`) with pure-Python models and metadata (`DataItem` spec, `Input` spec, `QC` spec) that has minimal cpl.core dependency.
- Keep recipe orchestration in `pymetis.recipes` but make it consume only the core interfaces + adapters.
- Wherever possible keep CPL/pyesorex specifics in a separate layer that turns core objects into CPL frames, tables, and images.

### 2. Reduce Package Coupling by Moving Shared Abstractions
- Make sure `classes` and `dataitems` do not depend on each other, even by moving shared mixin and format utilities into a true core module. Example: `Parametrizable` and formatting helpers currently live in `classes` and `utils`, but are used across domain types. The benefit is fewer cycles and clearer dependency direction.

## Changes made in pymetis_pedro (2026-02-23):

  - Introduced a new core/ layer (core/dataitems, core/mixins, core/format.py, core/param.py).
  - Moved mixins out of classes/ into core/.
  - Moved prefab implementations from classes/prefab/* to recipes/prefab/*.
  - Moved input “common” into recipes.
  - Rewrote imports.

# Recipes Refactoring Recommendations (2026-03-02):

## Use factory/helper for LM/N (and similar) recipe pairs
Many LM/N recipe files are near-identical and differ only in band/detector mixins and a couple of strings. Examples: `recipes/lm_img/metis_lm_img_calibrate.py` vs `recipes/n_img/metis_n_img_calibrate.py`, and the recipes within `recipes/lm_lss/` vs `recipes/n_lss/`.

Introduce a small factory/helper (e.g., `pymetis/recipes/prefab/recipe_factory.py`) that generates the `Impl` + `MetisRecipe` classes from band + detector + metadata.

## Consolidate `InputSet` definitions
Many recipes define an `InputSet` class that only mixes in band/detector traits and inherits from a prefab InputSet, e.g. `class InputSet(BandNMixin, DetectorGeoMixin, MetisImgCalibrateImpl.InputSet): pass`.

Create base `InputSet` classes per family (IMG/LSS/IFU) and per detector type in a shared module and reuse them.

## Move algorithm logic out of recipe modules
Some recipes (especially `recipes/metis_det_dark.py`, `recipes/metis_det_lingain.py`, `recipes/cal/metis_cal_chophome.py`, `recipes/ifu/metis_ifu_rsrf.py`) include heavy implementation logic in the recipe file itself.

Move algorithmic code into `pymetis/recipes/prefab` and keep recipe modules simple.

## Standardize parameter declarations
Multiple recipes (LM/N recipe pairs for example) repeat the same `stacking.method` parameter block.

Define shared parameter builders and shared parameter sets per recipe family.

## Clean up imports and boilerplate
There are clear unused imports (e.g., `BandLmMixin` imported in `recipes/n_img/metis_n_img_flat.py` but not used). These are spread across multiple recipe files.

Eliminate unused imports and keep the recipe modules clean.

