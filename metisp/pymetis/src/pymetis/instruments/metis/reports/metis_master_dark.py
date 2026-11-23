from adari_core.data_libs.master_dark_bias import MasterDarkBiasReport
from adari_core.plots.images import ImagePlot
from adari_core.plots.panel import Panel
from adari_core.plots.text import TextPlot
from .metis_utils import MetisSetupInfo, MetisReportMixin
from .metis_utils import det_ifu_extensions, sci_ifu_extensions
import os


class MetisMasterDarkReport(MetisReportMixin, MasterDarkBiasReport):
    image_category = "master_im"
    panel_kwargs = dict(x=4, y=2, height_ratios=[1, 5])

    def __init__(self):
        super().__init__("metis_master_dark")
        self.data_extensions = None
        self.det_extensions = None
        self.ifu = False

    def parse_sof(self):
        master_dark = None
        for filename, catg in self.inputs:
            if catg == "MASTER_DARK_IFU":
                master_dark = filename
                self.ifu = True
        file_lists = []
        if master_dark is not None:
            file_lists.append(
                {
                    "master_im": master_dark,
                }
            )
        if self.ifu:
            self.data_extensions = sci_ifu_extensions
            self.det_extensions = det_ifu_extensions
        return file_lists

    def generate_panels(self, **kwargs):
        panels = {}
        vspace = 0.3

        extra_panels = {}

        # Generate individual raw-cuts
        for extdet in self.det_extensions:
            ext = extdet + ".SCI"
            new_panels = super().generate_raw_cuts_panels(
                master_im_ext=ext,
            )
            for i, (panel, panel_descr) in enumerate(new_panels.items()):
                panel_descr["report_description"] = (
                    f"METIS dark panel - {os.path.basename(panel_descr['master_im'])}, {panel_descr['master_im_ext']}"
                )
                master_im = self.hdus[i]["master_im"]

                # Text Plot
                vspace = 0.3
                fname = os.path.basename(str(master_im.filename()))
                t1 = TextPlot(columns=1, v_space=vspace)
                col1 = (
                    str(master_im["PRIMARY"].header.get("INSTRUME")),
                    "EXTNAME: " + str(master_im[ext].header.get("EXTNAME", "N/A")),
                    "PRO CATG: " + str(master_im["PRIMARY"].header.get("HIERARCH ESO PRO CATG")),
                    "FILE NAME: " + fname,
                    "RAW1 NAME: " + str(master_im["PRIMARY"].header.get("HIERARCH ESO PRO REC1 RAW1 NAME")),
                )
                t1.add_data(col1)
                panel.assign_plot(t1, 0, 0, xext=2)

                t2 = TextPlot(columns=1, v_space=vspace, xext=1)
                self.metadata = MetisSetupInfo.dark(master_im)
                col2 = self.metadata
                t2.add_data(col2)
                panel.assign_plot(t2, 2, 0, xext=1)

                # Three panel plot with data, err, dq
                pextra = Panel(2, 3, height_ratios=[1, 4, 4])
                pextra.assign_plot(t1, 0, 0, xext=1)
                pextra.assign_plot(t2, 1, 0, xext=1)

                plot_data = ImagePlot(
                    master_im[ext].data,
                    title=ext,
                    v_clip="percentile",
                    v_clip_kwargs={"percentile": 50.0},
                )
                pextra.assign_plot(plot_data, 0, 1)
                plot_data = ImagePlot(
                    master_im[extdet + ".ERR"].data,
                    title=extdet + ".ERR",
                    v_clip="percentile",
                    v_clip_kwargs={"percentile": 50.0},
                )
                pextra.assign_plot(plot_data, 1, 1)
                plot_data = ImagePlot(
                    master_im[extdet + ".DQ"].data,
                    title=extdet + ".DQ",
                    v_clip="val",
                    v_clip_kwargs={"low": 0, "high": 6},
                    interpolation="none",
                )
                pextra.assign_plot(plot_data, 0, 2)
                addme = {}
                addme["report_name"] = panel_descr["report_name"] + "_extra"
                addme["report_description"] = (
                    f"METIS dark additional panel - "
                    f"{os.path.basename(panel_descr['master_im'])}, "
                    f"{panel_descr['master_im_ext']}"
                )
                addme["report_tags"] = []
                addme["input_files"] = [master_im.filename()]
                extra_panels[pextra] = addme

            panels = {**panels, **new_panels, **extra_panels}

        # Generate the multi-extension panels
        img_kw_list = [
            {
                "title": det,
                # "v_clip": self.im_clipping,
                # "v_clip_kwargs": self.im_clipping_kwargs,
            }
            for det in self.det_extensions
        ]

        new_panels = self.generate_multi_extension_panels(img_kw_list=img_kw_list)
        for i, (panel, panel_descr) in enumerate(new_panels.items()):
            hdul = self.hdus[i][self.image_category]
            setup = str(hdul["PRIMARY"].header.get("HIERARCH ESO INS MODE", "N/A"))
            panel_descr["report_name"] = "{}_{}_multi".format(
                self.name,
                setup.lower(),
            )
            panel_descr["report_description"] = f"{self.name} Panel - {os.path.basename(hdul.filename())}, "

            self.metadata = MetisSetupInfo.dark(self.hdus[0]["master_im"])
            text = TextPlot(columns=1, v_space=0.3)
            text.add_data(self.metadata)
            panel.assign_plot(text, 2, 0, xext=1)

        panels = {**panels, **new_panels}

        return panels


rep = MetisMasterDarkReport()
