# Changelog

All notable changes to this project will be documented in this file.

The format is based on "Keep a Changelog" and this project follows semantic versioning.

## [Unreleased]

- Development ongoing. See issues/PRs for details.

## [1.0.0] - 2025-10-26

### Added
- Initial public release of the Software-Based CPU simulator.
- Complete simulator sources and a 14-test test-suite under `tests/`.
- CMake + Ninja build system and a GitHub Actions CI workflow for Linux and Windows.
- Example programs in `examples/` and documentation in `docs/`.
- MIT License and contributor guidance.

### Fixed
- Build issues on Windows (CMake/Ninja generator adjustments) and identifier conflicts.
- CPU stepping and reset semantics corrected to match test expectations.
- Device MMIO behavior (timer high-byte handling and GPIO write semantics) corrected.

### Notes
- All tests in the included test-suite pass (14/14) at release.
# Changelog

All notable changes to this project will be documented in this file.

## [Unreleased]
- Initial import and cleanup.

## [1.0.0] - 2025-10-26
- Initial public release: core simulator, assembler, monitor, devices, examples, and tests.
- Added CI workflow, license, and contributing guide.

