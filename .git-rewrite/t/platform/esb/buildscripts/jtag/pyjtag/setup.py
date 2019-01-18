# setup.py
from distutils.core import setup
import glob
import py2exe

setup(
    name="msp430-jtag",
    scripts=["jtag.py"],
    )