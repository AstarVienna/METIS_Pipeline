pymetis — METIS Pipeline Documentation
========================================

**pymetis** is the Python data reduction pipeline framework for the
`METIS instrument <https://metis.strw.leidenuniv.nl/>`_ at ESO's Extremely Large Telescope (ELT).

This documentation is aimed at two audiences:

* **Pipeline developers** — newcomers to the team who need to understand the framework
  architecture and learn how to extend it by adding new recipes and algorithms.

* **AIT team members** — assembly, integration, and test engineers who want to use
  specific pipeline recipes to reduce data collected during instrument testing, and who
  need to understand the data-format contracts those recipes impose.

.. note::

   The :ref:`reference section <recipe-reference>` mirrors the structure of the DRLD
   (Data Reduction Library Design document).  Every recipe page is generated directly
   from the Python source code, so the two documents stay in sync automatically.

.. toctree::
   :maxdepth: 2
   :caption: Getting Started

   getting_started/index

.. toctree::
   :maxdepth: 2
   :caption: Concepts

   concepts/index

.. toctree::
   :maxdepth: 2
   :caption: Data Formats

   data_formats/index

.. toctree::
   :maxdepth: 2
   :caption: Recipe Reference
   :name: recipe-reference

   reference/index

.. toctree::
   :maxdepth: 2
   :caption: How-To Guides

   how_to/index

.. toctree::
   :maxdepth: 2
   :caption: API Reference

   api/index


Indices and tables
------------------

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`
