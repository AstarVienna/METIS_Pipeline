"""
autorecipe — Custom Sphinx extension for pymetis recipe and data-item documentation.

Provides two directives:

    .. autorecipe:: pymetis.instruments.metis.recipes.metis_det_dark.MetisDetDark

        Generates a structured recipe reference page mirroring the DRLD ``recipedef``
        block layout: purpose, algorithm, inputs, outputs, QC parameters, and recipe
        parameters.

    .. autodataitem:: pymetis.instruments.metis.dataitems.masterdark.masterdark.MasterDark

        Generates a data-product reference block mirroring the DRLD ``datastructdef``
        block: PRO.CATG tag, FITS HDU schema, frame level/group, OCA keywords.

At build time both directives also write machine-readable JSON manifests to
``<BUILDDIR>/drld_manifest/recipes/`` and ``<BUILDDIR>/drld_manifest/dataitems/``
respectively.  A future LaTeX-generation script can read these manifests and render
them into DRLD ``.tex`` files.
"""

from __future__ import annotations

import importlib
import inspect
import json
import os
from pathlib import Path
from typing import Any

from docutils import nodes
from docutils.parsers.rst import Directive
from docutils.statemachine import ViewList
from sphinx.application import Sphinx
from sphinx.util import logging
from sphinx.util.docutils import SphinxDirective

logger = logging.getLogger(__name__)


# ---------------------------------------------------------------------------
# Helper utilities
# ---------------------------------------------------------------------------

def _import_class(dotted_path: str):
    """Import a class from a dotted module path, e.g. ``a.b.c.MyClass``."""
    module_path, _, class_name = dotted_path.rpartition('.')
    try:
        module = importlib.import_module(module_path)
        return getattr(module, class_name)
    except (ImportError, AttributeError) as exc:
        logger.warning(f"autorecipe: cannot import {dotted_path!r}: {exc}")
        return None


def _get_inner_classes(container_class, base_class):
    """Return (attr_name, cls) pairs for inner classes that subclass *base_class*."""
    if container_class is None:
        return []
    return [
        (name, cls)
        for name, cls in inspect.getmembers(container_class, inspect.isclass)
        if issubclass(cls, base_class) and cls is not base_class
    ]


def _schema_to_str(schema: dict) -> str:
    """Render a DataItem._schema dict as a human-readable string."""
    lines = []
    for ext_name, ext_type in schema.items():
        type_name = ext_type.__name__ if ext_type is not None else '(header only)'
        lines.append(f"{ext_name}: {type_name}")
    return '\n'.join(lines)


def _rst_field(label: str, content: str, indent: int = 3) -> list[str]:
    """Return RST lines for a definition-list style field."""
    pad = ' ' * indent
    lines = [f'**{label}**']
    for line in content.splitlines():
        lines.append(f'{pad}{line}' if line.strip() else '')
    lines.append('')
    return lines


# ---------------------------------------------------------------------------
# Recipe manifest extraction
# ---------------------------------------------------------------------------

def _extract_recipe_manifest(recipe_class) -> dict[str, Any]:
    """
    Extract all DRLD-relevant fields from a Recipe class and its Impl.

    Returns a dict that mirrors the fields of a DRLD ``recipedef`` block.
    """
    impl = getattr(recipe_class, 'Impl', None)

    # --- inputs ---
    inputs = []
    if impl is not None and getattr(impl, 'InputSet', None) is not None:
        try:
            from pymetis.engine.inputs import PipelineInput
            for name, cls in _get_inner_classes(impl.InputSet, PipelineInput):
                item_cls = getattr(cls, 'Item', None)
                item_name = item_cls.name() if item_cls is not None else '<unknown>'
                item_title = getattr(item_cls, '_title_template', '') or ''
                required = getattr(cls, '_required', True)
                multiplicity = getattr(cls, '_multiplicity', '1')
                inputs.append({
                    'name': name,
                    'item': item_name,
                    'title': item_title,
                    'required': required,
                    'multiplicity': multiplicity,
                })
        except Exception as exc:
            logger.debug(f"autorecipe: failed to extract inputs for {recipe_class}: {exc}")

    # --- outputs ---
    outputs = []
    if impl is not None and getattr(impl, 'ProductSet', None) is not None:
        try:
            from pymetis.engine.dataitems import DataItem
            for name, cls in _get_inner_classes(impl.ProductSet, DataItem):
                schema = getattr(cls, '_schema', {})
                outputs.append({
                    'name': name,
                    'item': cls.name() if hasattr(cls, 'name') else name,
                    'title': cls.title() if hasattr(cls, 'title') else '',
                    'schema': {k: (v.__name__ if v else None) for k, v in schema.items()},
                    'frame_level': str(getattr(cls, '_frame_level', '')),
                    'frame_group': str(getattr(cls, '_frame_group', '')),
                })
        except Exception as exc:
            logger.debug(f"autorecipe: failed to extract outputs for {recipe_class}: {exc}")

    # --- QC parameters ---
    qc_params = []
    if impl is not None and getattr(impl, 'Qc', None) is not None:
        try:
            from pymetis.engine.qc import QcParameter
            for name, cls in _get_inner_classes(impl.Qc, QcParameter):
                qc_params.append({
                    'name': cls.name() if hasattr(cls, 'name') else name,
                    'description': cls.description() if hasattr(cls, 'description') else '',
                    'unit': getattr(cls, '_unit', ''),
                    'dtype': getattr(cls, '_dtype', ''),
                })
        except Exception as exc:
            logger.debug(f"autorecipe: failed to extract QC params for {recipe_class}: {exc}")

    # --- recipe parameters ---
    parameters = []
    param_list = getattr(recipe_class, 'parameters', None)
    if param_list is not None:
        try:
            for p in param_list:
                parameters.append({
                    'name': str(p.name),
                    'description': str(getattr(p, 'description', '')),
                    'default': str(getattr(p, 'default', '')),
                    'alternatives': list(getattr(p, 'alternatives', ()) or ()),
                })
        except Exception as exc:
            logger.debug(f"autorecipe: failed to extract parameters for {recipe_class}: {exc}")

    return {
        'class': f"{recipe_class.__module__}.{recipe_class.__qualname__}",
        'name': getattr(recipe_class, '_name', ''),
        'synopsis': getattr(recipe_class, '_synopsis', ''),
        'description': getattr(recipe_class, '_description', ''),
        'algorithm': getattr(recipe_class, '_algorithm', '') or getattr(impl, '_algorithm', ''),
        'matched_keywords': sorted(getattr(recipe_class, '_matched_keywords', set()) or set()),
        'requirements': sorted(getattr(recipe_class, '_requirements', set()) or set()),
        'templates': sorted(getattr(recipe_class, '_templates', set()) or set()),
        'expected_accuracy': getattr(impl, '_expected_accuracy', '') if impl else '',
        'inputs': inputs,
        'outputs': outputs,
        'qc_parameters': qc_params,
        'parameters': parameters,
    }


# ---------------------------------------------------------------------------
# DataItem manifest extraction
# ---------------------------------------------------------------------------

def _extract_dataitem_manifest(dataitem_class) -> dict[str, Any]:
    """Extract all DRLD-relevant fields from a DataItem class."""
    schema = getattr(dataitem_class, '_schema', {})
    return {
        'class': f"{dataitem_class.__module__}.{dataitem_class.__qualname__}",
        'name': dataitem_class.name() if hasattr(dataitem_class, 'name') else '',
        'title': dataitem_class.title() if hasattr(dataitem_class, 'title') else '',
        'description': dataitem_class.description() if hasattr(dataitem_class, 'description') else '',
        'frame_group': str(getattr(dataitem_class, '_frame_group', '')),
        'frame_level': str(getattr(dataitem_class, '_frame_level', '')),
        'frame_type': str(getattr(dataitem_class, '_frame_type', '')),
        'oca_keywords': sorted(getattr(dataitem_class, '_oca_keywords', set()) or set()),
        'dpr_catg': getattr(dataitem_class, '_dpr_catg', ''),
        'dpr_tech': getattr(dataitem_class, '_dpr_tech', ''),
        'dpr_type': getattr(dataitem_class, '_dpr_type', ''),
        'schema': {k: (v.__name__ if v else None) for k, v in schema.items()},
    }


# ---------------------------------------------------------------------------
# RST generation helpers
# ---------------------------------------------------------------------------

def _recipe_to_rst(manifest: dict) -> list[str]:
    """Convert a recipe manifest dict to a list of RST lines."""
    lines: list[str] = []
    name = manifest['name']

    lines += [
        f".. _{name}:",
        '',
        name,
        '=' * len(name),
        '',
        f"**Synopsis:** {manifest['synopsis']}",
        '',
    ]

    if manifest['description']:
        lines += [manifest['description'].strip(), '']

    # Requirements
    if manifest['requirements']:
        lines += ['**Requirements**', '']
        for req in manifest['requirements']:
            lines.append(f'- ``{req}``')
        lines.append('')

    # Templates
    if manifest['templates']:
        lines += ['**Observation Templates**', '']
        for tpl in manifest['templates']:
            lines.append(f'- ``{tpl}``')
        lines.append('')

    # Matched keywords
    if manifest['matched_keywords']:
        lines += ['**Matched Keywords**', '']
        for kw in manifest['matched_keywords']:
            lines.append(f'- ``{kw}``')
        lines.append('')

    # Inputs
    lines += ['**Input Frames**', '']
    if manifest['inputs']:
        lines += [
            '.. list-table::',
            '   :header-rows: 1',
            '   :widths: 20 30 10 10',
            '',
            '   * - Attribute',
            '     - PRO.CATG / Tag',
            '     - Required',
            '     - Multiplicity',
        ]
        for inp in manifest['inputs']:
            req_str = 'yes' if inp['required'] else 'no (optional)'
            lines += [
                f'   * - ``{inp["name"]}``',
                f'     - ``{inp["item"]}``',
                f'     - {req_str}',
                f'     - {inp["multiplicity"]}',
            ]
    else:
        lines.append('*(none defined)*')
    lines.append('')

    # Outputs
    lines += ['**Output Products**', '']
    if manifest['outputs']:
        lines += [
            '.. list-table::',
            '   :header-rows: 1',
            '   :widths: 20 30 30',
            '',
            '   * - Attribute',
            '     - PRO.CATG / Tag',
            '     - FITS Extensions',
        ]
        for out in manifest['outputs']:
            schema_str = ', '.join(f'``{k}`` ({v or "hdr"})' for k, v in out['schema'].items())
            lines += [
                f'   * - ``{out["name"]}``',
                f'     - ``{out["item"]}``',
                f'     - {schema_str}',
            ]
    else:
        lines.append('*(none defined)*')
    lines.append('')

    # Algorithm
    lines += ['**Algorithm**', '']
    algo = (manifest['algorithm'] or '').strip()
    if algo:
        for line in algo.splitlines():
            lines.append(line)
    else:
        lines.append('*(no algorithm description provided)*')
    lines.append('')

    # Expected accuracy
    if manifest['expected_accuracy']:
        lines += ['**Expected Accuracy**', '']
        lines.append(manifest['expected_accuracy'].strip())
        lines.append('')

    # Recipe parameters
    lines += ['**Recipe Parameters**', '']
    if manifest['parameters']:
        lines += [
            '.. list-table::',
            '   :header-rows: 1',
            '   :widths: 35 40 15',
            '',
            '   * - Name',
            '     - Description',
            '     - Default',
        ]
        for p in manifest['parameters']:
            alts = f" (choices: {', '.join(p['alternatives'])})" if p['alternatives'] else ''
            lines += [
                f'   * - ``{p["name"]}``',
                f'     - {p["description"]}{alts}',
                f'     - ``{p["default"]}``',
            ]
    else:
        lines.append('*(no parameters)*')
    lines.append('')

    # QC parameters
    lines += ['**QC Parameters**', '']
    if manifest['qc_parameters']:
        lines += [
            '.. list-table::',
            '   :header-rows: 1',
            '   :widths: 35 15 40',
            '',
            '   * - Name',
            '     - Unit',
            '     - Description',
        ]
        for qc in manifest['qc_parameters']:
            lines += [
                f'   * - ``{qc["name"]}``',
                f'     - {qc["unit"] or "—"}',
                f'     - {qc["description"] or "—"}',
            ]
    else:
        lines.append('*(none defined)*')
    lines.append('')

    # Source cross-reference
    cls_path = manifest['class']
    lines += [
        '**Source**',
        '',
        f'.. autoclass:: {cls_path}',
        '   :noindex:',
        '   :no-members:',
        '',
    ]

    return lines


def _dataitem_to_rst(manifest: dict) -> list[str]:
    """Convert a data-item manifest dict to a list of RST lines."""
    lines: list[str] = []
    tag = manifest['name']
    title_str = manifest['title'] or tag

    lines += [
        f'.. _{tag}:',
        '',
        tag,
        '=' * len(tag),
        '',
        f"**Title:** {title_str}",
        '',
    ]

    if manifest['description']:
        lines += [manifest['description'].strip(), '']

    # Identification fields (DRLD DPR fields)
    id_rows = [
        ('PRO.CATG', f'``{tag}``'),
        ('DPR.CATG', manifest['dpr_catg'] or '—'),
        ('DPR.TECH', manifest['dpr_tech'] or '—'),
        ('DPR.TYPE', manifest['dpr_type'] or '—'),
        ('Frame group', str(manifest['frame_group'])),
        ('Frame level', str(manifest['frame_level'])),
    ]

    lines += [
        '.. list-table:: Identification',
        '   :header-rows: 0',
        '   :widths: 30 70',
        '',
    ]
    for label, value in id_rows:
        lines += [f'   * - {label}', f'     - {value}']
    lines.append('')

    # OCA keywords
    if manifest['oca_keywords']:
        lines += ['**OCA Keywords**', '']
        for kw in manifest['oca_keywords']:
            lines.append(f'- ``{kw}``')
        lines.append('')

    # FITS HDU schema
    lines += [
        '**FITS Structure**',
        '',
        '.. list-table::',
        '   :header-rows: 1',
        '   :widths: 40 60',
        '',
        '   * - Extension (EXTNAME)',
        '     - Content',
    ]
    for ext_name, ext_type in manifest['schema'].items():
        content = ext_type if ext_type else 'header only (no data)'
        lines += [f'   * - ``{ext_name}``', f'     - {content}']
    lines.append('')

    # Source cross-reference
    cls_path = manifest['class']
    lines += [
        '**Source**',
        '',
        f'.. autoclass:: {cls_path}',
        '   :noindex:',
        '   :no-members:',
        '',
    ]

    return lines


# ---------------------------------------------------------------------------
# Sphinx directives
# ---------------------------------------------------------------------------

class AutoRecipeDirective(SphinxDirective):
    """
    ``.. autorecipe:: <dotted.path.to.RecipeClass>``

    Generates a structured recipe reference page and writes the DRLD JSON manifest.
    """
    required_arguments = 1
    optional_arguments = 0
    has_content = False

    def run(self) -> list[nodes.Node]:
        dotted_path = self.arguments[0].strip()
        recipe_class = _import_class(dotted_path)
        if recipe_class is None:
            return [self.state_machine.reporter.warning(
                f"autorecipe: could not import {dotted_path!r}",
                line=self.lineno,
            )]

        manifest = _extract_recipe_manifest(recipe_class)
        _write_manifest(self.env.app, 'recipes', manifest['name'], manifest)

        rst_lines = _recipe_to_rst(manifest)
        return _rst_to_nodes(self, rst_lines)


class AutoDataItemDirective(SphinxDirective):
    """
    ``.. autodataitem:: <dotted.path.to.DataItemClass>``

    Generates a structured data-product reference block and writes the DRLD JSON manifest.
    """
    required_arguments = 1
    optional_arguments = 0
    has_content = False

    def run(self) -> list[nodes.Node]:
        dotted_path = self.arguments[0].strip()
        dataitem_class = _import_class(dotted_path)
        if dataitem_class is None:
            return [self.state_machine.reporter.warning(
                f"autodataitem: could not import {dotted_path!r}",
                line=self.lineno,
            )]

        manifest = _extract_dataitem_manifest(dataitem_class)
        _write_manifest(self.env.app, 'dataitems', manifest['name'], manifest)

        rst_lines = _dataitem_to_rst(manifest)
        return _rst_to_nodes(self, rst_lines)


# ---------------------------------------------------------------------------
# RST → docutils nodes helper
# ---------------------------------------------------------------------------

def _rst_to_nodes(directive: Directive, rst_lines: list[str]) -> list[nodes.Node]:
    """Parse a list of RST lines into docutils nodes in the current document."""
    vl = ViewList()
    source = directive.get_source_info()[0]
    for i, line in enumerate(rst_lines):
        vl.append(line, source, i)

    node = nodes.section()
    node.document = directive.state.document
    directive.state.nested_parse(vl, directive.content_offset, node)
    return node.children


# ---------------------------------------------------------------------------
# JSON manifest writing
# ---------------------------------------------------------------------------

def _write_manifest(app: Sphinx, category: str, name: str, data: dict) -> None:
    """Write a JSON manifest file for a recipe or data item."""
    try:
        manifest_dir = Path(app.outdir).parent / app.config.autorecipe_manifest_dir / category
        manifest_dir.mkdir(parents=True, exist_ok=True)
        safe_name = (name or 'unknown').replace('/', '_').replace('{', '').replace('}', '')
        out_path = manifest_dir / f"{safe_name}.json"
        with open(out_path, 'w', encoding='utf-8') as fh:
            json.dump(data, fh, indent=2, default=str)
    except Exception as exc:
        logger.debug(f"autorecipe: failed to write manifest for {name!r}: {exc}")


# ---------------------------------------------------------------------------
# Sphinx setup
# ---------------------------------------------------------------------------

def setup(app: Sphinx) -> dict:
    app.add_config_value('autorecipe_root_module', 'pymetis.instruments.metis', 'env')
    app.add_config_value('autorecipe_manifest_dir', os.path.join('_build', 'drld_manifest'), 'env')

    app.add_directive('autorecipe', AutoRecipeDirective)
    app.add_directive('autodataitem', AutoDataItemDirective)

    return {
        'version': '0.1',
        'parallel_read_safe': True,
        'parallel_write_safe': True,
    }
