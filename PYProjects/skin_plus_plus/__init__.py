from __future__ import annotations

import importlib
import os
import pathlib
import sys

current_dcc = None

get_skin_data = None
set_skin_weights = None
skin_plus_plus_py = None
# get_vertex_positions = None

__py_version__ = f"{sys.version_info.major}{sys.version_info.minor}"


def __get_skin_plus_plus_py(python_version: str, debug: bool = False):
    global skin_plus_plus_py

    debug = bool(os.environ.get("SKIN_PLUS_PLUS_DEBUG", False)) or debug
    if debug:
        python_version = f"debug_{python_version}"

    current_directory = pathlib.Path(__file__).parent
    sub_module_path = current_directory / f"py/{python_version}"

    if not sub_module_path.exists():
        raise FileNotFoundError(f"Unsupported Python version!")

    import_path = f"skin_plus_plus.py.{python_version}.skin_plus_plus_py"
    print(f"import_path: {import_path}")
    if "skin_plus_plus_py" in sys.modules:
        del sys.modules["skin_plus_plus_py"]

    skin_plus_plus_py = importlib.import_module(import_path)
    return skin_plus_plus_py


def __get_dcc_backend(dcc:str, version: str, api:str):
    current_directory = pathlib.Path(__file__).parent
    sub_module_name = f"skin_plus_plus_{api}_{version}"
    sub_module_path = current_directory / f"dccs/{dcc}" / sub_module_name

    if not sub_module_path.exists():
        raise FileNotFoundError(f"Unsupported DCC version!")

    import_path = f"{__name__}.dccs.{dcc}.{sub_module_name}.skin_plus_plus_{api}"
    backend = importlib.import_module(import_path)

    global get_skin_data
    global set_skin_weights
    # global get_vertex_positions

    get_skin_data = backend.get_skin_data
    set_skin_weights = backend.set_skin_weights
    # get_vertex_positions = skin_plus_plus_pymxs.get_vertex_positions

    return backend


def set_debug(value: bool):
    """
    Toggle debug mode on or off.

    ---

    Debug mode is off by default.

    Arguments:
    ----------
    - `value`: Boolean to control the state of debug mode.

    Returns:
    --------
    - `None`
    """
    __get_skin_plus_plus_py(__py_version__, debug=value)


# DO NOT REMOVE - Required for access to SkinData class:
__get_skin_plus_plus_py(__py_version__)



executable = sys.executable.lower()
if "3ds max" in executable:
    from pymxs import runtime as mxRt

    version_info = mxRt.MaxVersion()
    version_number = version_info[7]
    __get_dcc_backend("max", version_number, "pymxs")
    current_dcc = "max"


elif "maya" in executable:
    from pymel import versions

    version = str(versions.current())[:4]
    __get_dcc_backend("maya", version, "pymaya")
    current_dcc = "maya"

else:
    raise RuntimeError(f"Unsupported executable: {executable}")


_typing = False
if _typing:
    from typing import Any

    from . import dccs
    from . import py

    from . import core
    from . import io
    from . import mesh

    from .core import export_skin_data
    from .core import import_skin_data
    from .core import FileType

    from .io import save
    from .io import load
    from .io import max_to_maya
    from .io import maya_to_max
del _typing


# this avoids having to import every sub-package to find where
# the object should be imported from:
_object_import_map = {
    "export_skin_data": "core",
    "import_skin_data": "core",
    "FileType": "core",
    "save": "io",
    "load": "io",
    "max_to_maya": "io",
    "maya_to_max": "io",
}


def __getattr__(name: str) -> Any:
    if name in _object_import_map:
        package_name = _object_import_map[name]
        module = importlib.import_module(f"{__package__}.{package_name}")
        return getattr(module, name)

    return importlib.import_module(f"{__package__}.{name}")


__all__ = (
    "dccs",
    "py",

    "core",
    "io",
    "mesh",

    "current_dcc",

    "get_skin_data",
    "set_skin_weights",
    "set_debug",

    "export_skin_data",
    "import_skin_data",
    "FileType",

    "save",
    "load",
    "max_to_maya",
    "maya_to_max"
)
