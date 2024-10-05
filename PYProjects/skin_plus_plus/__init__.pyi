from __future__ import annotations

from . import _types
from . import skin_plus_plus_py
from .core import FileType as FileType
from .core import ApplicationMode as ApplicationMode
from .core import export_skin_data as export_skin_data
from .core import import_skin_data as import_skin_data
from .dccs import core as _dccs_core
from .io import load as load
from .io import max_to_maya as max_to_maya
from .io import maya_to_max as maya_to_max
from .io import save as save
from typing import Sequence


current_dcc: str = ...
"""
The name of the current DCC.
"""
current_host_interface: _dccs_core.IHost = ...
"""
The interface to the current Host
"""

SkinData = skin_plus_plus_py.SkinData


def extract_skin_data(
    node: _types.T_Handle, vertex_ids: Sequence[int] | None = None
) -> SkinData:
    """
    Extract skin data from the given DCC node.
    """
    ...


def apply_skin_data(
    node: _types.T_Handle,
    skin_data: SkinData,
    application_mode: ApplicationMode = ApplicationMode.order,
) -> bool:
    """
    Apply the given skin data to the given DCC node.
    """
    ...
