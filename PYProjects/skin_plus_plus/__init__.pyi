from __future__ import annotations

from . import _types
from .dccs import core as _dccs_core
from . import skin_plus_plus_py
from .core import FileType as _FileType
from .core import export_skin_data as _export_skin_data
from .core import import_skin_data as _import_skin_data
from .io import save as _save
from .io import load as _load
from .io import max_to_maya as _max_to_maya
from .io import maya_to_max as _maya_to_max


FileType = _FileType
export_skin_data = _export_skin_data
import_skin_data = _import_skin_data
save = _save
load = _load
max_to_maya = _max_to_maya
maya_to_max = _maya_to_max

current_dcc: str = ...
"""
The name of the current DCC.
"""
current_host_interface: _dccs_core.IHost = ...
"""
The interface to the current Host
"""

SkinData = skin_plus_plus_py.SkinData

def extract_skin_data(node: _types.T_Node) -> SkinData:
    """
    Extract skin data from the given DCC node.
    """
    ...
def apply_skin_data(node: _types.T_Node, skin_data: SkinData) -> bool:
    """
    Apply the given skin data to the given DCC node.
    """
    ...