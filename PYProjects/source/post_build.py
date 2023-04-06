from __future__ import annotations

import pathlib
import inspect
import traceback


current_folder = pathlib.Path(inspect.getfile(lambda: None))


def get_parent_sibling_folder(sibling_name: str):
    for parent in current_folder.parents:
        for sub_path in parent.iterdir():
            if sub_path.name != sibling_name:
                continue

            return sub_path

    raise FileNotFoundError(f"{sibling_name} folder not found!")


build_folder = get_parent_sibling_folder("build")
build_x64_folder = build_folder / "x64"

skin_plus_plus_folder = get_parent_sibling_folder("skin_plus_plus")
skin_plus_plus_dccs_folder = skin_plus_plus_folder / "dccs"
skin_plus_plus_py_folder = skin_plus_plus_folder / "py"


folder_map_py = {
    "2021": "37",
    "2022": "37",
    "2023": "39",
}

folder_map_max = {
    "2021": "skin_plus_plus_pymxs_23000",
    "2022": "skin_plus_plus_pymxs_24000",
    "2023": "skin_plus_plus_pymxs_25000",
}


def move_pyd_files(
    folder_map: dict[str, str],
    target_folder: pathlib.Path,
    suffix: str | None = None,
    dcc: str | None = None,
):
    suffix = f"_{suffix}" if suffix else ""
    dcc = dcc or ""
    for year, target_folder_name in folder_map.items():
        source_folder = build_x64_folder / f"{year}-Release"
        source_file = source_folder / f"skin_plus_plus{suffix}/skin_plus_plus{suffix}.pyd"
        if not source_file.exists():
            print(f"Source file not found - skipping: {source_file}")
            continue

        target_file = target_folder / dcc / target_folder_name / f"skin_plus_plus{suffix}.pyd"
        if target_file.exists():
            target_file.unlink()

        print(f"Moving: {source_file} -> {target_file}")
        source_file.rename(target_file)


print("Moving pyd files into package")
try:
    move_pyd_files(folder_map_py, skin_plus_plus_py_folder, suffix="py")
except Exception:
    traceback.print_exc()

try:
    move_pyd_files(folder_map_max, skin_plus_plus_dccs_folder, suffix="pymxs", dcc="max")
except Exception:
    traceback.print_exc()

try:
    move_pyd_files(folder_map_max, skin_plus_plus_dccs_folder, suffix="pymaya", dcc="maya")
except Exception:
    traceback.print_exc()

# for year, target_folder_name in folder_map_py.items():
# 	source_folder = build_x64_folder / f"{year}-Release"
# 	source_file = source_folder / "skin_plus_plus_pymxs/skin_plus_plus_pymxs.pyd"
# 	if not source_file.exists():
# 		continue

# 	target_file = skin_plus_plus_py_folder / target_folder_name / "skin_plus_plus.pyd"
# 	if target_file.exists():
# 		target_file.unlink()

# 	source_file.rename(target_file)
