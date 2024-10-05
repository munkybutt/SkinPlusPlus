"""
Interface for working with skin data in 3DsMax
"""
from __future__ import annotations

_typing = False
if _typing:
    from .core import IHost
    from types import ModuleType
del _typing


def __getattr__(name: str) -> ModuleType | IHost | None:
    import importlib

    _object_map = {
        "IHost": "core"
    }

    try:
        if name in _object_map:
            module_name = _object_map[name]
            module = importlib.import_module(f"{__package__}.{module_name}")
            return getattr(module, name)

        return importlib.import_module(f"{__package__}.{name}")

    except ImportError as error:
        if f"{__package__}.{name}" in str(error):
            raise AttributeError(f"{__package__} has no attribute named: {name}")

        raise


__all__ = (
    "IHost",
)


def __dir__():
    return __all__
