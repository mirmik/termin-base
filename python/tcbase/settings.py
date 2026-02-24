"""Settings — persistent JSON-based key-value store with hierarchical keys."""

from tcbase._tcbase_native import Settings as _NativeSettings


class _SettingsGroup:
    """Context manager for Settings.group()."""

    def __init__(self, settings, name: str):
        self._settings = settings
        self._name = name

    def __enter__(self):
        self._settings.begin_group(self._name)
        return self._settings

    def __exit__(self, *_):
        self._settings.end_group()


class Settings(_NativeSettings):
    """Persistent JSON-based settings with hierarchical keys.

    Usage::

        settings = Settings("my-app")
        settings.set("last_dir", "/home/user")
        settings.get("last_dir", "")

        # Hierarchical keys
        settings.set("ui/theme", "dark")

        # Groups (context manager)
        with settings.group("mainwindow"):
            settings.set("width", 1280)      # -> "mainwindow/width"
            settings.get("height", 800)       # -> "mainwindow/height"
    """

    def group(self, name: str):
        """Return a context manager that pushes/pops a group prefix."""
        return _SettingsGroup(self, name)
