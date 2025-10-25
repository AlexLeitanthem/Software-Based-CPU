# Contributing

Thanks for your interest in improving this project! A few guidelines to make contributions smooth.

- Fork the repository and create a feature branch for your change.
- Keep changes small and focus on one issue per PR.
- Ensure the project builds and tests pass locally before opening a pull request.
  - Build: cmake -S . -B build -G Ninja
  - Build: cmake --build build --parallel
  - Test: ./build/build/tests.exe (on Windows: .\build\build\tests.exe)
- Prefer clear, well-documented code. Inline comments are welcome for non-obvious behavior.
- Follow the existing code style (C, C99); avoid large formatting changes in the same patch.
- When adding features, include or update tests under `tests/` where appropriate.
- For bug reports, include steps to reproduce and platform/compiler details.

Maintainers will review pull requests and provide feedback. Thanks!
Thanks for considering contributing to Software-Based-CPU!

This file contains a short guide to help you get started with contributing.

How to contribute
- Fork the repository and create a feature branch from `main`.
- Make small, focused commits with clear messages.
- Run the test suite locally before submitting a PR:

```powershell
cmake -S . -B build -G Ninja
cmake --build build --parallel
.\build\build\tests.exe
```

- Open a pull request against `main` and include a short description of the change and reasoning.

Coding guidelines
- The codebase uses C99. Keep changes compatible with C99.
- Keep functions small and well-documented.
- Add tests for any behaviour or bugfix you change.

Review process
- PRs should include a short description and link to any related issue.
- CI will run the build and tests; address failing checks before requesting final review.

Reporting bugs
- Open an issue with a minimal reproduction and steps to reproduce.

Thanks — maintainers
