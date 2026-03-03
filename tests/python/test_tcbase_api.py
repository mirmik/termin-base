from pathlib import Path

import tcbase


def test_input_enums_are_accessible():
    assert tcbase.Action.PRESS.value == 1
    assert tcbase.Action.RELEASE.value == 0
    assert tcbase.MouseButton.LEFT.value == 0
    assert tcbase.Mods.SHIFT.value != 0
    assert tcbase.Key.A.value == 65


def test_logging_functions_are_callable():
    tcbase.log.info("tcbase-python-smoke")
    tcbase.log.warn("tcbase-python-warn")
    tcbase.log.error("tcbase-python-error")


def test_settings_roundtrip_with_groups(tmp_path: Path):
    settings_path = tmp_path / "settings.json"
    s = tcbase.Settings(str(settings_path), True)

    s.set("value", 42)
    s.begin_group("ui")
    s.set("theme", "dark")
    s.set("width", 1280)
    s.end_group()
    s.save()

    s2 = tcbase.Settings(str(settings_path), True)
    s2.load()

    assert s2.get("value", None) == 42
    assert s2.get("ui/theme", None) == "dark"
    assert s2.get("ui/width", None) == 1280
    assert s2.contains("ui/theme")
