from setuptools import setup, Extension
import pybind11

cpp_args = ["-std=c++14", "-stdlib=libc++", "-mmacosx-version-min=10.7"]

skin_plus_plus_module = Extension(
    "SkinPlusPlusPy",
    sources=["SkinPlusPlusPy.cpp"],
    include_dirs=[pybind11.get_include()],
    language="c++",
    extra_compile_args=cpp_args,
    )

setup(
    name="SkinPlusPlusPy",
    version="1.0",
    description="Python package for getting and setting skin weights in 3DsMax",
    ext_modules=[skin_plus_plus_module],
)