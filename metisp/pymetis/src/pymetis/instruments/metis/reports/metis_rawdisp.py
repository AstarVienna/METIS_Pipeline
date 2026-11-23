import os
import logging
from adari_core.data_libs.master_rawdisp import MasterRawdispReport
from .metis_utils import MetisSetupInfo, MetisReportMixin
from .metis_utils import data_ifu_extensions

logger = logging.getLogger(__name__)


class MetisRawdispReport(MetisReportMixin, MasterRawdispReport):
    def __init__(self):
        super().__init__("metis_rawdisp")
        self.extensions = []
        self.raw_extensions = {}
        ## The content of this dictionary is always: {"SOF_TAG" : "recipe"}
        self.tasks = {
            "DARK_IFU_RAW": "dark",
            #
        }
        # For each keyword recipe in self.tasks, select a number of raw frames:
        # DARK: select the first frame
        self.select_raw_files = {
            "DARK_IFU_RAW": [1],
        }
        self.ifu = False
        self.setup_info = MetisSetupInfo

    def parse_sof(self):
        tags = list(self.tasks.keys())
        raw_count = dict.fromkeys(tags, 0)

        file_lists = []
        for filename, catg in self.inputs:
            if catg in tags:
                if filename is not None:
                    raw_count[catg] += 1
                    if raw_count[catg] in self.select_raw_files[catg]:
                        logger.debug(catg, raw_count[catg], self.select_raw_files[catg])
                        file_lists.append({"filename": filename})
                        self.sof_tag.append(catg)
                        if catg in ["DARK_IFU_RAW"]:
                            self.ifu = True
        return file_lists

    def get_extensions(self):
        """Find the data extensions required for each METIS files.

        Description
        -----------
        After the SOF has been parsed, this method iterates over the different
        HDUS files to find which extension(s) contains the data.
        """
        new_hdus_list = []
        new_sof_tag = []
        if self.ifu:
            self.raw_extensions = {c: c for c in data_ifu_extensions}
        for i, filedict in enumerate(self.hdus):
            hdul = filedict["filename"]
            all_hdu_names = [hdu.name for hdu in hdul]

            for chip_name in self.raw_extensions.keys():
                if chip_name in all_hdu_names:
                    new_hdus_list.append(filedict)
                    new_sof_tag.append(self.sof_tag[i])
                    self.extensions.append(chip_name)

        self.hdus = new_hdus_list
        self.sof_tag = new_sof_tag

    def generate_panels(self, **kwargs):
        panels = {}
        self.get_extensions()
        new_panels = super().generate_panels(ext=self.extensions, **kwargs)
        for i, (panel, panel_descr) in enumerate(new_panels.items()):
            try:
                task_name = panel_descr["task_name"]
            except KeyError:
                raise RuntimeError(
                    "A report has been created by MasterRawdispReport that did not come back with a task name attached!"
                )
            panel_descr["report_name"] = "metis_rawdisp_{}_{}_{}_{}".format(
                task_name,
                self.raw_extensions[self.extensions[i]],
                self.sof_tag[i].lower(),
                os.path.basename(panel_descr["filename"]),
            )
            panel_descr["report_description"] = (
                f"METIS rawdisp panel - "
                f"{panel_descr['task_name']}, "
                f"{panel_descr['tag']}, "
                f"{os.path.basename(panel_descr['filename'])}, "
                f"{panel_descr['ext']}"
            )

        panels = {**panels, **new_panels}

        return panels


rep = MetisRawdispReport()
