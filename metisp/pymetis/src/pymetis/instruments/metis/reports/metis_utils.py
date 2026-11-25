from adari_core.utils.utils import fetch_kw_or_default


class MetisReportMixin(object):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self._version = "TBD"


det_ifu_extensions = ["DET{:01d}".format(i) for i in range(1, 5)]
sci_ifu_extensions = ["DET{:01d}.SCI".format(i) for i in range(1, 5)]
data_ifu_extensions = ["DET{:01d}.DATA".format(i) for i in range(1, 5)]

det_img_extensions = ["DET{:01d}".format(i) for i in range(1, 2)]
sci_img_extensions = ["DET{:01d}.SCI".format(i) for i in range(1, 2)]
data_img_extensions = ["DET{:01d}.DATA".format(i) for i in range(1, 2)]


class MetisSetupInfo(object):
    @staticmethod
    def dark(hdul):
        metadata = [
            "DET.DIT: " + str(hdul["PRIMARY"].header.get("HIERARCH ESO DET DIT", "N/A")),
            "DET.NDIT: " + str(hdul["PRIMARY"].header.get("HIERARCH ESO DET NDIT", "N/A")),
        ]
        return metadata

    @staticmethod
    def detmon(hdul):
        metadata = [
            "INS3.SPXW.NAME: "
            + str(
                fetch_kw_or_default(
                    hdul["PRIMARY"], "HIERARCH ESO INS3 SPXW NAME", default="N/A"
                )
            ),
            "INS3.SPGW.NAME: "
            + str(
                fetch_kw_or_default(
                    hdul["PRIMARY"], "HIERARCH ESO INS3 SPGW NAME", default="N/A"
                )
            ),
        ]
        return metadata

    @staticmethod
    def lingain(hdul):
        """Return relevant header keywords for the lingain product."""
        metadata = [
            "INS.MODE: "
            + str(fetch_kw_or_default(hdul["PRIMARY"], "HIERARCH ESO INS MODE", default="N/A")),
            "DPR.TECH: "
            + str(fetch_kw_or_default(hdul["PRIMARY"], "HIERARCH ESO DPR TECH", default="N/A")),
            "QC.LINGAIN.MEAN: "
            + str(fetch_kw_or_default(hdul["PRIMARY"], "HIERARCH ESO QC LINGAIN MEAN", default="N/A")),
            "QC.LINGAIN.RMS: "
            + str(fetch_kw_or_default(hdul["PRIMARY"], "HIERARCH ESO QC LINGAIN RMS", default="N/A")),
            "QC.LIN.NUMBADPIX: "
            + str(fetch_kw_or_default(hdul["PRIMARY"], "HIERARCH ESO QC LIN NUMBADPIX", default="N/A")),
        ]
        return metadata
