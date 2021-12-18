import pathlib
import site
import sys


# Required for access to SkinData class:
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

    site.addsitedir(str(max_sub_module_path))

    from skin_plus_plus_pymxs import get_data
    from skin_plus_plus_pymxs import get_vertex_positions
    from skin_plus_plus_pymxs import set_skin_weights

else:
    raise RuntimeError(f"Unsupported executable: {executable}")


from .core import export_skin_data
from .core import import_skin_data

__all__ = (
    "export_skin_data",
    "get_vertex_positions",
    "get_data",
    "import_skin_data",
    "set_skin_weights"
)
