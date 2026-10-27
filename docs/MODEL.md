# Historical Nuremberg Great Clock — computational and historical model

| Field | Value |
| --- | --- |
| Stable provider ID | `nuremberg-hours` |
| Resource kind | `clock` |
| Canonical compiled name | `Historical Nuremberg Great Clock.so` |
| Intended installation | `/System/Resources/Clocks/Historical Nuremberg Great Clock.so` |

## Maintained model

Nürnberg Great Clock fixed civic Wendetage based on a documented 1488 Julian schedule; separate day/night allocation.

## Historical scope / fidelity limitation

**Implemented interpretation:** Historical Nürnberg Great Clock.

**Fidelity boundary:** Modern annual projection of a documented civic schedule; user coordinates do not change it.

This is an account of the implemented model, not a claim that a particular mathematical projection was observed or officially used in every historical period. Where the source model uses approximations, retrospective continuation or external ICU/CLDR authorities, that distinction must remain explicit.

## Observable behaviour

The clock owns both its notation and next-visible-tick scheduling. It does not require user-supplied geographic coordinates for its stated model. Elapsed-duration calculations in Common do not provide a fallback absolute clock-face implementation.

## Unsupported inputs and reproducibility

Missing providers are unavailable; consumers must not substitute a different provider with a similar name. Malformed or out-of-range inputs must return an explicit failure without modifying outputs. Module metadata must convey its true ID and kind, independent of filename ordering.

## Provenance

The model and boundary above are documented from the current resource build map, Calendar model matrix and Common historical-fidelity audit. They have not yet been independently requalified in this repository.

- [Libraries/Common temporal sources](https://github.com/Infiltrator-Projects/Libraries/tree/main/resources/temporal)
- [Current canonical build catalogue](https://github.com/Infiltrator-Projects/Libraries/blob/main/resources/temporal/Makefile)
- [Calendar chronology and clock models](https://github.com/Infiltrator-Projects/Calendar/blob/main/docs/MODELS.md)
- [Historical fidelity audit](https://github.com/Infiltrator-Projects/Libraries/blob/main/docs/TEMPORAL_HISTORICAL_FIDELITY.md)
- [Temporal discovery and executable provider ABI](https://github.com/Infiltrator-Projects/Libraries/blob/main/docs/TEMPORAL_PROVIDERS.md)
