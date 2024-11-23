from __future__ import annotations

import importlib
import os
import pathlib
import sys

_typing = False
if _typing:
    import skin_plus_plus_py as skpp

    from . import _types
    from .dccs import core as dccs_core
del _typing

current_host_interface: dccs_core.IHost | None = None

extract_skin_data: _types.C_ExtractSkinData | None = None
apply_skin_data: _types.C_ApplySkinData | None = None
get_vertex_positions = None
skin_plus_plus_py = None
SkinData: skpp.SkinData | None = None

try:
    is_reloading  # pyright: ignore[reportUndefinedVariable, reportUnusedExpression]
    is_reloading = True
except NameError:
    is_reloading = False

__py_version__ = f"{sys.version_info.major}{sys.version_info.minor}"


def _activate_skin_plus_plus_py_(python_version: str, debug: bool = False):
    global skin_plus_plus_py
    global SkinData

    debug = bool(os.environ.get("SKIN_PLUS_PLUS_DEBUG", False)) or debug
    if debug:
        python_version = f"debug_{python_version}"

    current_directory = pathlib.Path(__file__).parent
    sub_module_path = current_directory / f"py/{python_version}"

    if not sub_module_path.exists():
        raise FileNotFoundError(f"Unsupported Python version: {python_version}")

    import_path = f"skin_plus_plus.py.{python_version}.skin_plus_plus_py"
    if "skin_plus_plus_py" in sys.modules:
        del sys.modules["skin_plus_plus_py"]

    skin_plus_plus_py = importlib.import_module(import_path)
    if is_reloading:
        _ = importlib.reload(skin_plus_plus_py)

    SkinData = skin_plus_plus_py.SkinData


def _activate_host_():
    global current_host_interface

    global apply_skin_data
    global extract_skin_data
    global get_vertex_positions

    executable = sys.executable.lower()
    if "3ds max" in executable:
        from .dccs.max import IHost

        current_host_interface = IHost()

    elif "maya" in executable:
        from .dccs.maya import IHost

        current_host_interface = IHost()

    if current_host_interface is None:
        raise RuntimeError(f"Unsupported executable: {executable}")

    extract_skin_data = current_host_interface.extract_skin_data
    apply_skin_data = current_host_interface.apply_skin_data
    get_vertex_positions = current_host_interface.get_vertex_positions


def set_debug(value: bool) -> None:
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
    _activate_skin_plus_plus_py_(__py_version__, debug=value)


_activate_skin_plus_plus_py_(__py_version__)
_activate_host_()


_typing = False
if _typing:
    from typing import Any

    from . import core
    from . import dccs
    from . import enums
    from . import io
    from . import py
    from .core import export_skin_data
    from .core import import_skin_data
    from .core import load_from_json_file
    from .core import load_from_pickle_file
    from .enums import ApplicationMode
    from .enums import FileType
    from .io import load
    from .io import max_to_maya
    from .io import maya_to_max
    from .io import save


def __getattr__(name: str) -> Any:
    # this avoids having to import every sub-package to find where
    # the object should be imported from:
    _object_import_map = {
        "load_from_json_file": "core",
        "load_from_pickle_file": "core",
        "export_skin_data": "core",
        "import_skin_data": "core",
        "FileType": "enums",
        "ApplicationMode": "enums",
        "save": "io",
        "load": "io",
        "max_to_maya": "io",
        "maya_to_max": "io",
    }

    if name in _object_import_map:
        package_name = _object_import_map[name]
        module = importlib.import_module(f"{__package__}.{package_name}")
        return getattr(module, name)

    return importlib.import_module(f"{__package__}.{name}")


__all__ = (
    "SkinData",
    "dccs",
    "py",
    "core",
    "enums",
    "io",
    "current_host_interface",
    "extract_skin_data",
    "apply_skin_data",
    "load_from_json_file",
    "load_from_pickle_file",
    "export_skin_data",
    "import_skin_data",
    "save",
    "load",
    "FileType",
    "ApplicationMode",
    "max_to_maya",
    "maya_to_max",
    "set_debug",
)


def __dir__():
    return __all__
