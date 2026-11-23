class MetisReportMixin(object):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self._version = "TBD"


det_ifu_extensions = ["DET{:01d}".format(i) for i in range(1, 5)]
sci_ifu_extensions = ["DET{:01d}.SCI".format(i) for i in range(1, 5)]
data_ifu_extensions = ["DET{:01d}.DATA".format(i) for i in range(1, 5)]


class MetisSetupInfo(object):
    @staticmethod
    def dark(hdul):
        metadata = [
            "DET.DIT: " + str(hdul["PRIMARY"].header.get("HIERARCH ESO DET DIT", "N/A")),
            "DET.NDIT: " + str(hdul["PRIMARY"].header.get("HIERARCH ESO DET NDIT", "N/A")),
        ]
        return metadata
