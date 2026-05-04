QC Parameters
=============

Quality control (QC) parameters are scalar values computed during recipe
execution and written to the primary FITS header of each output product under
``ESO QC`` keywords.

Declaring QC parameters
------------------------

QC parameters are declared as attributes of the nested ``Qc`` class inside a
``RecipeImpl``:

.. code-block:: python

    class Qc(QcParameterSet):
        DarkMean   = DarkMean
        DarkMedian = DarkMedian
        DarkRms    = DarkRms
        DarkNBadpix = DarkNBadpix

Each attribute value is a :class:`~pymetis.engine.qc.QcParameter` subclass
(not an instance).  The framework writes the QC values to the product header
after ``process()`` returns.

Defining a QC parameter class
-------------------------------

QC parameter classes inherit from :class:`~pymetis.engine.qc.QcParameter` and
live in ``pymetis.instruments.metis.qc``:

.. code-block:: python

    class DarkMean(QcParameter):
        _name_template        = 'QC DARK MEAN'
        _description_template = 'Mean level of combined dark frame'
        _unit                 = 'Counts'
        _dtype                = float

Setting values in ``process()``
---------------------------------

QC parameter values are computed inside ``process()`` and stored on the
``RecipeImpl`` instance.  The framework collects them after ``process()``
completes::

    # Inside process():
    mean_value = float(combined_image.get_mean())
    self.qc.DarkMean.value = mean_value

How QC values appear in the product
-------------------------------------

Each QC parameter is written to the product's primary FITS header as an
``ESO QC`` keyword:

.. code-block:: text

    ESO QC DARK MEAN  =          1234.5 / Mean level of combined dark frame

The keyword name is taken from ``QcParameter._name_template`` (with any tag
placeholders filled).

DRLD traceability
-----------------

The ``autorecipe`` Sphinx extension extracts all ``Qc`` class members and
includes them in both the HTML reference page and the JSON DRLD manifest.
The manifest contains, for each QC parameter: name, unit, dtype, and
description — matching the fields of a DRLD QC parameter table entry.
