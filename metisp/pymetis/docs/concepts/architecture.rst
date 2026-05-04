Architecture Overview
=====================

``pymetis`` is a layered Python framework that sits on top of ESO's CPL/PyCPL library.
It provides the abstractions needed to write pipeline recipes in a structured,
testable, and DRLD-traceable way.

Big picture
-----------

.. code-block:: text

                      ┌─────────────────────────────────────────────────────────┐
                      │                     pyesorex / EDPS                     │
                      │   (discovers Recipe subclasses, passes FrameSet in)     │
                      └──────────────────────────┬──────────────────────────────┘
                                                 │ calls Recipe.run(frameset, settings)
                                                 ▼
                      ┌─────────────────────────────────────────────────────────┐
                      │  Recipe  (thin shell — metadata + parameter list only)  │
                      │  ─────────────────────────────────────────────────────  │
                      │  _name, _synopsis, _description, _algorithm             │
                      │  _matched_keywords, _requirements, _templates           │
                      │  parameters: ParameterList                              │
                      │  Impl: type[RecipeImpl]   ◄── points to the algorithm  │
                      └──────────────────────────┬──────────────────────────────┘
                                                 │ instantiates and delegates to
                                                 ▼
                      ┌─────────────────────────────────────────────────────────┐
                      │  RecipeImpl  (all logic lives here)                     │
                      │  ─────────────────────────────────────────────────────  │
                      │  ┌──────────────┐  ┌──────────────┐  ┌─────────────┐  │
                      │  │  InputSet    │  │  ProductSet  │  │     Qc      │  │
                      │  │  (inputs)    │  │  (outputs)   │  │  (QC vals)  │  │
                      │  └──────────────┘  └──────────────┘  └─────────────┘  │
                      │                                                         │
                      │  process() → set[DataItem]   ◄── YOU IMPLEMENT THIS   │
                      └─────────────────────────────────────────────────────────┘

The key design decision is that **Recipe** and **RecipeImpl** are separate classes:

* ``Recipe`` contains only the metadata that ``pyesorex`` needs (name, version,
  author, parameters) plus a pointer to its implementation class.
* ``RecipeImpl`` contains all the actual processing logic inside ``process()``.

This separation makes it easy to swap out algorithm implementations, write unit
tests that target only the algorithm (without pyesorex overhead), and
introspect the recipe contract (inputs, outputs, QC params) independently of
running it.

Layers
------

**CPL / PyCPL** (external)
    The C Pipeline Library from ESO and its Python bindings.  Handles low-level
    FITS I/O, image arithmetic, and the plugin interface used by ``pyesorex``.

**Engine** (``pymetis.engine``)
    Framework-level base classes: ``Recipe``, ``RecipeImpl``, ``DataItem``,
    ``PipelineInput``, ``PipelineInputSet``, ``PipelineProductSet``,
    ``QcParameter``, and the tag/parametrization system.  These are not
    METIS-specific and could in principle be used for any instrument pipeline.

**METIS instrument layer** (``pymetis.instruments.metis``)
    Concrete implementations of the framework classes for METIS:
    all the ``DataItem`` subclasses (one per data product), all the
    ``QcParameter`` subclasses, the recipe implementations, and the
    detector/band mixins that specialize the generic classes.

**Prefab mixins** (``pymetis.instruments.metis.recipes.prefab``)
    Reusable processing components that can be mixed into recipe
    implementations to avoid code duplication: ``RawImageProcessor``,
    ``PersistenceCorrectionMixin``, and others.

Data flow through a recipe run
---------------------------------

When ``pyesorex`` calls ``Recipe.run(frameset, settings)``:

1. ``Recipe.run`` instantiates ``RecipeImpl(recipe, frameset, settings)``.
2. The ``RecipeImpl.__init__`` creates an ``InputSet`` object, which matches
   frames from the frameset to the declared ``PipelineInput`` members by tag.
3. The ``InputSet`` validates that required inputs are present.
4. ``RecipeImpl.promote()`` looks at the loaded frame tags and promotes
   generic product classes to their concrete specializations
   (e.g. ``MasterDark`` → ``MasterDark2rg``).
5. ``RecipeImpl.run()`` calls ``process()``, which loads pixel data lazily,
   performs the reduction, and returns a ``set[DataItem]``.
6. Each ``DataItem`` in the set is saved to disk via ``DataItem.save()``.
7. A CPL ``FrameSet`` of the output files is returned to ``pyesorex``.

What to read next
-----------------

* :doc:`recipe_impl_pattern` — the Recipe / RecipeImpl split in detail
* :doc:`data_items` — how FITS products are described with ``DataItem``
* :doc:`inputs` — how input frames are declared and validated
* :doc:`parametrization` — the tag system that handles band/detector variants
