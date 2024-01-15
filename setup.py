from lokomat_fes import __version__ as lokomat_fes_version
from setuptools import setup

with open("README.md", "r") as fh:
    long_description = fh.read()

setup(
    name="lokomat_fes",
    version=lokomat_fes_version,
    author="Pariterre",
    author_email="pariterre@hotmail.com",
    description="Helps physical rehab using the functional electric stimulation using the Lokomat",
    long_description=long_description,
    url="https://github.com/cr-crme/lokomat_fes",
    packages=[
        ".",
        "lokomat_fes",
        "lokomat_fes/misc",
        "lokomat_fes/nidaq",
        "examples",
    ],
    license="LICENSE",
    keywords=[
        "Functional electric stimulation",
        "Lokomat",
        "Physical rehab",
        "biomechanics",
    ],
    classifiers=[
        "Programming Language :: Python :: 3",
        "License :: OSI Approved :: MIT License",
    ],
    include_package_data=True,
    python_requires=">=3.11",
    zip_safe=False,
)
