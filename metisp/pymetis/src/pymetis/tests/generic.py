import inspect
from abc import ABC
import subprocess

import cpl


from pymetis.base import MetisRecipe
from pymetis.inputs import PipelineInputSet



class BaseInputTest(ABC):
    impl = None
    count = None

    def test_is_input(self):
        assert issubclass(self.impl.InputSet, PipelineInputSet)

    def test_is_concrete(self):
        assert not inspect.isabstract(self.impl.InputSet)

    def test_can_load(self, load_frameset, sof):
        instance = self.impl.InputSet(load_frameset(sof))
        assert instance.verify() is None
        assert len(instance.raw.frameset) == self.count


class BaseRecipeTest(ABC):
    _recipe = None

    @staticmethod
    def run_with_pyesorex(name, sof):
        output = subprocess.run(['pyesorex', name, sof,
                                 '--recipe-dir', 'metisp/pyrecipes/',
                                 '--log-level', 'DEBUG'],
                                capture_output=True)
        return output.stdout.decode('utf-8').split('\n')[-3]

    def test_create(self):
        recipe = self._recipe()
        assert isinstance(recipe, cpl.ui.PyRecipe)


    def test_can_be_run_directly(self, load_frameset, sof):
        instance = self._recipe()
        frameset = cpl.ui.FrameSet(load_frameset(sof))
        instance.run(frameset, {})


    def test_can_be_run_with_pyesorex(self, name, create_pyesorex):
        pyesorex = create_pyesorex(self._recipe)
        assert isinstance(pyesorex.recipe, cpl.ui.PyRecipe)
        assert pyesorex.recipe.name == name
