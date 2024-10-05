"""
Interface for working with skin data in various dcc's
"""
from __future__ import annotations

_typing = False
if _typing:
    from . import core
    from . import max
    from . import maya
    from types import ModuleType
del _typing


def __getattr__(name: str) -> ModuleType | None:
    import importlib

    try:
        return importlib.import_module(f"{__package__}.{name}")

    except ImportError as error:
        if f"{__package__}.{name}" in str(error):
            raise AttributeError(f"{__package__} has no attribute named: {name}")

        raise


__all__ = (
    "core",
    "max",
    "maya",
)


def __dir__():
    return __all__
