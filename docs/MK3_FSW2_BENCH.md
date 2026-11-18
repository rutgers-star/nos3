# FSW-2 busOBC bench check

**Verification status: open.** Rev E FSW-2 requires an operator command to exit Safe. The MDD §3.7 specifies the recovery destination as **Idle**. This check uses the busOBC and EGSE hardline. The PayOBC is optional. [Rev E requirement](https://docs.google.com/spreadsheets/d/1paqaMUJ77agVSauEICsr32fSxNOsj9gEF4DwlThXcM0/edit#gid=1294587388&range=A125:H125) · [Verification plan](https://docs.google.com/spreadsheets/d/1rCFQ4XCJsU5cspdpFYkFFPPmXmgvwLAuUGnhMFK4wvk/edit#gid=966785229&range=A10:L10) · [Current MDD](https://drive.google.com/file/d/12v19bOyLw0DtmBu6zTUR3sWlGHmZSIkj/view)

1. Build and package MK3 `link` with the GomSpace Yocto tree from the revision under test. Record `git rev-parse HEAD`, the `repository_commit` in `MANIFEST.txt`, the archive filename, and the SHA-256 in its `.sha256` file. Verify the archive with `sha256sum -c <archive>.sha256`. Deploy to a new writable directory and verify `sha256sum -c SHA256SUMS` before starting cFS. Use the [MK3 bench setup](../scripts/mk3/bench/README.md) and confirm the running package matches the recorded one.
2. Start the OpenC3 bench tunnel and record timestamped commands, `BUSOBC MGR_HK` (`SPACECRAFT_MODE` and command counters), and `BUSOBC EVENT` (event ID and text). Request `BUSOBC MGR_REQ_HK` before and after each test action. Record the startup mode. Also capture the cFS console log.
3. Run the matrix below once in `SAFE_REBOOT` and once in `SAFE`. A fresh run with no saved MGR state starts in `SAFE_REBOOT`. After the first round and the operator exit, command `MGR_SET_MODE` to `SAFE` to set up the second round.

| Send or exercise | Expected result |
| --- | --- |
| `MGR_SET_MODE` with `IDLE`, `SCIENCE`, `SCIENCE_REBOOT`, and the other Safe state | The mode stays at the exact starting value. Each blocked change reports a rejection. |
| The MGR commands used by onboard RTSs (`SET_MODE SCIENCE` from RTS 025, `UPDATE_SCI_STATUS`, and `SCI_PASS_INC`) | The mode stays latched. Inject the same command packets from EGSE because the MK3 `link` profile does not start LC or SC. |
| An invalid mode, unknown command code, and `EXIT_SAFE` with an extra payload byte | The mode stays latched. Capture the error event and counter. |
| Operator sends `BUSOBC MGR_EXIT_SAFE` once with no payload | `MGR_HK` changes to `IDLE` and MGR emits an exit acceptance event. A repeat outside Safe is rejected and leaves `IDLE`. |

For raw test packets from the board console use the verified package's `bin/sbtool`. With `$SBTOOL` set to that binary, examples are `$SBTOOL cmd 0x18F8 9 01` (science status), `$SBTOOL cmd 0x18F8 4` (science pass), `$SBTOOL cmd 0x18F8 2 FF` (invalid mode), `$SBTOOL cmd 0x18F8 127` (unknown command), and `$SBTOOL cmd 0x18F8 10 00` (malformed exit). Send the valid exit explicitly from the operator console after the hold checks.

Record expected and observed modes, command and event timestamps, board/image ID, and any discrepancies. Attach the signed as-run procedure and logs to the verification record. Keep FSW-2 open until the hardware evidence is reviewed.
