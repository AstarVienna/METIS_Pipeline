import os


class TestConfigured:
    """
    These are actually meta-tests: if they fail, probably your environment is not set up correctly.
    Everything here is designed ad hoc currently.
    """
    def test_pythonpath(self):
        """
            If this fails, your Python is unable to find the package.
            Set PYTHONPATH to "." in the root directory.
            This is a temporary workaround and should be solved properly instead.
        """
        from prototypes.recipes.metis_det_dark import MetisDetDarkImpl
        assert MetisDetDarkImpl.__name__ == 'MetisDetDarkImpl', "Could not import, review your PYTHONPATH"

    def test_sof_data(self):
        """
            You need to point `pyesorex` to the SOF data (setting the `SOF_DATA` environment variable),
            by default to ../METIS_Simulations/ESO/output/.
        """
        assert os.environ['SOF_DATA'] is not None
