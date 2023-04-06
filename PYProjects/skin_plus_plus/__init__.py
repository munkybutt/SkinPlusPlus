from __future__ import annotations

import importlib
import pathlib
import sys

get_skin_data = None
set_skin_weights = None
# get_vertex_positions = None


def __get_environmet_core(python_version: str):
    import importlib
    import pathlib

    current_directory = pathlib.Path(__file__).parent
    sub_module_path = current_directory / f"py/{python_version}"

    if not sub_module_path.exists():
        raise FileNotFoundError(f"Unsupported Python version!")

    import_path = f"skin_plus_plus.py.{python_version}.skin_plus_plus_py"
    core = importlib.import_module(import_path)
    return core


def __get_dcc_backend(dcc: str, version: str, api: str):
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


# DO NOT REMOVE - Required for access to SkinData class:
__version = f"{sys.version_info.major}{sys.version_info.minor}"
__get_environmet_core(__version)


executable = sys.executable.lower()
if "3ds max" in executable:
    from pymxs import runtime as mxRt

    version_info = mxRt.MaxVersion()
    version_number = version_info[0]
    __get_dcc_backend("max", version_number, "pymxs")


elif "maya" in executable:
    from pymel import versions  # type: ignore

    version = str(versions.current())[:4]
    __get_dcc_backend("maya", version, "pymaya")

else:
    raise RuntimeError(f"Unsupported executable: {executable}")


from .core import export_skin_data
from .core import import_skin_data

__all__ = (
    "export_skin_data",
    "get_skin_data",
    # "get_vertex_positions",
    "import_skin_data",
    "set_skin_weights",
)


if __name__ == "__main__":
    import skin_plus_plus

    importlib.reload(skin_plus_plus)
