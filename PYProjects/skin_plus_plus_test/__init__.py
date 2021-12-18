import sys

# Required for access to SkinData class:
from . import _skin_plus_plus_py

executable = sys.executable
if "3ds max" in executable:
	from ._skin_plus_plus_pymxs import get_data
	from ._skin_plus_plus_pymxs import get_vertex_positions
	from ._skin_plus_plus_pymxs import set_skin_weights

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
