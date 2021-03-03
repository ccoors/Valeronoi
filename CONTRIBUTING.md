# Contributing

Contributions to Valeronoi are welcome! Please note that this project is licensed under the GPLv3, so your contributions will also fall under that license.

## Development

Make sure to follow the existing project structure. The Qt-Dialog files (\*.ui) are generated using the Qt 5 Designer, so they should be compatible with both Qt 5 and Qt 6. This project has a [pre-commit](https://pre-commit.com/) config, so make sure any PRs pass the pre-commit checks.

By default, IPO Optimizations are enabled. This can lead to highly increased linking times. During development it is advised to disable IPO (e.g. remove `set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)` from CMakeLists.txt).

### Coding conventions

- Include guards: `#ifndef VALERONOI_$PATH$_$FILENAME$_H`
- Variables: `snake_case`
- Members: `m_name`
- Qt Signals: `signal_name`
- Qt Slots: `slot_name`
