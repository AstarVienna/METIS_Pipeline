"""
ADARI detector monitoring report for the METIS metis_det_lingain recipe.

Analogous to eris_detmon: shows the raw ramp frames together with the
reduced calibration products (linearity coefficients, gain, bad-pixel map)
so that the quality of the detector characterisation can be assessed
in one place.

SOF expected contents
---------------------
DETLIN_{det}_RAW   : one or more raw ramp frames (multiple DITs)
LINEARITY_{det}    : linearity polynomial coefficient cube  (DET{n}.SCI cube)
GAIN_MAP_{det}     : bulk gain table                        (DET{n}.SCI table)
BADPIX_MAP_{det}   : bad-pixel map image                    (DET{n}.SCI image)

Products are detector-tagged (e.g. LINEARITY_2RG, GAIN_MAP_IFU).
Raw frames carry the detector tag in their category and end with _RAW.
"""

import os
import numpy as np

from adari_core.data_libs.master_detmon import MasterDetmonReport
from adari_core.plots.combined import CombinedPlot
from adari_core.plots.cut import CutPlot
from adari_core.plots.histogram import HistogramPlot
from adari_core.plots.images import ImagePlot
from adari_core.plots.panel import Panel
from adari_core.plots.text import TextPlot
from adari_core.utils.utils import fetch_kw_or_default

from .metis_utils import MetisReportMixin, MetisSetupInfo, det_ifu_extensions


# ---------------------------------------------------------------------------
# helpers (shared with metis_det_lingain)
# ---------------------------------------------------------------------------

def _gain_text(gain_hdul, det_prefix):
    """Return a list of gain info strings for one detector."""
    ext = det_prefix + ".SCI"
    try:
        tbl = gain_hdul[ext].data
        gain_val = tbl["gain"][0]
        gain_err = tbl["gain_err"][0]
        return [
            f"GAIN [{det_prefix}]: {gain_val:.4f} e/ADU",
            f"GAIN ERR [{det_prefix}]: {gain_err:.4f} e/ADU",
        ]
    except Exception:
        return [f"GAIN [{det_prefix}]: N/A"]


def _qc_text(hdul):
    """Return a list of QC keyword strings from a primary header."""
    kw_map = {
        "QC.LINGAIN.MEAN":  "HIERARCH QC LIN GAIN MEAN",
        "QC.LINGAIN.RMS":   "HIERARCH ESO QC LIN GAIN RMS",
        "QC.LIN.NUMBADPIX": "HIERARCH ESO QC LIN NUM BADPIX",
        "QC.LIN.MINFLUX":   "HIERARCH ESO QC LIN MIN FLUX",
        "QC.LIN.MAXFLUX":   "HIERARCH ESO QC LIN MAX FLUX",
    }
    lines = []
    for label, kw in kw_map.items():
        lines.append(
            f"{label}: {fetch_kw_or_default(hdul['PRIMARY'], kw, default='N/A')}"
        )
    return lines


# ---------------------------------------------------------------------------
# report
# ---------------------------------------------------------------------------

class MetisDetMonReport(MetisReportMixin, MasterDetmonReport):
    """
    METIS detector monitoring report.

    Shows the full characterisation chain: raw ramp data → linearity
    coefficients, bulk gain, and bad-pixel map, in a layout analogous
    to eris_detmon.
    """

    # DIT keyword in METIS raw DETLIN frames
    _dit_key = "HIERARCH ESO DET DIT"

    def __init__(self):
        # files_needed is unused (parse_sof is overridden), but
        # MasterDetmonReport.__init__ requires the argument.
        super().__init__("metis_det_mon", files_needed={})
        self.ifu = False

    # ------------------------------------------------------------------ #
    # required abstract method from MasterDetmonReport
    # ------------------------------------------------------------------ #

    def access_port(self, im_hdu, port):
        """METIS detectors have a single readout port — return HDU as-is."""
        return im_hdu

    # ------------------------------------------------------------------ #
    # SOF parsing
    # ------------------------------------------------------------------ #

    def parse_sof(self):
        """
        Map SOF entries to role keys.

        Raw DETLIN frames are stored as on000, on001, … (using
        MasterDetmonReport.generate_file_key) so they can be sorted
        by DIT later.  The reduced products are stored under fixed keys.
        """
        raw_count    = 0
        linearity_map = None
        gain_map      = None
        badpix_map    = None
        entry         = {}

        for filename, catg in self.inputs:
            if catg.startswith("DETLIN") and catg.endswith("RAW"):
                entry[self.generate_file_key("on", raw_count)] = filename
                raw_count += 1
                if "IFU" in catg:
                    self.ifu = True
            elif catg.startswith("LINEARITY"):
                linearity_map = filename
                if "IFU" in catg:
                    self.ifu = True
            elif catg.startswith("GAIN_MAP"):
                gain_map = filename
            elif catg.startswith("BADPIX_MAP"):
                badpix_map = filename

        if linearity_map is not None:
            entry["linearity_map"] = linearity_map
        if gain_map is not None:
            entry["gain_map"] = gain_map
        if badpix_map is not None:
            entry["badpix_map"] = badpix_map

        return [entry] if entry else []

    # ------------------------------------------------------------------ #
    # panel generation
    # ------------------------------------------------------------------ #

    def generate_panels(self, **kwargs):
        panels = {}
        det_exts = det_ifu_extensions if self.ifu else ["DET1"]

        for hdu_dict in self.hdus:
            for det_prefix in det_exts:
                panels.update(self._panel_main(hdu_dict, det_prefix))
                if hdu_dict.get("badpix_map") is not None:
                    panels.update(self._panel_bpm(hdu_dict, det_prefix))

        return panels

    # ---- helpers ------------------------------------------------------ #

    def _longest_on_frame(self, hdu_dict):
        """Return the HDUList of the raw frame with the highest DIT."""
        on_frames = [v for k, v in hdu_dict.items() if k.startswith("on")]
        if not on_frames:
            return None
        return sorted(
            on_frames,
            key=lambda hdul: hdul[0].header.get(self._dit_key, -1.0),
            reverse=True,
        )[0]

    # ---- panel 1: raw frame + linearity -------------------------------- #

    def _panel_main(self, hdu_dict, det_prefix):
        """
        Main monitoring panel — analogous to ERIS panel 1.

        Layout (2 cols × 4 rows):
          row 0 : metadata text (file info left, QC keywords right)
          row 1 : longest-DIT raw frame image  (full width)
          row 2 : central X+Y cuts             | raw frame histogram
          row 3 : linearity coeff[0] image     | coeff[0] histogram
        """
        sci_ext = det_prefix + ".SCI"
        data_ext = det_prefix + ".DATA"
        vspace  = 0.3
        panels  = {}

        lin_hdul  = hdu_dict.get("linearity_map")
        gain_hdul = hdu_dict.get("gain_map")
        raw_hdul  = self._longest_on_frame(hdu_dict)

        if lin_hdul is None:
            return panels   # nothing to show without the linearity product

        try:
            lin_data = lin_hdul[sci_ext].data
        except KeyError:
            return panels

        fname_lin = os.path.basename(str(lin_hdul.filename()))
        pro_catg  = str(lin_hdul["PRIMARY"].header.get(
            "HIERARCH ESO PRO CATG", "LINEARITY"))

        # ---- text blocks ------------------------------------------------
        col1_lines = [
            str(lin_hdul["PRIMARY"].header.get("INSTRUME", "METIS")),
            f"EXTNAME: {sci_ext}",
            f"PRO CATG: {pro_catg}",
            f"FILE NAME: {fname_lin}",
            "RAW1 NAME: " + str(lin_hdul["PRIMARY"].header.get(
                "HIERARCH ESO PRO REC1 RAW1 NAME", "N/A")),
        ]
        if gain_hdul is not None:
            col1_lines += _gain_text(gain_hdul, det_prefix)

        col2_lines = _qc_text(lin_hdul)

        t1 = TextPlot(columns=1, v_space=vspace)
        t1.add_data(col1_lines)
        t2 = TextPlot(columns=1, v_space=vspace)
        t2.add_data(col2_lines)

        # ---- panel skeleton ---------------------------------------------
        p = Panel(2, 4, height_ratios=[1, 4, 4, 4])
        p.assign_plot(t1, 0, 0)
        p.assign_plot(t2, 1, 0)

        # ---- row 1: longest-DIT raw frame image -------------------------
        raw_im = None
        raw_data = None
        if raw_hdul is not None:
            try:
                print('hereeeee')
                print(raw_hdul.info())
                raw_data = raw_hdul[data_ext].data
                dit = raw_hdul[0].header.get(self._dit_key, "?")
                raw_im = ImagePlot(
                    raw_data,
                    title=f"Raw frame  DIT = {dit} s  –  {data_ext}",
                    v_clip="sigma",
                    v_clip_kwargs={"nsigma": 3},
                )
                p.assign_plot(raw_im, 0, 1, xext=2)
            except KeyError as e:
                print(e)
                raw_im = None

        # ---- row 2: cuts + histogram of the raw frame -------------------
        if raw_im is not None and raw_data is not None:
            bunit = fetch_kw_or_default(raw_hdul[data_ext], "BUNIT", default="ADU")

            cx = CutPlot("x", title="Cut X")
            cx.add_data(raw_im,
                        raw_im.get_data_coord(raw_data.shape[1] // 2, "x"),
                        label="col", color="blue")
            cy = CutPlot("y", title="Cut Y")
            cy.add_data(raw_im,
                        raw_im.get_data_coord(raw_data.shape[0] // 2, "y"),
                        label="row", color="black")

            cuts = CombinedPlot(
                title="Central cuts in X & Y",
                x_label="pixel",
                y_label=bunit,
            )
            cuts.add_data(cy, z_order=0)
            cuts.add_data(cx, z_order=1)
            p.assign_plot(cuts, 0, 2)

            raw_hist = HistogramPlot(master_data=raw_data, bins=200,
                                     title="Raw frame histogram")
            p.assign_plot(raw_hist, 1, 2)

        # ---- row 3: linearity coeff[0] image + histogram ----------------
        coeff0 = lin_data[0] if lin_data.ndim == 3 else lin_data

        img_c0 = ImagePlot(
            coeff0,
            title=f"Linearity coeff[0] (slope)  –  {sci_ext}",
            v_clip="sigma",
            v_clip_kwargs={"nsigma": 3},
        )
        p.assign_plot(img_c0, 0, 3)

        hist_c0 = HistogramPlot(master_data=coeff0, bins=50,
                                 title="coeff[0] histogram")
        p.assign_plot(hist_c0, 1, 3)

        panels[p] = {
            "report_name":        f"metis_det_mon_{det_prefix.lower()}_main",
            "report_description": (
                f"METIS detector monitoring – {fname_lin}, {sci_ext}"
            ),
            "report_tags": [],
        }
        return panels

    # ---- panel 2: bad-pixel map ---------------------------------------- #

    def _panel_bpm(self, hdu_dict, det_prefix):
        """
        Bad-pixel-map panel — analogous to ERIS panel 2.

        Layout (2 cols × 2 rows):
          row 0 : metadata text
          row 1 : BPM image (full width)
        """
        sci_ext  = det_prefix + ".SCI"
        vspace   = 0.3
        panels   = {}

        bpm_hdul  = hdu_dict["badpix_map"]
        gain_hdul = hdu_dict.get("gain_map")
        lin_hdul  = hdu_dict.get("linearity_map")

        try:
            bpm_data = bpm_hdul[sci_ext].data
        except KeyError:
            return panels

        fname_bpm = os.path.basename(str(bpm_hdul.filename()))
        pro_catg  = str(bpm_hdul["PRIMARY"].header.get(
            "HIERARCH ESO PRO CATG", "BADPIX_MAP"))

        col1_lines = [
            str(bpm_hdul["PRIMARY"].header.get("INSTRUME", "METIS")),
            f"EXTNAME: {sci_ext}",
            f"PRO CATG: {pro_catg}",
            f"FILE NAME: {fname_bpm}",
            "RAW1 NAME: " + str(bpm_hdul["PRIMARY"].header.get(
                "HIERARCH ESO PRO REC1 RAW1 NAME", "N/A")),
        ]
        if gain_hdul is not None:
            col1_lines += _gain_text(gain_hdul, det_prefix)

        col2_lines = _qc_text(lin_hdul) if lin_hdul is not None else []

        t1 = TextPlot(columns=1, v_space=vspace)
        t1.add_data(col1_lines)
        t2 = TextPlot(columns=1, v_space=vspace)
        t2.add_data(col2_lines)

        p = Panel(2, 2, height_ratios=[1, 4])
        p.assign_plot(t1, 0, 0)
        p.assign_plot(t2, 1, 0)

        img_bpm = ImagePlot(
            bpm_data,
            title=f"Bad Pixel Map  –  {sci_ext}",
            v_clip="val",
            v_clip_kwargs={"low": 0, "high": 1},
            interpolation="none",
        )
        p.assign_plot(img_bpm, 0, 1, xext=2)

        panels[p] = {
            "report_name":        f"metis_det_mon_{det_prefix.lower()}_bpm",
            "report_description": (
                f"METIS detector monitoring – BPM – {fname_bpm}, {sci_ext}"
            ),
            "report_tags": [],
        }
        return panels


rep = MetisDetMonReport()
