// sound.c

#include <3ds.h>
#include <stdio.h>
#include <string.h>
#include "sound.h"

typedef struct {
    s16 *sample;
    ndspWaveBuf wavebuf;
} Sfx;

static Sfx sounds[SNDCOUNT];
static bool initialized;

bool sound_load(Sound id, const char *soundname) {
    if (!initialized) {
    	return false;
    }
    char filename[128];
    snprintf(filename, sizeof(filename), "romfs:/sounds/%s.raw", soundname);
    FILE *f = fopen(filename, "rb");
    if (!f) {
        printf("File not found: %s\n", filename);
        return false;
    }

    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return false;
    }

    long end = ftell(f);
    if (end <= 0 || end % sizeof(s16) != 0) {
        printf("Invalid sound file: %s\n", filename);
        fclose(f);
        return false;
    }

    size_t size = (size_t)end;
    rewind(f);

    s16 *sample = linearAlloc(size);
    if (!sample) {
        fclose(f);
        return false;
    }

    if (fread(sample, 1, size, f) != size) {
        fclose(f);
        linearFree(sample);
        return false;
    }
    fclose(f);

    DSP_FlushDataCache(sample, size);

    Sfx *sfx = &sounds[id];

    ndspChnReset(id);
    if (sfx->sample) linearFree(sfx->sample);

    memset(&sfx->wavebuf, 0, sizeof(sfx->wavebuf));
    sfx->sample = sample;
    sfx->wavebuf.data_pcm16 = sample;
    sfx->wavebuf.nsamples = size / sizeof(s16);
    sfx->wavebuf.looping = false;

    return true;
}

bool sound_play(Sound id) {
    if (!initialized) {
    	return false;
    }

    if (!sounds[id].sample) {
    	return false;
    }

    ndspChnReset(id);
    ndspChnSetInterp(id, NDSP_INTERP_LINEAR);
    ndspChnSetRate(id, 22050.0f);
    ndspChnSetFormat(id, NDSP_FORMAT_MONO_PCM16);

    float mix[12] = {0};
    mix[0] = 1.0f;
    mix[1] = 1.0f;

    ndspChnSetMix(id, mix);
    ndspChnWaveBufAdd(id, &sounds[id].wavebuf);

    return true;
}

void sound_init(void) {
	if (R_FAILED(ndspInit())) {
		printf("Cannot initialize audio (missing dspfirm.cdc?)");
		return;
	}
	initialized = true;
}

void sound_reset(void) {
	if (!initialized) {
		return;
	}
    for (int i = 0; i < SNDCOUNT; i++) {
        ndspChnReset(i);
        if (sounds[i].sample) {
        	linearFree(sounds[i].sample);
        }
        memset(&sounds[i], 0, sizeof(sounds[i]));
    }
}

void sound_close(void) {
    if (!initialized) {
    	return;
    }
    sound_reset();
    ndspExit();
    initialized = false;
}