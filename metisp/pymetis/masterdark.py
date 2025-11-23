#!/usr/bin/env python

import cpl
import argparse

from pymetis.recipes import MetisDetDark


def main():
    parser = argparse.ArgumentParser(description='Pyesorexless recipe runner')
    parser.add_argument('sof', type=argparse.FileType('r'))
    args = parser.parse_args()

    frameset = cpl.ui.FrameSet(args.sof.name)
    MetisDetDark().run(frameset, {})


main()