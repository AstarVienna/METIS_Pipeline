"""
ADARI distortion-solution report for the METIS metis_ifu_distortion recipe.
Analogous to eris_distortion: shows the distortion map for each IFU detector
together with a trace-model overlay from the distortion polynomial table.

SOF expected contents
---------------------
IFU_DIST_REDUCED     : distortion map image (DET{n}.DATA extensions, n = 1–4)
IFU_DISTORTION_TABLE : distortion polynomial table (DET{n} BINTABLEs, n = 1–4)
IFU_DISTORTION_RAW   : raw pinhole frame (optional fallback background)

Category matching uses startswith() so minor name variations are tolerated.
"""

import os

import numpy as np

from adari_core.report import AdariReportBase
from adari_core.plots.images import ImagePlot
from adari_core.plots.combined import CombinedPlot
from adari_core.plots.points import LinePlot
from adari_core.plots.panel import Panel
from adari_core.plots.text import TextPlot
from adari_core.utils.utils import fetch_kw_or_default

from .metis_utils import MetisReportMixin, det_ifu_extensions


# ---------------------------------------------------------------------------
# Helper: metadata text columns
# ---------------------------------------------------------------------------

def _dist_map_text(hdul, det_prefix):
    """Left-column metadata lines for a distortion-map HDU."""
    fname = os.path.basename(str(hdul.filename()))
    pro_catg = str(hdul["PRIMARY"].header.get("HIERARCH ESO PRO CATG", "IFU_DIST_REDUCED"))
    return [
        str(hdul["PRIMARY"].header.get("INSTRUME", "METIS")),
        f"DET: {det_prefix}",
        f"PRO CATG: {pro_catg}",
        f"FILE: {fname}",
        "RAW1: " + str(hdul["PRIMARY"].header.get(
            "HIERARCH ESO PRO REC1 RAW1 NAME", "N/A")),
    ]


def _dist_qc_text(hdul):
    """Right-column QC lines for a distortion-map HDU."""
    kw_map = {
        "QC.DIST.NMODEL":   "HIERARCH ESO QC DIST NMODEL",
        "QC.DIST.RESIDRMS": "HIERARCH ESO QC DIST RESIDRMS",
        "QC.DIST.MAXRESID": "HIERARCH ESO QC DIST MAXRESID",
        "QC.DIST.NREJ":     "HIERARCH ESO QC DIST NREJ",
    }
    return [
        f"{label}: {fetch_kw_or_default(hdul['PRIMARY'], kw, default='N/A')}"
        for label, kw in kw_map.items()
    ]


def _table_info_text(hdul, det_prefix):
    """Info lines drawn from the trace table PRIMARY header."""
    fname = os.path.basename(str(hdul.filename()))
    return [
        "PRO CATG: " + str(hdul["PRIMARY"].header.get(
            "HIERARCH ESO PRO CATG", "IFU_DISTORTION_TABLE")),
        "INS MODE: " + str(hdul["PRIMARY"].header.get(
            "HIERARCH ESO INS MODE", "N/A")),
        "NTRACES: " + str(hdul["PRIMARY"].header.get(
            "HIERARCH QC IFU DISTORT NTRACES", "N/A")),
        f"FILE: {fname}",
        f"DET: {det_prefix}",
    ]


# ---------------------------------------------------------------------------
# Helper: polynomial evaluation
# ---------------------------------------------------------------------------

def _eval_poly2(coeffs, x):
    """Evaluate  a[0] + a[1]*x + a[2]*x²  (coefficients ordered low-to-high)."""
    a0, a1, a2 = float(coeffs[0]), float(coeffs[1]), float(coeffs[2])
    return a0 + a1 * x + a2 * x ** 2


def _build_trace_lines(trace_rows):
    """
    Build concatenated (x, y) arrays for all trace centre and edge curves,
    with NaN breaks between traces so each trace is rendered as a separate
    line segment.

    Returns
    -------
    xy_cen   : (2, N) float array – centre curves (x, y in FITS 1-based coords)
    xy_edges : (2, M) float array – top + bottom edge curves
    n_traces : int
    """
    nan = np.array([np.nan])
    x_cen,   y_cen   = [], []
    x_edges, y_edges = [], []

    for row in trace_rows:
        col_start = float(row["column_range"][0])
        col_end   = float(row["column_range"][1])
        x_fits = np.linspace(col_start, col_end, 300)

        x_cen.extend([x_fits, nan])
        y_cen.extend([_eval_poly2(row["pos"], x_fits), nan])

        # top and bottom edges share the same x sampling; separate them with NaN
        x_edges.extend([x_fits, nan, x_fits, nan])
        y_edges.extend([
            _eval_poly2(row["bottom"], x_fits), nan,
            _eval_poly2(row["top"],    x_fits), nan,
        ])

    xy_cen   = np.array([np.concatenate(x_cen),   np.concatenate(y_cen)])
    xy_edges = np.array([np.concatenate(x_edges), np.concatenate(y_edges)])
    return xy_cen, xy_edges, len(trace_rows)


# ---------------------------------------------------------------------------
# Report class
# ---------------------------------------------------------------------------

class MetisIfuDistortionReport(MetisReportMixin, AdariReportBase):
    """
    Two panel types per IFU detector (DET1–DET4).

    Map panel  (requires IFU_DIST_REDUCED in the SOF)
    --------------------------------------------------
    2 cols × 3 rows, height_ratios=[1, 4, 4]
      row 0 : file / detector metadata  |  QC keywords
      rows 1–2 (full-width): distortion map image

    Trace overlay panel  (requires IFU_DISTORTION_TABLE in the SOF)
    ----------------------------------------------------------------
    1 col × 2 rows, height_ratios=[1, 8]
      row 0 : table metadata text
      row 1 : CombinedPlot — ImagePlot (background) with LinePlot trace
              curves (centre solid, edges dashed) overlaid
    """

    def __init__(self):
        super().__init__("metis_ifu_distortion")

    # ------------------------------------------------------------------
    # SOF parsing
    # ------------------------------------------------------------------

    def parse_sof(self):
        dist_map   = None
        dist_table = None
        dist_raw   = None
        for filename, catg in self.inputs:
            if catg.startswith("IFU_DIST_REDUCED"):
                dist_map = filename
            elif catg.startswith("IFU_DISTORTION_TABLE"):
                dist_table = filename
            elif catg.startswith("IFU_DISTORTION_RAW"):
                if dist_raw is None:            # keep only the first raw frame
                    dist_raw = filename

        entry = {}
        if dist_map   is not None: entry["dist_map"]   = dist_map
        if dist_table is not None: entry["dist_table"] = dist_table
        if dist_raw   is not None: entry["dist_raw"]   = dist_raw

        return [entry] if entry else []

    # ------------------------------------------------------------------
    # Individual panel builders
    # ------------------------------------------------------------------

    def _map_panel(self, dist_hdul, det_prefix):
        """Standard distortion-map panel: image + two text columns."""
        data_ext = det_prefix + ".DATA"
        try:
            dist_data = dist_hdul[data_ext].data
        except KeyError:
            return {}

        vspace = 0.3
        t1 = TextPlot(columns=1, v_space=vspace)
        t1.add_data(_dist_map_text(dist_hdul, det_prefix))
        t2 = TextPlot(columns=1, v_space=vspace)
        t2.add_data(_dist_qc_text(dist_hdul))

        img = ImagePlot(
            dist_data,
            title=f"Distortion map  –  {det_prefix}.DATA",
            v_clip="sigma",
            v_clip_kwargs={"nsigma": 3},
        )

        p = Panel(2, 3, height_ratios=[1, 4, 4])
        p.assign_plot(t1, 0, 0)
        p.assign_plot(t2, 1, 0)
        p.assign_plot(img, 0, 1, xext=2, yext=2)

        fname = os.path.basename(str(dist_hdul.filename()))
        return {p: {
            "report_name": f"metis_ifu_distortion_{det_prefix.lower()}",
            "report_description": (
                f"METIS IFU distortion map – {fname}, {det_prefix}.DATA"
            ),
            "report_tags": [],
        }}

    def _trace_overlay_panel(self, hdu_dict, det_prefix):
        """
        Panel: distortion-map background with trace model curves overlaid.

        Uses a CombinedPlot: an ImagePlot (background) combined with two
        LinePlots (trace centres in red solid, trace edges in white dashed).
        All coordinates are in FITS 1-based pixel units, consistent with the
        ImagePlot default origin of (1, 1).
        """
        table_hdul = hdu_dict.get("dist_table")
        if table_hdul is None:
            return {}

        # Trace table for this detector (bare extension name, e.g. "DET1")
        try:
            trace_rows = table_hdul[det_prefix].data
        except KeyError:
            return {}

        # Determine background image source
        bg_hdul = hdu_dict.get("dist_map") or hdu_dict.get("dist_raw")

        bg_data = None
        if bg_hdul is not None:
            # Try ".DATA" first (reduced map), then ".SCI" (raw frame)
            for ext_try in (det_prefix + ".DATA", det_prefix + ".SCI"):
                try:
                    bg_data = bg_hdul[ext_try].data.astype(float)
                    break
                except KeyError:
                    continue

        if bg_data is None:
            # No background available: synthesise a blank canvas sized from
            # the trace column ranges and polynomial extents.
            max_col = int(max(float(r["column_range"][1]) for r in trace_rows)) + 10
            max_row = int(max(
                float(_eval_poly2(r["top"], float(r["column_range"][1])))
                for r in trace_rows
            )) + 10
            bg_data = np.zeros((max(max_row, 100), max(max_col, 100)), dtype=float)

        # Build concatenated trace data with NaN gaps between traces
        xy_cen, xy_edges, n_traces = _build_trace_lines(trace_rows)

        # Background image (z_order=0)
        img = ImagePlot(
            bg_data,
            title=(
                f"Distortion map + trace model  –  {det_prefix}.DATA  "
                f"({n_traces} traces)"
            ),
            v_clip="sigma",
            v_clip_kwargs={"nsigma": 3},
        )

        # Trace centre lines (z_order=1, solid red)
        line_cen = LinePlot()
        line_cen.add_data(xy_cen, label="trace_centres", color="b")

        # Trace edge lines (z_order=2, dashed white)
        line_edges = LinePlot(linestyle="dashed")
        line_edges.add_data(xy_edges, label="trace_edges", color="c")

        # Combine into a single plot object
        combined = CombinedPlot()
        combined.add_data(img,        z_order=0)
        combined.add_data(line_cen,   z_order=1)
        combined.add_data(line_edges, z_order=2)

        # Metadata text block (table header info)
        t = TextPlot(columns=1, v_space=0.3)
        t.add_data(_table_info_text(table_hdul, det_prefix))

        p = Panel(2, 3, height_ratios=[1, 4, 4])
        p.assign_plot(t,        0, 0, xext=2)
        p.assign_plot(combined, 0, 1, xext=2, yext=2)

        tbl_fname = os.path.basename(str(table_hdul.filename()))
        return {p: {
            "report_name": f"metis_ifu_distortion_{det_prefix.lower()}_traces",
            "report_description": (
                f"METIS IFU trace model overlay – {tbl_fname}, {det_prefix}"
            ),
            "report_tags": [],
        }}

    # ------------------------------------------------------------------
    # Main entry point
    # ------------------------------------------------------------------

    def generate_panels(self, **kwargs):
        panels = {}
        for hdu_dict in self.hdus:
            dist_hdul = hdu_dict.get("dist_map")
            for det_prefix in det_ifu_extensions:
                # Standard map panel (only when the reduced map product is in the SOF)
                if dist_hdul is not None:
                    panels.update(self._map_panel(dist_hdul, det_prefix))
                # Trace overlay panel (whenever the polynomial table is in the SOF)
                panels.update(self._trace_overlay_panel(hdu_dict, det_prefix))
        return panels


rep = MetisIfuDistortionReport()
