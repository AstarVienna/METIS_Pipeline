"""
Sphinx configuration for the pymetis documentation.
"""

import os
import sys

# Make the pymetis source importable for autodoc
sys.path.insert(0, os.path.abspath('../src'))
# Make the local extensions importable
sys.path.insert(0, os.path.abspath('_ext'))

# -- Project information -----------------------------------------------------
project = 'pymetis'
copyright = '2024, A* Pipeline Team / European Southern Observatory'
author = 'A* Pipeline Team'
release = '0.1.0'

# -- General configuration ---------------------------------------------------
extensions = [
    'sphinx.ext.autodoc',
    'sphinx.ext.autosummary',
    'sphinx.ext.napoleon',
    'sphinx.ext.viewcode',
    'sphinx.ext.intersphinx',
    'sphinx.ext.todo',
    'sphinx_autodoc_typehints',
    'autorecipe',          # local extension: _ext/autorecipe.py
]

autosummary_generate = True

templates_path = ['_templates']
exclude_patterns = ['_build', 'Thumbs.db', '.DS_Store']

# -- Mock imports for unavailable C-extension dependencies -------------------
# cpl (PyCPL) and other ESO packages require compiled C extensions that are
# not available in a plain docs-build environment.  We mock them so that
# autodoc can still import pymetis modules and extract docstrings.
autodoc_mock_imports = [
    'cpl',
    'cpl.core',
    'cpl.ui',
    'cpl.dfs',
    'edps',
    'pyesorex',
    'adari_core',
]

# -- Autodoc configuration ---------------------------------------------------
autodoc_default_options = {
    'members': True,
    'undoc-members': True,
    'show-inheritance': True,
    'member-order': 'bysource',
}
autodoc_typehints = 'description'
autodoc_class_signature = 'separated'

# -- Napoleon (docstring style) ----------------------------------------------
napoleon_google_docstring = False
napoleon_numpy_docstring = True
napoleon_include_init_with_doc = True
napoleon_use_param = True
napoleon_use_rtype = True

# -- Intersphinx mapping -----------------------------------------------------
intersphinx_mapping = {
    'python': ('https://docs.python.org/3', None),
}

# -- Todo extension ----------------------------------------------------------
todo_include_todos = True

# -- HTML output configuration -----------------------------------------------
html_theme = 'sphinx_rtd_theme'
html_theme_options = {
    'navigation_depth': 4,
    'titles_only': False,
    'logo_only': False,
}
html_static_path = ['_static']
html_show_sourcelink = True

# -- autorecipe extension configuration -------------------------------------
# Root package to search for Recipe and DataItem subclasses
autorecipe_root_module = 'pymetis.instruments.metis'
# Where to write the DRLD JSON manifests during the build
autorecipe_manifest_dir = os.path.join('_build', 'drld_manifest')
