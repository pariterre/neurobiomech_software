from setuptools import setup, find_packages

from stimwalker import __version__ as stimwalker_version
from setuptools import setup

with open("README.md", "r") as fh:
    long_description = fh.read()

setup(
    name="stimwalker",
    version=stimwalker_version,
    author="Pariterre",
    author_email="pariterre@hotmail.com",
    description="Helps physical rehab using the functional electric stimulation using the Lokomat",
    long_description=long_description,
    url="https://github.com/cr-crme/stimwalker",
    packages=find_packages(),
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
    python_requires=">=3.12",
    zip_safe=False,
)
