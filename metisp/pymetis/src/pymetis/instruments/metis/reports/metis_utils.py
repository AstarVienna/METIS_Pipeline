"""Shared helpers for METIS ADARI reports, incl. a generic placeholder report for recipes without a real one.

This module must not create report instances: ADARI picks the first AdariReportBase instance it finds in a module.
"""

import math
import os

import numpy as np
from astropy.io import fits

from adari_core.plots.images import ImagePlot
from adari_core.plots.panel import Panel
from adari_core.plots.text import TextPlot
from adari_core.report import AdariReportBase

REPORT_VERSION = "0.1"
MAX_IMAGES = 4  # image extensions shown per frame
MAX_QC = 8      # QC keywords listed per frame


class MetisReportMixin:
    """Sets the report version, which ADARI requires."""

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self._version = REPORT_VERSION


def image_extensions(hdul, limit=MAX_IMAGES):
    """(EXTNAME, 2D array) for the first *limit* image HDUs; cubes are reduced to their first plane."""
    images = []
    for i, hdu in enumerate(hdul):
        if not isinstance(hdu, (fits.PrimaryHDU, fits.ImageHDU)) or hdu.data is None or hdu.data.ndim < 2:
            continue
        data = hdu.data
        while data.ndim > 2:
            data = data[0]
        if not np.isfinite(data).any():
            continue
        images.append((hdu.name or f"HDU{i}", np.asarray(data, dtype=float)))
        if len(images) == limit:
            break
    return images


def header_lines(hdul, filename, catg):
    """Text block describing a frame: instrument, category, file, recipe and up to MAX_QC QC keywords."""
    hdr = hdul[0].header
    lines = [
        str(hdr.get("INSTRUME", "METIS")),
        f"CATG: {catg}",
        f"FILE NAME: {os.path.basename(filename)}",
    ]
    if "HIERARCH ESO PRO REC1 ID" in hdr:
        lines.append(f"RECIPE: {hdr['HIERARCH ESO PRO REC1 ID']}")
    else:
        dpr = [str(hdr.get(f"HIERARCH ESO DPR {k}", "")) for k in ("CATG", "TYPE", "TECH")]
        lines.append("DPR: " + ", ".join(dpr))
    qc = [(k, v) for k, v in hdr.items() if k.startswith("ESO QC ")][:MAX_QC]
    lines += [f"{k[4:]}: {v}" for k, v in qc]
    return lines


class MetisDummyReport(MetisReportMixin, AdariReportBase):
    """Generic placeholder report: one panel per selected frame with header info and its image extensions.

    Frames are selected by *catgs* if given, else raw frames (``raw=True``), else products whose
    PRO.REC1.ID equals *recipe*.
    """

    def __init__(self, name, recipe=None, catgs=None, raw=False, one_per_catg=False):
        super().__init__(name)
        self.recipe = recipe or name
        self.catgs = set(catgs) if catgs else None
        self.raw = raw
        self.one_per_catg = one_per_catg
        self.selected = []

    def _wanted(self, filename, catg):
        if self.catgs is not None:
            return catg in self.catgs
        try:
            hdr = fits.getheader(filename)
        except (OSError, ValueError):
            return False
        if self.raw:
            return "HIERARCH ESO PRO CATG" not in hdr
        return hdr.get("HIERARCH ESO PRO REC1 ID") == self.recipe

    def parse_sof(self):
        self.selected = []
        seen = set()
        for filename, catg in self.inputs:
            if self.one_per_catg and catg in seen:
                continue
            if self._wanted(filename, catg):
                seen.add(catg)
                self.selected.append((filename, catg))
        return [{"frame": filename} for filename, _ in self.selected]

    def generate_panels(self, **kwargs):
        panels = {}
        for i, (filename, catg) in enumerate(self.selected):
            hdul = self.hdus[i]["frame"]
            images = image_extensions(hdul)
            cols = 1 if len(images) == 1 else 2
            rows = math.ceil(len(images) / cols)

            lines = header_lines(hdul, filename, catg)
            if images:
                lines.append(f"Placeholder report '{self.name}'")
            else:
                lines.append(f"Placeholder report '{self.name}' - no 2D image data")
                lines += [f"EXT {hdu.name}: {getattr(hdu.data, 'shape', None)}" for hdu in hdul[1:MAX_QC + 1]]
            # v_space is in axes units: spread the lines over the text row and grow the row with them
            text = TextPlot(columns=1, v_space=1.0 / len(lines))
            text.add_data(lines)

            panel = Panel(cols, rows + 1, height_ratios=[max(1.0, 0.3 * len(lines))] + [4] * rows)
            panel.assign_plot(text, 0, 0, xext=cols)
            for j, (extname, data) in enumerate(images):
                plot = ImagePlot(data, title=extname, v_clip="percentile", v_clip_kwargs={"percentile": 95.0})
                panel.assign_plot(plot, j % cols, 1 + j // cols)

            panels[panel] = {
                "report_name": f"{self.name}_{catg.lower()}_{i}",
                "report_description": f"METIS {self.name} - {os.path.basename(filename)} ({catg})",
                "report_tags": [],
                "report_prodcatg": catg,
            }
        return panels


def make_dummy_report(name, recipe=None, catgs=None):
    """Placeholder report for recipe *recipe* (defaults to *name*); replace with a real report once one exists."""
    return MetisDummyReport(name, recipe=recipe, catgs=catgs)
