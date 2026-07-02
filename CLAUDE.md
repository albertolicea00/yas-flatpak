# CLAUDE.md — YAS Flatpak

## What
Native GUI wrapper for **Flatpak** (`flatpak`). Part of YAS suite.
Status: **scaffolded & unit-tested** — vendored core + adapter + QML shell compile, 3/3 QtTest suites pass (verified cross-compiling on macOS). Pending: build + QA on the real target platform.

## Stack
- C++20 + Qt 6.7+ (Qt Quick / QML), CMake ≥ 3.24, GCC/Clang
- Native windowing via Qt QPA plugins: **wayland** with **xcb** (X11) fallback.
- CLI execution: `QProcess` wrapping `flatpak`. Never bundle it.
- Architecture: **vendored core copy** (identical across suite, NO shared library by design) + `flatpak` adapter. Master template: `../yas-core/` local folder (not published). Core fixes must be replicated across repos.

## Target platform
Any Linux distro with flatpak. x64 + arm64.

## flatpak specifics
- Two install scopes: `--user` (no privileges — default for the GUI) and `--system` (polkit handles auth automatically via flatpak's own policy). Easiest privilege story on Linux; good first Linux app.
- Key commands: `flatpak search`, `flatpak info`, `flatpak list --columns=...`, `flatpak install/uninstall/update -y`, `flatpak mask/unmask` (pin equivalent), `flatpak uninstall --unused`, `flatpak remotes`, `flatpak remote-add` (flathub etc.), `flatpak history`, `flatpak permissions` / `flatpak override`.
- `--columns=` gives stable tab-separated output — use it everywhere instead of default formatting.
- Runtimes vs apps: UI must distinguish; unused-runtime cleanup is a headline feature.
- Per-app sandbox permissions view/override is a differentiator vs every other YAS app — expose it.
- Appstream metadata (icons, screenshots, descriptions) available locally under remote's appstream dir — use for rich UI instead of CLI-only metadata. libflatpak exists if CLI wrapping falls short; v1 wraps CLI per suite convention.

## Design (see DESIGN.md)
- Dark theme. Base `#222629`, accent **Lime `#CDDC39`**, highlight `#CDDC391A`, text `#F8F8F2` / `#ACADAD`.
- App tag: **FLATPAK**. Fonts: Outfit/Inter (UI), Fira Code or JetBrains Mono (CLI output).

## Conventions
- Conventional Commits (no co-author attribution), feature branches, PRs per CONTRIBUTING.md. Never push to origin without explicit ask.
- Planned layout (mirrors yas-brew, the reference scaffold): `src/core/` (vendored), `src/flatpakadapter.*`, `src/main.cpp`, `qml/core/` (vendored) + `qml/Main.qml`, `tests/`, `assets/fonts/`, `icons/` (exists), CMakeLists.txt + CMakePresets.json.
- Packaging: flatpak on Flathub (dogfooding) — mind sandbox: app needs talk-permission to run host `flatpak` (flatpak-spawn --host).

## Key files
README.md · DESIGN.md · CONTRIBUTING.md · EULA.md · SECURITY.md · icons/
