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
        from pymetis.recipes.metis_det_dark import MetisDetDarkImpl
        assert MetisDetDarkImpl.__name__ == 'MetisDetDarkImpl', "Could not import, review your PYTHONPATH"

    def test_input(self):
        from pymetis.inputs import RawInput
        assert RawInput.__name__ == 'RawInput', "Could not import, review your PYTHONPATH"

    def test_sof_data(self):
        """
            You need to point `pyesorex` to the SOF data (setting the `SOF_DATA` environment variable),
            by default to ../METIS_Simulations/ESO/output/. Maybe this can be set for every run though?
        """
        assert os.environ['SOF_DATA'] is not None, "You need to define `SOF_DATA` environment variable"

    def test_recipe_dir(self):
        assert os.environ['PYESOREX_PLUGIN_DIR'] is not None
