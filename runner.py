#!/usr/bin/env python

import cpl
import argparse
import inspect

from cpl.core import Msg

from metisp.pyrecipes import metis_recipes
from pymetis.engine.recipes import Recipe


def main():
    parser = argparse.ArgumentParser(
        description='Pyesorex-less recipe runner\n\n'
                    'Should be useful for testing recipes under development.\n'
                    'It might be good to generate your files with EDPS first, then run\n\n'
                    './runner.py recipe EDPS_data/<instrument>/<recipe>/<uuid>/input.sof',
        formatter_class=argparse.RawTextHelpFormatter,
    )
    parser.add_argument('recipe', metavar='recipe', type=str,
                        help='recipe to run')
    parser.add_argument('sof', type=argparse.FileType('r'),
                        help='set-of-frames file to process')
    parser.add_argument('--debug', action='store_true',
                        help='enable debug mode (sets CPL Msg level to DEBUG, '
                             'similar to `pyesorex --msg-level DEBUG`)')
    args = parser.parse_args()

    recipes = {
        recipe._name: recipe
        for name, recipe in inspect.getmembers(metis_recipes)
        if inspect.isclass(recipe) and issubclass(recipe, Recipe)
    }

    if args.debug:
        Msg.set_config(level=Msg.SeverityLevel.DEBUG)

    frameset = cpl.ui.FrameSet(args.sof.name)
    recipe = recipes[args.recipe]()
    recipe.run(frameset, {})


main()
