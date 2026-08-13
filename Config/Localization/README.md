# Unreal localization targets

- `GsId`: runtime texts, loaded with `Always`.
- `GsIdEditor`: editor and reflected Blueprint metadata, loaded with `Editor`.

The `.ini` files are templates. `Scripts/Update-GsLocalization.*` replaces
`__GS_PLUGIN_ROOT__` with the installed plugin path before invoking Unreal commandlets.
