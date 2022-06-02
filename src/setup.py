from setuptools import setup
from Cython.Build import cythonize

setup(ext_modules=cythonize('./src/pycheckers.pyx'), options={'build_ext':{'build_lib':'./src'}})