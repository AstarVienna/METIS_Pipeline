"""
Checks the ADARI reports: every workflow task is wired to a report, every report loads the way
genreport loads it (via ADARI_REPORTS_DIR), and the placeholder report renders PNGs from a SOF.
"""
import ast
import json
from pathlib import Path

import numpy as np
import pytest
from astropy.io import fits

pytest.importorskip("adari_core")
from adari_core.server.server_png import AdariServerPng  # noqa: E402

METIS = Path(__file__).resolve().parents[1]
REPORTS_DIR = METIS / "reports"
WORKFLOWS_DIR = METIS / "workflows" / "metis"
REPORTS = sorted(p.stem for p in REPORTS_DIR.glob("metis_*.py") if p.stem != "metis_utils")


def _workflow_tasks():
    """(task name, recipe, [report names]) for every task built in the workflow modules."""
    tasks = []
    for path in sorted(WORKFLOWS_DIR.glob("*.py")):
        for node in ast.walk(ast.parse(path.read_text())):
            if not isinstance(node, ast.Assign) or not isinstance(node.value, ast.Call):
                continue
            calls, call = {}, node.value
            while isinstance(call, ast.Call):
                name = getattr(call.func, "attr", getattr(call.func, "id", None))
                calls.setdefault(name, []).append(call.args)
                call = getattr(call.func, "value", None)
            if "task" in calls and "with_recipe" in calls:
                reports = [args[0].value for args in calls.get("with_report", [])]
                tasks.append((calls["task"][0][0].value, calls["with_recipe"][0][0].value, reports))
    return tasks


TASKS = _workflow_tasks()


@pytest.fixture
def server(monkeypatch, tmp_path):
    monkeypatch.setenv("ADARI_REPORTS_DIR", str(REPORTS_DIR))
    monkeypatch.chdir(tmp_path)  # the PNG backend writes into the working directory
    return AdariServerPng({})


@pytest.mark.parametrize("task,recipe,reports", TASKS, ids=[t[0] for t in TASKS])
def test_task_has_reports(task, recipe, reports):
    assert sorted(reports) == sorted(["metis_rawdisp", recipe])


@pytest.mark.parametrize("name", REPORTS)
def test_report_loads(server, name):
    assert server.loadreport(name) is None
    assert server.report_dict[name]["report"].name == name


def _write(path, header, data=None):
    hdul = fits.HDUList([fits.PrimaryHDU(header=fits.Header(header))])
    if data is not None:
        hdul.append(fits.ImageHDU(data, name="DET1.SCI"))
    hdul.writeto(path)
    return path


@pytest.fixture
def sof(tmp_path):
    img = np.arange(64 * 64, dtype=float).reshape(64, 64)
    raw = {"HIERARCH ESO DPR CATG": "CALIB", "HIERARCH ESO DPR TYPE": "DARK", "HIERARCH ESO DPR TECH": "IMAGE,LM"}
    dark = {"HIERARCH ESO PRO CATG": "MASTER_DARK_2RG", "HIERARCH ESO PRO REC1 ID": "metis_det_dark",
            **{f"HIERARCH ESO QC DARK VAL{i}": float(i) for i in range(10)}}
    gain = {"HIERARCH ESO PRO CATG": "GAIN_MAP_2RG", "HIERARCH ESO PRO REC1 ID": "metis_det_lingain"}
    frames = [
        (_write(tmp_path / "raw1.fits", raw, img), "DARK_2RG_RAW"),
        (_write(tmp_path / "raw2.fits", raw, img), "DARK_2RG_RAW"),
        (_write(tmp_path / "gain.fits", gain, np.ones(1)), "GAIN_MAP_2RG"),
        (_write(tmp_path / "dark.fits", dark, img), "MASTER_DARK_2RG"),
    ]
    path = tmp_path / "in.sof"
    path.write_text("".join(f"{p} {catg}\n" for p, catg in frames))
    return path


def _run(server, name, sof):
    assert server.loadreport(name) is None
    assert server.runreport(name, str(sof), "out.json") == 0
    return json.loads(Path("out.json").read_text())


def test_dummy_report_shows_only_its_recipes_products(server, sof):
    artifacts = _run(server, "metis_det_dark", sof)
    assert [a["prodcatg"] for a in artifacts] == ["MASTER_DARK_2RG"]
    assert all(Path(a["filename"]).is_file() and a["mime_type"] == "image/png" for a in artifacts)


def test_rawdisp_shows_one_raw_frame_per_category(server, sof):
    artifacts = _run(server, "metis_rawdisp", sof)
    assert [a["prodcatg"] for a in artifacts] == ["DARK_2RG_RAW"]


def test_dummy_report_without_image_data(server, sof):
    artifacts = _run(server, "metis_det_lingain", sof)
    assert [a["prodcatg"] for a in artifacts] == ["GAIN_MAP_2RG"]
    assert Path(artifacts[0]["filename"]).is_file()
