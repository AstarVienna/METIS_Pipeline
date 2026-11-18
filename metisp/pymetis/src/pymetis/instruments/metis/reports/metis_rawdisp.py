"""ADARI report showing the first raw frame of each input category."""

from .metis_utils import MetisDummyReport

rep = MetisDummyReport("metis_rawdisp", raw=True, one_per_catg=True)
