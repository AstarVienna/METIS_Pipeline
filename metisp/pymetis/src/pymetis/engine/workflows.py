"""
This file is part of the METIS Pipeline.
Copyright (C) 2024 European Southern Observatory

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

The EDPS workflows, read from Python: load a workflow module offline and bind each of its
tasks to the recipe it runs.

Everything that touches edps internals lives here, and only here. The workflow is the
authority on *what EDPS associates* (per task: main input, associated inputs, how many,
whether optional); the recipe's `InputSet` is the authority on *what the recipe accepts*.
`bind()` compares the two and reports every disagreement, for the tests and for the
association maps of the DRLD generator.

The workflow package `metis` is not installed; it is found in `$METIS_WORKFLOWS`, else in
the `workflows` directory next to `pymetis` in the source tree.
"""
import importlib
import importlib.metadata
import logging
import os
import sys
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any

import cpl

from pymetis.engine.dataitems import DataItem
from pymetis.engine.recipes import Recipe

SUPPORTED_EDPS = '1.7'
INFREQUENT_DAYS = 30        # a main input matched over a month or more is an infrequent (AIT / monthly) calibration


def workflows_directory() -> Path | None:
    """ Where the `metis` workflow package lives, or None when it is not around (installed package). """
    candidates = [Path(p) for p in [os.environ.get('METIS_WORKFLOWS')] if p]
    candidates.append(Path(__file__).resolve().parents[4] / 'workflows')
    return next((c for c in candidates if (c / 'metis' / '__init__.py').exists()), None)


def _edps():
    """ The edps generator API this module relies on, checked against the version it was written for. """
    version = importlib.metadata.version('edps')
    if not version.startswith(SUPPORTED_EDPS):
        raise ImportError(f"pymetis.engine.workflows was written against edps {SUPPORTED_EDPS}.x; found {version}. "
                          f"Check the task / data-source attributes it uses before lifting this check.")
    from edps.generator import task as edps_task
    from edps.generator.workflow_manager import WorkflowManager
    for name in ('DataSource', 'AssociationConfiguration', 'edps', 'Task', 'Workflow'):
        logging.getLogger(name).setLevel(logging.WARNING)
    return edps_task, WorkflowManager


def load_workflow(module_name: str, workflows_dir: Path | None = None):
    """
    The `edps.generator.workflow.Workflow` of a workflow module, e.g. 'metis.metis_lm_img_wkf',
    built offline (no EDPS server). Tasks and data sources are module-level singletons, so a
    module loaded twice in one process yields the same objects.
    """
    directory = workflows_dir or workflows_directory()
    if directory is None:
        raise FileNotFoundError("the METIS workflow package was not found; set $METIS_WORKFLOWS to the "
                                "directory holding `metis/`")
    if str(directory) not in sys.path:
        sys.path.insert(0, str(directory))
    edps_task, WorkflowManager = _edps()
    module = importlib.import_module(module_name)
    workflow = WorkflowManager(None).create_workflow(module)
    # The order the tasks are written in (a module that imports other workflow modules inherits
    # theirs, in import order): the tie-breaker of `tasks()`, so a map is drawn the same whichever
    # modules were loaded before.
    order: dict[int, int] = {}
    seen: set[int] = set()

    def walk(mod) -> None:
        if id(mod) in seen:
            return
        seen.add(id(mod))
        for value in list(vars(mod).values()):
            if isinstance(value, type(module)) and getattr(value, '__name__', '').startswith(module_name.split('.')[0] + '.'):
                walk(value)
            elif isinstance(value, edps_task.Task):
                order.setdefault(id(value), len(order))
    walk(module)
    workflow._pymetis_definition_order = order
    return workflow


# --- the classification rules --------------------------------------------------------------

DPR_KEYWORDS = tuple(k.edps for k in DataItem.DPR)      # ('dpr.catg', 'dpr.tech', 'dpr.type'): the workflow package's spelling


def classification_rules(workflows_dir: Path | None = None) -> dict[str, dict[str, Any]]:
    """
    The dictionary classification rules of `metis.metis_classification`, keyed by the
    classification (tag) they assign: tag -> {keyword: value}. Raw data is classified by
    its DPR triple, products by `pro.catg`.
    """
    directory = workflows_dir or workflows_directory()
    if directory is None:
        raise FileNotFoundError("the METIS workflow package was not found; set $METIS_WORKFLOWS to the "
                                "directory holding `metis/`")
    if str(directory) not in sys.path:
        sys.path.insert(0, str(directory))
    _edps()
    module = importlib.import_module('metis.metis_classification')
    rules: dict[str, dict[str, Any]] = {}
    for value in vars(module).values():
        if hasattr(value, 'classification') and isinstance(getattr(value, 'keyword_values', None), dict):
            rules[value.classification] = dict(value.keyword_values)
    return rules


def rule_dpr(rule: dict[str, Any]) -> tuple[Any, Any, Any]:
    """ The (DPR.CATG, DPR.TECH, DPR.TYPE) a rule requires; None where it leaves a keyword free. """
    return tuple(rule.get(key) for key in DPR_KEYWORDS)


# --- the keyword strings the workflows use ---------------------------------------------------

def keyword_usage(module_names) -> dict[str, set[str]]:
    """
    Every header keyword string the workflow package uses, in its own (lowercase) spelling,
    and where: 'grouping:<data source>', 'setup:<data source>', 'match:<data source>',
    'rule:<classification>', 'constant:<name>' (metis_keywords.py), 'read:<function>'
    (get_keyword_value() calls in the task functions). The test against the vocabulary
    reads this; the workflow package itself never imports pymetis.
    """
    import re
    from collections import defaultdict
    out: dict[str, set[str]] = defaultdict(set)
    seen: set[int] = set()
    for module in module_names:
        for task in tasks(load_workflow(module)):
            sources = [task.main_input] + [a.input_task for a in task.flatten_associated_inputs()]
            for source in sources:
                if not is_data_source(source) or id(source) in seen:
                    continue
                seen.add(id(source))
                for k in source.grouping_keywords or ():
                    out[k].add(f"grouping:{source.name}")
                for k in source.setup_keywords or ():
                    out[k].add(f"setup:{source.name}")
                for k in source.match_keywords or ():
                    out[k].add(f"match:{source.name}")
                for config in source.assoc_configs or ():
                    for k in config.match_keywords or ():
                        out[k].add(f"match:{source.name}")
                for rule in source.classification_rules or ():
                    for k in getattr(rule, 'keyword_values', {}) or {}:
                        out[k].add(f"rule:{rule.classification}")
    directory = workflows_directory()
    if directory is not None:
        constants = importlib.import_module('metis.metis_keywords')
        for name, value in vars(constants).items():
            if isinstance(value, str) and not name.startswith('_'):
                out[value].add(f"constant:{name}")
            elif isinstance(value, tuple) and not name.startswith('_'):
                for v in value:
                    out[v].add(f"constant:{name}")
        functions = (directory / 'metis' / 'metis_task_functions.py').read_text()
        for m in re.finditer(r'get_keyword_value\(\s*["\']([^"\']+)["\']', functions):
            out[m.group(1)].add("read:metis_task_functions")
    return dict(out)


# --- recipe parameter overrides of the workflows -----------------------------------------------

# What `metis_task_functions.instrument_to_linlimit` injects per detector, mirrored here for the documentation.
LINLIMIT_PER_TECH = {'LM': 22100, 'N': 13000, 'IFU': 44000}


def parameter_overrides() -> dict[str, dict[str, list[tuple[Any, str]]]]:
    """
    The recipe-parameter defaults the workflows override: recipe -> parameter -> [(value, task)].
    From `metis_parameters.yaml` (default_parameters.recipe_parameters, keyed by task, entries
    `<recipe>.<parameter>: value`) plus the per-detector linearity limit the lingain task
    function sets. Empty when the workflow package is not around.
    """
    directory = workflows_directory()
    if directory is None:
        return {}
    import yaml
    out: dict[str, dict[str, list[tuple[Any, str]]]] = {}
    config = yaml.safe_load((directory / 'metis' / 'metis_parameters.yaml').read_text()) or {}
    for task, parameters in (config.get('default_parameters', {}).get('recipe_parameters') or {}).items():
        for name, value in (parameters or {}).items():
            recipe = name.split('.')[0]
            out.setdefault(recipe, {}).setdefault(name, []).append((value, task))
    for tech, limit in LINLIMIT_PER_TECH.items():
        out.setdefault('metis_det_lingain', {}).setdefault('metis_det_lingain.linlimit', []).append((limit, f"DPR.TECH {tech}"))
    return out


# --- reading a workflow -------------------------------------------------------------------

def tasks(workflow) -> list:
    """
    The tasks of a workflow in a deterministic topological order: by dependency depth (the
    longest chain of tasks feeding a task), then by the order the tasks are defined in the
    module. `Workflow.topological_sort()` alone is a valid order too, but which one it yields
    depends on the modules loaded before.
    """
    edps_task, _ = _edps()
    nodes = [node for node in workflow.topological_sort() if isinstance(node, edps_task.Task)]
    depth: dict[int, int] = {}
    for task in nodes:                                  # topological: every predecessor is already done
        preds = [task.main_input] + [a.input_task for a in task.flatten_associated_inputs()]
        depth[id(task)] = 1 + max((depth.get(id(p), -1) for p in preds if isinstance(p, edps_task.Task)), default=-1)
    definition = getattr(workflow, '_pymetis_definition_order', {})
    return sorted(nodes, key=lambda t: (depth[id(t)], definition.get(id(t), len(definition)), t.name))


def is_data_source(node) -> bool:
    edps_task, _ = _edps()
    return isinstance(node, edps_task.DataSource)


def rule_tags(rules) -> list[str]:
    return [rule.classification for rule in (rules or [])]


def main_tags(task) -> list[str]:
    """ The tags a task's main input delivers: a data source's classifications, or the product classes requested from an upstream task. """
    if is_data_source(task.main_input):
        return rule_tags(task.main_input.classification_rules)
    return rule_tags(task.accepted_classification_rules)


def assoc_tags(assoc) -> list[str]:
    """ The tags an associated input delivers: the product classes requested from a task, or a data source's classifications. """
    if assoc.categories:
        return sorted(assoc.categories)
    if assoc.classification_rules:
        return rule_tags(assoc.classification_rules)
    if is_data_source(assoc.input_task):
        return rule_tags(assoc.input_task.classification_rules)
    return []


def is_optional(assoc) -> bool:
    """ EDPS runs the task without this input when `min_ret` is 0 or a workflow condition can switch it off. """
    edps_task, _ = _edps()
    return assoc.min_ret == 0 or assoc.condition is not edps_task.TRUE_CONDITION


def _days(time_range) -> float:
    start = getattr(time_range, 'start', None)
    if start is None:                       # RelativeTimeRange prints as "-1, 1"
        start = str(time_range).split(',')[0]
    return abs(float(start))


def is_infrequent(task) -> bool:
    """ A task whose main raw data is matched over a month or more: an AIT / monthly calibration
    (the dashed separator of the DRLD association maps). Tasks fed by another task count as daily. """
    if not is_data_source(task.main_input):
        return False
    level0 = [c for c in task.main_input.assoc_configs if c.level == 0]
    if not level0:
        return False
    days = _days(level0[0].time_range)
    return INFREQUENT_DAYS <= days < float('inf')       # unlimited = science data, not a calibration cadence


def consumers(task, tasks_of_map) -> list[tuple[Any, str, list[str], bool]]:
    """ (consumer task, 'main' | 'assoc', tags taken, optional) for every task of the map that reads this task's products. """
    out = []
    for other in tasks_of_map:
        if other.main_input is task:
            out.append((other, 'main', rule_tags(other.accepted_classification_rules), False))
        for assoc in other.flatten_associated_inputs():
            if assoc.input_task is task:
                out.append((other, 'assoc', assoc_tags(assoc), is_optional(assoc)))
    return out


def requested_products(task, tasks_of_map) -> list[str]:
    """
    The product tags of a task as far as this map is concerned: what its consumers ask for,
    plus its output filter. Never `task.categories`, which accumulates across every workflow
    module loaded in the process.
    """
    tags: list[str] = []
    for _, _, taken, _ in consumers(task, tasks_of_map):
        tags += [t for t in taken if t not in tags]
    tags += [t for t in (task.output_filter or []) if t not in tags]
    return tags


# --- binding a task to its recipe ------------------------------------------------------------

@dataclass
class InputBinding:
    tag: str
    role: str                       # 'main' or 'assoc'
    accepted_by: list[str]          # attribute names of the recipe inputs whose Item covers the tag
    raw_role: bool                  # some accepting input has the RAW role (PrimaryInputMixin)
    min_ret: int
    max_ret: int
    optional: bool                  # EDPS side: min_ret == 0 or conditional


@dataclass
class TaskBinding:
    workflow: str
    task: str
    recipe: str
    inputs: list[InputBinding] = field(default_factory=list)
    unaccepted: list[str] = field(default_factory=list)             # fed tags no recipe input covers
    unknown: list[str] = field(default_factory=list)                # fed tags that are no catalogue item at all
    main_not_raw_role: list[str] = field(default_factory=list)      # main-input tags accepted only by CALIB-role inputs
    optionality: list[str] = field(default_factory=list)            # "TAG: workflow optional, recipe required" and vice versa
    multiplicity: list[str] = field(default_factory=list)           # inputs taking N frames fed with max_ret 1
    missing_required: list[str] = field(default_factory=list)       # recipe inputs (attribute: Item) no task input feeds
    missing_optional: list[str] = field(default_factory=list)
    products_not_declared: list[str] = field(default_factory=list)  # tags consumers request that the recipe does not produce

    @property
    def problems(self) -> list[str]:
        out = []
        for label in ('unaccepted', 'unknown', 'main_not_raw_role', 'optionality', 'multiplicity',
                      'missing_required', 'products_not_declared'):
            out += [f"{label}: {entry}" for entry in getattr(self, label)]
        return out


def bind(task, tasks_of_map=(), workflow: str = '') -> TaskBinding:
    """ Compare what a workflow task feeds its recipe with what the recipe's InputSet declares. """
    recipe = Recipe._registry.get(task.command)
    binding = TaskBinding(workflow=workflow, task=task.name, recipe=task.command)
    if recipe is None:
        binding.unknown.append(f"recipe {task.command} is not registered")
        return binding
    inputs = recipe.Impl.InputSet.list_input_classes()          # specialized with the recipe's own tags at import

    fed: list[tuple[str, str, int, int, bool]] = [(t, 'main', 1, 1, False) for t in main_tags(task)]
    for assoc in task.flatten_associated_inputs():
        fed += [(t, 'assoc', assoc.min_ret, assoc.max_ret, is_optional(assoc)) for t in assoc_tags(assoc)]

    fed_items: dict[str, type[DataItem] | None] = {}
    for tag, role, min_ret, max_ret, optional in fed:
        item = DataItem.find(tag)
        fed_items[tag] = item
        if item is None:
            binding.unknown.append(tag)
            continue
        accepting = [(name, inp) for name, inp in inputs if issubclass(item, inp.Item)]
        raw_role = any(inp._group == cpl.ui.Frame.FrameGroup.RAW for _, inp in accepting)
        binding.inputs.append(InputBinding(tag, role, [n for n, _ in accepting], raw_role, min_ret, max_ret, optional))
        if not accepting:
            binding.unaccepted.append(tag)
            continue
        if role == 'main' and not raw_role:
            binding.main_not_raw_role.append(tag)
        if role == 'assoc':
            required = all(inp.required() for _, inp in accepting)
            if optional and required:
                binding.optionality.append(f"{tag}: workflow optional, recipe requires it")
            if not optional and not required:
                binding.optionality.append(f"{tag}: workflow requires it, recipe has it optional")
            if max_ret == 1 and any(inp.multiplicity() == 'N' for _, inp in accepting):
                binding.multiplicity.append(f"{tag}: recipe takes N frames, workflow max_ret=1")

    for name, inp in inputs:
        if not any(item is not None and issubclass(item, inp.Item) for item in fed_items.values()):
            (binding.missing_required if inp.required() else binding.missing_optional).append(f"{name}: {inp.Item.name()}")

    produced = set()
    for _, product in recipe._list_products():
        produced.add(product.name())
    for tag in requested_products(task, tasks_of_map):
        if tag not in produced and not any(_matches_template(tag, template) for template in produced):
            binding.products_not_declared.append(tag)
    return binding


def _matches_template(tag: str, template: str) -> bool:
    import re
    if '{' not in template:
        return tag == template
    return re.fullmatch(re.sub(r'\\\{\w+\\\}', '[A-Z0-9]+', re.escape(template)), tag) is not None
