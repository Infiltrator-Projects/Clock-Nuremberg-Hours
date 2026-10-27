# Historical Nuremberg Great Clock — validation requirements

**Evidence status:** No independent executable, standalone CI, installer or released artifact is established by these documents. All checkboxes are future gates, not claims of passing tests.

## Qualification checklist

- [ ] Establish exact original Common revision, licensed source provenance, direct and shared dependencies.
- [ ] Keep the implementation limited to stable ID `nuremberg-hours`; do not embed another independent resource's algorithm.
- [ ] Produce the directly named portable shared object `Historical Nuremberg Great Clock.so`.
- [ ] Verify metadata version, name, ID, kind, loadability and executable ABI through Common.
- [ ] Verify model-specific computation and historical-reference dates/times against evidence independent of self-round-tripping.
- [ ] Reject malformed, out-of-range and unsupported inputs with bounded work and no caller-owned output mutation.
- [ ] Verify bad/missing/different-kind resources cannot silently fall back or break other installed resources.
- [ ] Test the actual installed package bytes through the native resource directory.
- [ ] Verify clean installation, upgrade, removal and recovery without conflicting with the combined Common package.
- [ ] Record reproducibility, package/hash, build environment and release commit.
- [ ] Confirm consumer discovery in Calendar, Files and System Settings without embedding duplicate implementations.

## Model-specific reference qualification

Verify the documented 1488 civic Wendetage, without applying user location.

**Documented model:** Nürnberg Great Clock fixed civic Wendetage based on a documented 1488 Julian schedule; separate day/night allocation.

**Fidelity boundary:** Modern annual projection of a documented civic schedule; user coordinates do not change it.

Also test visible-change scheduling, seconds capability, time-zone and rollover handling. 

## Prior evidence versus independent release evidence

The existing Common suite (`resources/temporal/tests/test-clocks.c`) supplies the starting contract, but a passing Common pack does not prove a standalone package or this repository's release. Attach specific commits, test runs, environments and artifacts before marking any item complete.

- [Libraries/Common temporal sources](https://github.com/Infiltrator-Projects/Libraries/tree/main/resources/temporal)
- [Current canonical build catalogue](https://github.com/Infiltrator-Projects/Libraries/blob/main/resources/temporal/Makefile)
- [Calendar chronology and clock models](https://github.com/Infiltrator-Projects/Calendar/blob/main/docs/MODELS.md)
- [Historical fidelity audit](https://github.com/Infiltrator-Projects/Libraries/blob/main/docs/TEMPORAL_HISTORICAL_FIDELITY.md)
- [Temporal discovery and executable provider ABI](https://github.com/Infiltrator-Projects/Libraries/blob/main/docs/TEMPORAL_PROVIDERS.md)
