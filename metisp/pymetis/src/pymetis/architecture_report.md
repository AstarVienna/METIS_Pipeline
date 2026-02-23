# Architecture Report

## Pedro Beirao, Feb 2026

At the first glance here are my observations and suggestions of pymetis architecture:

## Observations
- The core runtime is built around `MetisRecipe` + `MetisRecipeImpl`, with dynamic promotion and class introspection used to assemble inputs, products, and QC metadata. See `src/pymetis/classes/recipes/recipe.py` and `src/pymetis/classes/recipes/impl.py`.
- Domain types (`DataItem` for example) are tightly coupled to CPL and pyesorex APIs. 
- Recipes are heavy consumers that depend on `classes`, `dataitems`, `qc`, `utils`, and sometimes `functions`, which indicates high coupling.

## Risks & Architectural Friction
- Coupling to CPL/pyesorex inside domain classes reduces testability and blocks reuse of logic outside the CPL runtime.
- The `classes` and `dataitems` packages depend on each other, which can complicate refactors and tests.

### Example: `DataItem` Depends Directly on CPL Types and Metadata
- `DataItem` imports and uses CPL directly:
- `import cpl`
- `from cpl.core import Msg, Image, Table, PropertyList, Property`
- It stores CPL-specific metadata like `_frame_group`, `_frame_level`, `_frame_type`.
- Its `__init__` signature expects a `cpl.core.PropertyList` and `Hdu` objects, which themselves wrap CPL objects.

This reduces testability unit tests on `DataItem` behavior cannot be performed without a CPL runtime available. Even simple tests of metadata formatting require CPL object creation (e.g., `cpl.core.PropertyList()`). Mocking CPL is possible but cumbersome because the class imports and calls CPL types directly.

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

## Main Recommendations

### 1. Split the Architecture Into Explicit Layers
- Create a new domain layer (e. g. `pymetis.core`) with pure-Python models and metadata (`DataItem` spec, `Input` spec, `QC` spec) that has zero CPL dependency.
- Keep CPL/pyesorex specifics in a layer that turns core objects into CPL frames, tables, and images.
- Keep recipe orchestration in `pymetis.recipes` but make it consume only the core interfaces + adapters.

### 2. Reduce Package Coupling by Moving Shared Abstractions
- Make sure `classes` and `dataitems` do not depend on each other, even by moving shared mixin and format utilities into a true core module. Example: `Parametrizable` and formatting helpers currently live in `classes` and `utils`, but are used across domain types. The benefit is fewer cycles and clearer dependency direction.

