from __future__ import annotations

import importlib
import pathlib
import site
import sys


# DO NOT REMOVE - Required for access to SkinData class:
from . import skin_plus_plus_py

executable = sys.executable.lower()
if "3ds max" in executable:

    from pymxs import runtime as mxRt

    version_info = mxRt.MaxVersion()
    version_number = version_info[0]
    current_directory = pathlib.Path(__file__).parent
    max_sub_module_name = f"skin_plus_plus_pymxs_{version_number}"
    max_sub_module_path = current_directory / "dccs/max" / max_sub_module_name

    if not max_sub_module_path.exists():
        raise FileNotFoundError(f"Unsupported DCC version!")

    import_path = f"{__name__}.dccs.max.{max_sub_module_name}.skin_plus_plus_pymxs"
    skin_plus_plus_pymxs = importlib.import_module(import_path)

    get_data = skin_plus_plus_pymxs.get_data
    get_vertex_positions = skin_plus_plus_pymxs.get_vertex_positions
    set_skin_weights = skin_plus_plus_pymxs.set_skin_weights

else:
    raise RuntimeError(f"Unsupported executable: {executable}")


from .core import export_skin_data
from .core import import_skin_data

__all__ = (
    "export_skin_data",
    "get_data",
    "get_vertex_positions",
    "import_skin_data",
    "set_skin_weights"
)


if __name__ == "__main__":
    import skin_plus_plus
    importlib.reload(skin_plus_plus)