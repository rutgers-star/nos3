# busOBC to PayOBC bench tools

For the payload link on the bench: the busOBC (NanoCom SDR MK3) on J14, RS-422 to the PayOBC (Raspberry Pi with the Zihatec RS422 HAT, running PiDaemon). The full guide, with wiring and troubleshooting, is the team vault note "Bench guide - busOBC to payOBC link".

| Directory | Runs on | What |
| --- | --- | --- |
| `pi/` | the machine with the SDR's FTDI cable on J1 | `sdr` (shared console), `sdr-deploy` (copy a package over the console), `uart-stats` and `uart-watch` (Pi link UART), `console-relay` |
| `sdr/` | the SDR | `bench.sh`: `status`, `cfs_start`, `cfs_stop`, `mon`, `hk`, `pingpi`, `picmd`, `events` |
| `laptop/` | the laptop running OpenC3 | `sdr-link` (tunnel from OpenC3 to cFS over J1, plus a link monitor), `sdr-link-check` (60-check rehearsal), `link-fault` (bad frames from the Pi), `sdr-link-via-pi` (cable on the Pi instead) |
| `openc3/` | the laptop | `install.sh` loads the `openc3-cosmos-star-bench` plugin: targets BUSOBC, PAYOBC, GROUND and four screens |

## Demo from OpenC3

With the FTDI cable on the laptop, the SDR booted and `daemond` running on the Pi:

```sh
make fsw-mk3-package MK3_PROFILE=link
scripts/mk3/bench/pi/sdr-deploy dist/mk3/spicesat-mk3-link-localq-<commit>-<time>.tar.gz
scripts/mk3/bench/pi/sdr-deploy scripts/mk3/bench/sdr/bench.sh
scripts/mk3/bench/openc3/install.sh
scripts/mk3/bench/laptop/sdr-link         # leave it running
scripts/mk3/bench/laptop/sdr-link-check   # in a second terminal, alone: want "Failed checks: 0"
```

Then OpenC3 at http://localhost:2900, Telemetry Viewer, screen PAYOBC LINK.

Limits worth stating in a review: there is no radio in this path (J1 stands in for the ground link); the link monitor (`LINK_STATE`, `UNANSWERED`) runs on the laptop, because the flight software has no payload link watchdog yet; the Pi's payload application is a stand-in, so its temperature, storage, attitude requests and bulk data are placeholders or test patterns.
