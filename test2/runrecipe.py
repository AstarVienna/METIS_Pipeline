from pyesorex.pyesorex import Pyesorex
import os
from pathlib import Path

p = Pyesorex()
#print(p.get_params_text())

p.parameters.update({"recipe_dirs":str(Path(os.getcwd(), "pyrecipes"))})

print(p.get_params_text())

print(p.get_recipes_text())

#p.recipe = "hellouser"
p.recipe = "basic_science"
p.sof_location = "test.sof"
result = p.run()
