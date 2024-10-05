from __future__ import annotations

from beartype.vale import Is
import beartype.door as btdoor
import numpy as np
import site
import typeguard
import typing
import typing_extensions

# from typing import assert_never
# from typing_extensions import assert_type

from pymxs import runtime as mxrt

site.addsitedir(
    "D:/Code/Git/SkinPlusPlus-latest/PYProjects/.venvs/py310/.venv/Lib/site-packages"
)


T_Node = typing.TypeVar("T_Node", bound=np.ndarray)

Numpy2DFloatArray = typing_extensions.Annotated[
    np.ndarray[typing.Any, np.dtype[np.float64]],
    Is[lambda array: array.ndim == 2 and np.issubdtype(array.dtype, np.floating)],
]


class Test(typing.Generic[T_Node]):
    def __init__(self, arg1: T_Node):
        self.arg1 = arg1


class Test2(Test[Numpy2DFloatArray]):

    @typeguard.typechecked()
    def test_fn(self):
        return self.arg1


# test_fn("mxrt.Selection[0]")
