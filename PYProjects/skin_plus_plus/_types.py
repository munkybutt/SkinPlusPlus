from __future__ import annotations

_typing = False
if _typing:
    from pymel.core import nodetypes as pm_ntypes
    from pymxs import runtime as mxrt
    from typing import Callable
    from typing import Sequence
    from typing import TypeVar
    from typing import Union
    from typing import Protocol

    from . import SkinData

    T_Node = TypeVar("T_Node", mxrt.Node, pm_ntypes.DagNode)
    T_Handle = Union[int, str]
    T_CApSD = Callable[[T_Handle, SkinData], None]


    class T_EXSD(Protocol):
        """
        Signature for extrac skin data function
        """
        def __call__(self, handle: T_Handle, vertex_ids: Sequence[int] | None = None) -> SkinData: ...

del _typing