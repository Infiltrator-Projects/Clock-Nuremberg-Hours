// SPDX-License-Identifier: GPL-3.0-or-later
/* Seasonal clock resources. Sole owner of clock-face arithmetic and scheduling. */

#define CLOCK_ID "nuremberg-hours"
#define CLOCK_SECONDS true
#define CLOCK_LOCATION false
#include <infiltratr/temporal_clock_resource.h>
#include "temporal_solar_compact.h"

#include "infiltratr/arithmetic.h"
#include "infiltratr/core.h"
#include "infiltratr/i18n.h"
#include "infiltratr/timing.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#define MICROSECONDS_PER_SECOND INT64_C(1000000)
#define SECONDS_PER_MINUTE INT64_C(60)
#define SECONDS_PER_HOUR INT64_C(3600)
#define SECONDS_PER_DAY INT64_C(86400)
#define MICROSECONDS_PER_DAY (SECONDS_PER_DAY * MICROSECONDS_PER_SECOND)
#define INFILTRATR_PI 3.141592653589793238462643383279502884L

static bool write_text(char *buffer, size_t capacity, size_t *length,
                       const char *text)
{
    size_t needed;

    if (buffer == NULL || capacity == 0U || text == NULL) {
        return false;
    }
    needed = strlen(text);
    if (needed >= capacity) {
        return false;
    }
    memcpy(buffer, text, needed + 1U);
    if (length != NULL) {
        *length = needed;
    }
    return true;
}

static bool finish_output(char *buffer,size_t capacity,size_t *length,int written)
{
    if (written < 0 || (size_t)written >= capacity) {
        if (buffer != NULL && capacity != 0U) buffer[0]='\0';
        return false;
    }
    if (length != NULL) *length=(size_t)written;
    return true;
}

static int64_t positive_modulo(int64_t value, int64_t modulus)
{
    int64_t remainder = 0;

    if (!infiltratr_i64_floor_divmod(value, modulus, NULL, &remainder)) {
        return 0;
    }
    return remainder;
}

static long double mean_solar_seconds(int64_t unix_microseconds,
                                      double longitude)
{
    const int64_t utc_microseconds =
        positive_modulo(unix_microseconds, MICROSECONDS_PER_DAY);
    long double seconds =
        (long double)utc_microseconds / MICROSECONDS_PER_SECOND +
        (long double)temporal_solar_clamp_longitude(longitude) * 240.0L;

    seconds = fmodl(seconds, (long double)SECONDS_PER_DAY);
    if (seconds < 0.0L) {
        seconds += (long double)SECONDS_PER_DAY;
    }
    return seconds;
}

static long double apparent_solar_seconds(int64_t unix_microseconds,
                                          double longitude)
{
    double equation_minutes = 0.0;
    long double seconds;

    if (!temporal_solar_compact_terms(unix_microseconds, &equation_minutes, NULL)) {
        return mean_solar_seconds(unix_microseconds, longitude);
    }
    seconds = mean_solar_seconds(unix_microseconds, longitude) +
              (long double)equation_minutes * 60.0L;
    seconds = fmodl(seconds, (long double)SECONDS_PER_DAY);
    if (seconds < 0.0L) {
        seconds += (long double)SECONDS_PER_DAY;
    }
    return seconds;
}


typedef struct SeasonalPeriod {
    bool daylight;
    unsigned index;
    long double fraction;
} SeasonalPeriod;


static bool nuremberg_historical_hour_allocation(
    int64_t unix_microseconds,
    unsigned *day_hours,
    unsigned *night_hours,
    long double *day_start,
    long double *day_end)
{
    /*
     * Nürnberg's 1488 Wendetage were recorded in the Julian calendar.
     * Converting those dates to the proleptic Gregorian calendar of that
     * epoch adds nine days. The allocation therefore changes on the modern
     * seasonal equivalents below, not at every actual sunrise/sunset.
     */
    static const int thresholds[] = {
        16, 37, 54, 71, 87, 104, 122, 144,
        201, 223, 241, 257, 274, 290, 309, 329
    };
    static const unsigned allocations[] = {
        9U, 10U, 11U, 12U, 13U, 14U, 15U, 16U,
        15U, 14U, 13U, 12U, 11U, 10U, 9U, 8U
    };
    int day_of_year;
    bool leap = false;
    unsigned count = 8U;
    size_t index;

    if (day_hours == NULL || night_hours == NULL ||
        day_start == NULL || day_end == NULL ||
        !temporal_solar_day_of_year_with_leap(unix_microseconds, &day_of_year, &leap)) {
        return false;
    }

    for (index = 0U; index < INFILTRATR_ARRAY_LENGTH(thresholds); ++index) {
        int threshold = thresholds[index];
        if (leap && threshold >= 71) {
            threshold += 1;
        }
        if (day_of_year < threshold) {
            break;
        }
        count = allocations[index];
    }

    *day_hours = count;
    *night_hours = 24U - count;
    *day_start =
        ((long double)(24U - count) / 2.0L) * SECONDS_PER_HOUR;
    *day_end = *day_start + (long double)count * SECONDS_PER_HOUR;
    return true;
}

static bool format_nuremberg_hours(char *buffer,
                                   size_t capacity,
                                   size_t *length,
                                   int64_t unix_microseconds,
                                   bool show_seconds,
                                   bool vertical,
                                   bool historical,
                                   double latitude,
                                   double longitude)
{
    (void)historical; (void)latitude; (void)longitude;
    static const double nuremberg_longitude = 11.0767;
    unsigned day_hours;
    unsigned night_hours;
    long double day_start;
    long double day_end;
    const double clock_longitude = nuremberg_longitude;
    const long double solar =
        apparent_solar_seconds(unix_microseconds, clock_longitude);
    long double elapsed;
    int hour;
    int minute;
    int second;
    const char *suffix;
    const char *separator = vertical ? "\n" : ":";
    bool ok;

    ok = nuremberg_historical_hour_allocation(
            unix_microseconds, &day_hours, &night_hours,
            &day_start, &day_end);
    if (!ok) {
        return write_text(buffer, capacity, length,
                          vertical ? "N/A\nNUR" : "N/A NUR");
    }

    if (solar >= day_start && solar < day_end) {
        elapsed = solar - day_start;
        suffix = "NUR-D";
    } else {
        elapsed = solar >= day_end
            ? solar - day_end
            : (long double)SECONDS_PER_DAY - day_end + solar;
        suffix = "NUR-N";
    }
    (void)day_hours;
    (void)night_hours;

    hour = (int)floorl(elapsed / SECONDS_PER_HOUR);
    minute = (int)floorl(
        fmodl(elapsed, (long double)SECONDS_PER_HOUR) /
        SECONDS_PER_MINUTE);
    second = (int)floorl(
        fmodl(elapsed, (long double)SECONDS_PER_MINUTE));

    if (show_seconds) {
        return finish_output(buffer, capacity, length,
            snprintf(buffer, capacity, "%02d%s%02d%s%02d%s%s", hour, separator, minute, separator, second, vertical ? "\n" : " ", suffix));
    }
    return finish_output(buffer, capacity, length,
            snprintf(buffer, capacity, "%02d%s%02d%s%s", hour, separator, minute, vertical ? "\n" : " ", suffix));
}

static bool mode_location_valid(const InfiltratrTemporalClockModeInfo *info,
                                bool location_configured,
                                double latitude,
                                double longitude)
{
    if (info == NULL) {
        return false;
    }
    if ((info->requires_latitude || info->requires_longitude) &&
        !location_configured) {
        return false;
    }
    return (!info->requires_latitude ||
            (isfinite(latitude) && latitude >= -90.0 && latitude <= 90.0)) &&
           (!info->requires_longitude ||
            (isfinite(longitude) && longitude >= -180.0 &&
             longitude <= 180.0));
}


/* Names are transliterated proper names, not animal/help labels. Wording
 * belongs to the selected language; the shared phase calculation stays below. */
#define CLOCK_REMAINING_WORDS(hour, watch, twelfth, twelfths) \
    { "hour", hour }, { "watch", watch }, \
    { "twelfth", twelfth }, { "twelfths", twelfths }
#define JAPANESE_BRANCHES \
    { "jp-ne", "Ne" }, { "jp-ushi", "Ushi" }, { "jp-tora", "Tora" }, \
    { "jp-u", "U" }, { "jp-tatsu", "Tatsu" }, { "jp-mi", "Mi" }, \
    { "jp-uma", "Uma" }, { "jp-hitsuji", "Hitsuji" }, { "jp-saru", "Saru" }, \
    { "jp-tori", "Tori" }, { "jp-inu", "Inu" }, { "jp-i", "I" }
#define JAPANESE_NATIVE_BRANCHES \
    { "jp-ne", "子" }, { "jp-ushi", "丑" }, { "jp-tora", "寅" }, \
    { "jp-u", "卯" }, { "jp-tatsu", "辰" }, { "jp-mi", "巳" }, \
    { "jp-uma", "午" }, { "jp-hitsuji", "未" }, { "jp-saru", "申" }, \
    { "jp-tori", "酉" }, { "jp-inu", "戌" }, { "jp-i", "亥" }
#define EDO_WORDS(horizontal, vertical) \
    { "edo", horizontal }, { "edo-vertical", vertical }


#undef CLOCK_REMAINING_WORDS
#undef JAPANESE_BRANCHES
#undef JAPANESE_NATIVE_BRANCHES
#undef EDO_WORDS

#define CLOCK_CATALOG(locale, entries) \
    { locale, entries, sizeof(entries) / sizeof((entries)[0]) }

#undef CLOCK_CATALOG


static bool format_clock_mode(const InfiltratrI18n *language,
                                           const char *mode,
                                           int64_t unix_microseconds,
                                           int32_t utc_offset_seconds,
                                           bool show_seconds,
                                           bool vertical,
                                           bool location_configured,
                                           double latitude,
                                           double longitude,
                                           char *buffer,
                                           size_t capacity,
                                           size_t *length)
{
    (void)language; (void)mode;
    const InfiltratrTemporalClockModeInfo *info;

    // The extension modes use absolute instants and, when applicable, the
    // selected solar location. Civil UTC offsets remain part of the stable
    // caller contract, but do not affect this custom-mode renderer.
    (void)utc_offset_seconds;
    if (buffer == NULL || capacity == 0U || mode == NULL) {
        return false;
    }
    buffer[0] = '\0';

    const InfiltratrTemporalClockModeInfo selected = {
        CLOCK_ID, INFILTRATR_TEMPORAL_DISPLAY_NAME, CLOCK_SECONDS, CLOCK_LOCATION, CLOCK_LOCATION
    };
    info = strcmp(mode, CLOCK_ID) == 0 ? &selected : NULL;
    if (info == NULL ||
        !mode_location_valid(
            info, location_configured, latitude, longitude)) {
        return false;
    }

    ;

    ;

    {
        return format_nuremberg_hours(
            buffer, capacity, length, unix_microseconds,
            show_seconds, vertical, true, 0.0, 0.0);
    }

    ;

    return false;
}

static bool seasonal_format_clock_mode(const char *mode,
    int64_t unix_microseconds, int32_t utc_offset_seconds,
    bool show_seconds, bool vertical, bool location_configured,
    double latitude, double longitude,
    char *buffer, size_t capacity, size_t *length)
{
    (void)mode;
    return format_clock_mode(NULL, mode, unix_microseconds, utc_offset_seconds,
        show_seconds, vertical, location_configured, latitude, longitude,
        buffer, capacity, length);
}


static bool render(const InfiltratrTemporalClockProvider *self,
    const char *locale, int64_t instant, int32_t offset, bool seconds,
    bool vertical, double latitude, double longitude,
    char *buffer, size_t capacity, size_t *length)
{
    (void)self; (void)locale;
    return seasonal_format_clock_mode(CLOCK_ID, instant, offset, seconds,
        vertical, true, latitude, longitude, buffer, capacity, length);
}

/* Bracket and refine the first visible boundary against this very renderer.
 * Rendering and scheduling cannot disagree over moving seasonal boundaries,
 * Wendetage or floating-point quantisation. This is a bounded one-shot search.
 */
static bool next_tick(const InfiltratrTemporalClockProvider *self,
    int64_t instant, int32_t offset, bool seconds,
    double latitude, double longitude, uint32_t *milliseconds)
{
    char initial[256], future[256];
    uint32_t low = 0U, high = 1U, step = 1U;
    const uint32_t maximum = 129600000U;
    (void)self;
    if (milliseconds == NULL || !render(NULL, NULL, instant, offset, seconds,
            false, latitude, longitude, initial, sizeof(initial), NULL))
        return false;
    /* Solar events are undefined in polar N/A periods. Retry without
     * searching 36 hours of identical unavailable clock faces. */
    if (strncmp(initial, "N/A", 3U) == 0) {
        *milliseconds = 60000U;
        return true;
    }
    for (;;) {
        const int64_t delta = (int64_t)high * 1000;
        if (instant > INT64_MAX - delta ||
            !render(NULL, NULL, instant + delta, offset, seconds, false,
                latitude, longitude, future, sizeof(future), NULL))
            return false;
        if (strcmp(initial, future) != 0) break;
        if (high == maximum)
            return false; /* A valid clock must actually change. */
        low = high;
        if (step < 60000U) step = step > 30000U ? 60000U : step * 2U;
        high = maximum - high < step ? maximum : high + step;
    }
    while (high - low > 1U) {
        const uint32_t middle = low + (high - low) / 2U;
        if (!render(NULL, NULL, instant + (int64_t)middle * 1000, offset,
                seconds, false, latitude, longitude, future, sizeof(future), NULL))
            return false;
        if (strcmp(initial, future) == 0) low = middle;
        else high = middle;
    }
    *milliseconds = high;
    return true;
}

static const InfiltratrTemporalResourceMetadata metadata = {
    sizeof(InfiltratrTemporalResourceMetadata),
    INFILTRATR_TEMPORAL_RESOURCE_METADATA_ABI,
    INFILTRATR_TEMPORAL_RESOURCE_CLOCK, CLOCK_ID, INFILTRATR_TEMPORAL_DISPLAY_NAME,
    CLOCK_SECONDS, CLOCK_LOCATION, CLOCK_LOCATION
};
static const InfiltratrTemporalClockProvider provider = {
    sizeof(InfiltratrTemporalClockProvider),
    INFILTRATR_TEMPORAL_CLOCK_PROVIDER_ABI, CLOCK_ID, render, next_tick
};
#if defined(_WIN32)
#define RESOURCE_EXPORT __declspec(dllexport)
#else
#define RESOURCE_EXPORT __attribute__((visibility("default")))
#endif
RESOURCE_EXPORT const InfiltratrTemporalResourceMetadata *
infiltratr_temporal_resource_metadata_v1(void);
RESOURCE_EXPORT const InfiltratrTemporalClockProvider *
infiltratr_temporal_clock_provider_v1(void);
RESOURCE_EXPORT const InfiltratrTemporalResourceMetadata *
infiltratr_temporal_resource_metadata_v1(void) { return &metadata; }
RESOURCE_EXPORT const InfiltratrTemporalClockProvider *
infiltratr_temporal_clock_provider_v1(void) { return &provider; }
