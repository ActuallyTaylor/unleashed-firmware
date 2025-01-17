#include "dolphin_state.h"
#include "dolphin/helpers/dolphin_deed.h"
#include "dolphin_state_filename.h"

#include <stdint.h>
#include <storage/storage.h>
#include <furi.h>
#include <furi_hal.h>
#include <math.h>
#include <toolbox/saved_struct.h>

#define TAG "DolphinState"

#define DOLPHIN_STATE_PATH INT_PATH(DOLPHIN_STATE_FILE_NAME)
#define DOLPHIN_STATE_HEADER_MAGIC 0xD0
#define DOLPHIN_STATE_HEADER_VERSION 0x01
#define BUTTHURT_MAX 14
#define BUTTHURT_MIN 0


const uint32_t DOLPHIN_LEVELS[] = {0,      300,    900,    2700,   6500,   14000,  23000,
                                   34000,  48000,  64000,  85000,  100000, 120000, 140000,
                                   165000, 195000, 225000, 265000, 305000, 355000};
const size_t DOLPHIN_LEVEL_COUNT = COUNT_OF(DOLPHIN_LEVELS);

DolphinState* dolphin_state_alloc() {
    return malloc(sizeof(DolphinState));
}

void dolphin_state_free(DolphinState* dolphin_state) {
    free(dolphin_state);
}

bool dolphin_state_save(DolphinState* dolphin_state) {
    if(!dolphin_state->dirty) {
        return true;
    }

    bool result = saved_struct_save(
        DOLPHIN_STATE_PATH,
        &dolphin_state->data,
        sizeof(DolphinStoreData),
        DOLPHIN_STATE_HEADER_MAGIC,
        DOLPHIN_STATE_HEADER_VERSION);

    if(result) {
        FURI_LOG_I(TAG, "State saved");
        dolphin_state->dirty = false;
    } else {
        FURI_LOG_E(TAG, "Failed to save state");
    }

    return result;
}

bool dolphin_state_load(DolphinState* dolphin_state) {
    bool success = saved_struct_load(
        DOLPHIN_STATE_PATH,
        &dolphin_state->data,
        sizeof(DolphinStoreData),
        DOLPHIN_STATE_HEADER_MAGIC,
        DOLPHIN_STATE_HEADER_VERSION);

    if(success) {
        if((dolphin_state->data.butthurt > BUTTHURT_MAX) ||
           (dolphin_state->data.butthurt < BUTTHURT_MIN)) {
            success = false;
        }
    }

    if(!success) {
        FURI_LOG_W(TAG, "Reset dolphin-state");
        memset(dolphin_state, 0, sizeof(*dolphin_state));
        dolphin_state->dirty = true;
    }

    return success;
}

uint64_t dolphin_state_timestamp() {
    DateTime datetime;
    furi_hal_rtc_get_datetime(&datetime);
    return datetime_datetime_to_timestamp(&datetime);
}

bool dolphin_state_is_levelup(uint32_t icounter) {
    for(size_t i = 0; i < DOLPHIN_LEVEL_COUNT; i++) {
        if(icounter == DOLPHIN_LEVELS[i]) {
            return true;
        }
    }

    return false;
}

uint8_t dolphin_get_level(uint32_t icounter) {
    for(size_t i = 0; i < DOLPHIN_LEVEL_COUNT; i++) {
        if(icounter <= DOLPHIN_LEVELS[i]) {
            return i + 1;
        }
    }

    return DOLPHIN_LEVEL_COUNT + 1;
}

uint32_t dolphin_state_xp_above_last_levelup(uint32_t icounter) {
    uint8_t level = dolphin_get_level(icounter) - 1;

    if(level > 0) {
        return icounter - DOLPHIN_LEVELS[level - 1];
    }

    return icounter;
}

uint32_t dolphin_state_xp_to_levelup(uint32_t icounter) {
    uint8_t level = dolphin_get_level(icounter) - 1;

    if(level < DOLPHIN_LEVEL_COUNT) {
        return DOLPHIN_LEVELS[level] - icounter;
    }

    return -1;
}

void dolphin_state_on_deed(DolphinState* dolphin_state, DolphinDeed deed) {
    // Special case for testing
    if(deed > DolphinDeedMAX) {
        if(deed == DolphinDeedTestLeft) {
            dolphin_state->data.butthurt =
                CLAMP(dolphin_state->data.butthurt + 1, BUTTHURT_MAX, BUTTHURT_MIN);
            if(dolphin_state->data.icounter > 0) dolphin_state->data.icounter--;
            dolphin_state->data.timestamp = dolphin_state_timestamp();
            dolphin_state->dirty = true;
        } else if(deed == DolphinDeedTestRight) {
            dolphin_state->data.butthurt = BUTTHURT_MIN;
            if(dolphin_state->data.icounter < UINT32_MAX) dolphin_state->data.icounter++;
            dolphin_state->data.timestamp = dolphin_state_timestamp();
            dolphin_state->dirty = true;
        }
        return;
    }

    DolphinApp app = dolphin_deed_get_app(deed);
    uint8_t level = dolphin_get_level(dolphin_state->data.icounter);
    int8_t weight_limit =
        dolphin_deed_get_app_limit(app, level) - dolphin_state->data.icounter_daily_limit[app];
    uint8_t deed_weight = CLAMP(dolphin_deed_get_weight(deed, level), weight_limit, 0);

    uint32_t xp_to_levelup = dolphin_state_xp_to_levelup(dolphin_state->data.icounter);
    if(xp_to_levelup) {
        deed_weight = MIN(xp_to_levelup, deed_weight);
        dolphin_state->data.icounter += deed_weight;
        dolphin_state->data.icounter_daily_limit[app] += deed_weight;
    }

    /* decrease butthurt:
     * 0 deeds accumulating --> 0 butthurt
     * +1....+15 deeds accumulating --> -1 butthurt
     * +16...+30 deeds accumulating --> -1 butthurt
     * +31...+45 deeds accumulating --> -1 butthurt
     * +46...... deeds accumulating --> -1 butthurt
     * -4 butthurt per day is maximum
     * */
    uint8_t butthurt_icounter_level_old = dolphin_state->data.butthurt_daily_limit / 15 +
                                          !!(dolphin_state->data.butthurt_daily_limit % 15);
    dolphin_state->data.butthurt_daily_limit =
        CLAMP(dolphin_state->data.butthurt_daily_limit + deed_weight, 46, 0);
    uint8_t butthurt_icounter_level_new = dolphin_state->data.butthurt_daily_limit / 15 +
                                          !!(dolphin_state->data.butthurt_daily_limit % 15);
    int32_t new_butthurt = ((int32_t)dolphin_state->data.butthurt) -
                           (butthurt_icounter_level_old != butthurt_icounter_level_new);
    new_butthurt = CLAMP(new_butthurt, BUTTHURT_MAX, BUTTHURT_MIN);

    dolphin_state->data.butthurt = new_butthurt;
    dolphin_state->data.timestamp = dolphin_state_timestamp();
    dolphin_state->dirty = true;

    dolphin_state_save(dolphin_state);

    FURI_LOG_D(
        TAG,
        "icounter %lu, butthurt %ld",
        dolphin_state->data.icounter,
        dolphin_state->data.butthurt);
}

void dolphin_state_butthurted(DolphinState* dolphin_state) {
    if(dolphin_state->data.butthurt < BUTTHURT_MAX) {
        dolphin_state->data.butthurt++;
        dolphin_state->data.timestamp = dolphin_state_timestamp();
        dolphin_state->dirty = true;
    }
}

void dolphin_state_increase_level(DolphinState* dolphin_state) {
    furi_assert(dolphin_state_is_levelup(dolphin_state->data.icounter));
    ++dolphin_state->data.icounter;
    dolphin_state->dirty = true;
}

void dolphin_state_clear_limits(DolphinState* dolphin_state) {
    furi_assert(dolphin_state);

    for(int i = 0; i < DolphinAppMAX; ++i) {
        dolphin_state->data.icounter_daily_limit[i] = 0;
    }
    dolphin_state->data.butthurt_daily_limit = 0;
    dolphin_state->dirty = true;
}
