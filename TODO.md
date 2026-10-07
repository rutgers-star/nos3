# SPICEsat Flight Software TODO

The Mission Design Document (MDD), Software Design Document (SDD), and Command and Telemetry List (CTL) are the current design authorities for mission intent. Current `main` is authoritative for implemented software. Known differences must be reconciled explicitly.

The current busOBC/payOBC code uses the pinned `payload-link` framing library, not SPICEnet. Its frame is a 4-byte sync marker, 2-byte big-endian body length, one complete CCSDS Space Packet, and CRC-16/CCITT-FALSE. The SDD and CTL now explicitly supersede the former SPICEnet CRC-8, COP-1 sequence-byte, and SNACK framing details.

As of 2026-10-06, local `main` is synchronized with `origin/main` at `beb9f664`. PR #13 consolidated CI into `.github/workflows/build.yml`: it retained the useful triggers and FSW-test intent from the old pipeline, removed the failing host-based `.github/workflows/build-and-test.yml`, and extended the NOS3 workflow with flight-target, simulator-test, CryptoLib, and OpenC3 validation jobs.

## 1. Clean Up the Old Branch

- [x] Switch the local repository to the latest `origin/main`.
- [x] Delete the obsolete local `dev` branch.
- [x] Delete `origin/dev` from GitHub.

## 2. Stabilize the Existing `main` Baseline

- [x] Initialize/update every submodule to the revisions pinned by `main`.
- [x] Perform a clean local configuration and build.
- [x] Build and run the existing FSW unit tests.
- [x] Build the NOS3 simulators.
- [x] Record every build, test, configuration, and runtime failure as a tracked issue.
- [x] Fix existing defects and repeat the clean build/test cycle until the current baseline works.

Baseline validation completed 2026-10-06 at `beb9f664`:

- Recursive submodules are initialized at their pinned revisions, with no dirty or mismatched submodules.
- `make clean`, `make config`, and the flight-software build passed using `ivvitc/nos3-64:20260619`.
- The FSW unit-test build passed; CTest reported 126 of 126 tests passed.
- The NOS3 simulator build passed.
- The CryptoLib build and pinned OpenC3 build, gem validation, and plugin installation passed. Temporary OpenC3 services were stopped afterward.
- No source, configuration, test, or runtime defect remained to track, so no issue was created. Initial setup interruptions were limited to a pre-existing generated `gsw/cosmos` directory and a non-TTY Docker invocation; both were resolved without source changes. The prior `gsw/cosmos` contents were preserved at `/home/swcaskey/nos3-gsw-cosmos-backup-20261006`.

## 3. Fix Existing Components and Documentation

- [x] Review and stabilize the newly expanded `payload_if`/PayOBC implementation, including its link model, OpenC3 definitions, simulator tests, and remaining template placeholders.
- [x] Adopt and document one busOBC/payOBC protocol based on the current `payload-link` implementation.
- [x] Update the SDD and CTL to remove or clearly supersede obsolete SPICEnet framing details.
- [x] Reconcile the Raspberry Pi cFS component and Python prototype with the approved payOBC architecture.
- [x] Correct the TMP100 conversion used by `thermal_control`.
- [x] Add thermal-control unit and closed-loop integration tests.
- [x] Reconcile the thermal and EPS simulations with the intended flight-hardware interfaces.
- [x] Archive or replace the obsolete root `Design Doc.md`.

Section 3 implementation completed 2026-10-06 from baseline `beb9f664`:

- The mission payload link is documented as RS-422 at 115200 baud, 8N1, carrying one unsegmented CCSDS Space Packet per `payload-link` frame. Length, CRC coverage, APID direction, and resynchronization behavior are now explicit.
- `payload_if`, its OpenC3 command/telemetry definitions, F Prime message IDs, and the standalone simulator link model are aligned. The undefined configuration command is rejected rather than reporting false success.
- The generic Raspberry Pi cFS/SBN component is disabled in the mission configuration and identified as legacy. The Python prototype now implements the approved serial framing and APID-direction checks.
- TMP100 signed 12-bit conversion, heater fail-safe shutdown, and EPS switch assignment were corrected. Unit tests cover positive and negative conversion, hysteresis, telemetry-to-heater control, and disable behavior.
- Heater, TMP100, and EPS simulation behavior now models the dedicated 12 V heater on EPS switch 7 and permits closed-loop temperature driving.
- The root design export was replaced with a short supersession notice. The SDD and CTL retain RevB as the authority for application APIDs/opcodes while explicitly superseding its SPICEnet frame layer.
- Validation passed: configuration generation, FSW unit-test build, all 127 CTest cases, simulator build, the standalone payload-link simulator test, three Python link tests, and OpenC3 gem generation/validation/plugin installation. A LaTeX compiler was not available locally, so PDF regeneration remains part of the document-release workflow.

Detailed future mission capabilities are intentionally deferred until the existing components are stable.

## 4. Verify the Repaired Components

Start this section after Section 3.

- [ ] Run the repaired components' unit and coverage tests.
- [ ] Run focused NOS3 integration tests for payload communication, thermal control, and EPS interaction.
- [ ] Exercise invalid frames, boundary sizes, partial UART I/O, restart, timeout, and recovery behavior.
- [ ] Record the software commit, configuration, procedure, acceptance criteria, and result.
- [ ] Confirm the stabilized baseline before starting branch/pipeline work.

## 5. Draft and Migrate Legacy GitHub Work

Complete this section before creating the new `dev` branch.

- [x] Inventory the eight open issues in `rutgers-star/nos3_spicesat`.
- [x] Draft rewritten legacy issues against the current MDD, SDD, CTL, and `main` implementation.
- [x] Create the reviewable draft issues in `rutgers-star/nos3` with source links to the legacy issues.
- [x] Recreate useful labels and acceptance criteria.
- [ ] Review and revise the migrated issue drafts with the team before treating them as approved work.
- [ ] Update the GitHub issues as code, build results, and document decisions change their scope, status, dependencies, or acceptance criteria.
- [ ] Assign current owners and milestones after team review.
- [ ] Inspect/recreate any organization Project board after GitHub Project access is authorized.
- [ ] Close or archive the superseded legacy issues after the migration is reviewed.
