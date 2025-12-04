#!/usr/bin/env python

import cpl
import argparse
import inspect

from cpl.core import Msg

from metisp.pyrecipes import metis_recipes
from pymetis.classes.recipes import MetisRecipe


def main():
    parser = argparse.ArgumentParser(description='Pyesorexless recipe runner')
    parser.add_argument('recipe', metavar='recipe', type=str)
    parser.add_argument('sof', type=argparse.FileType('r'))
    parser.add_argument('--debug', action='store_true')
    args = parser.parse_args()

    recipes = {
        recipe._name: recipe
        for name, recipe in inspect.getmembers(metis_recipes)
        if inspect.isclass(recipe) and issubclass(recipe, MetisRecipe)
    }

    if args.debug:
        Msg.set_config(level=Msg.SeverityLevel.DEBUG)

    frameset = cpl.ui.FrameSet(args.sof.name)
    recipe = recipes[args.recipe]()
    recipe.run(frameset, {})


main()