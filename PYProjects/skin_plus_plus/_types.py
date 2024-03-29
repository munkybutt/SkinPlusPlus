from pymel.core import nodetypes as pm_ntypes
from pymxs import runtime as mxrt
from typing import Callable
from typing import TypeVar
from typing import Union

from . import SkinData

T_Node = TypeVar("T_Node", mxrt.Node, pm_ntypes.DagNode)
T_Handle = Union[int, str]
T_CExSD = Callable[[T_Handle], SkinData]
T_CApSD = Callable[[T_Handle, SkinData], None]