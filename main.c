#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <Windows.h>

#include <al/al.h>
#include <al/alc.h>
#include <al/alut.h>

/*
 * Debugging, extra features, run-time user manipulations of OpenAL, etc.
 */
#include "stuff.h"

const char* AL_errors[6] = {
    "AL_NO_ERROR", /* There is no current error. */
    "AL_INVALID_NAME", /* Invalid name parameter. */
    "AL_INVALID_ENUM", /* Invalid parameter. */
    "AL_INVALID_VALUE", /* Invalid enum parameter value. */
    "AL_INVALID_OPERATION", /* Illegal call. */
    "AL_OUT_OF_MEMORY" /* Unable to allocate memory. */
};

#define AT_X    ( 0.0F)
#define AT_Y    ( 0.0F)
#define AT_Z    (-1.0F)
#define UP_X    ( 0.0F)
#define UP_Y    (+1.0F)
#define UP_Z    ( 0.0F)
ALboolean initialize_listener(void)
{
    ALenum ALstatus;
    const ALfloat ori[2*3] = { /* Listener orientation is really two vectors. */
        AT_X, AT_Y, AT_Z,
        UP_X, UP_Y, UP_Z };

    ALstatus = alGetError();
    if (ALstatus != AL_NO_ERROR)
        printf("alListener\n%s\n\n", AL_errors[ALstatus]);

/*
 * Demonstrate the common attributes by starting with OpenAL 1.1 defaults.
 * These three parameters are also valid for dynamic source objects.
 */
    alListener3f(AL_POSITION, 0.0f, 0.0f, 0.0f);
    alListener3f(AL_VELOCITY, 0.0f, 0.0f, 0.0f);
    alListenerf(AL_GAIN, 1.0F); /* This overrides alSourcef(src, AL_GAIN...). */
    ALstatus = alGetError();
    if (ALstatus != AL_NO_ERROR)
    {
        printf("alListener\n%s\n\n", AL_errors[ALstatus]);
        return AL_FALSE;
    }

/*
 * The following is specific exactly to the listener.
 * You cannot set this for sources.  (The sound card's DSP should do a NOP.)
 */
    alListenerfv(AL_ORIENTATION, ori);
    ALstatus = alGetError();
    if (ALstatus != AL_NO_ERROR)
    {
        printf("alListener\n%s\n\n", AL_errors[ALstatus]);
        return AL_FALSE;
    }
    return AL_TRUE;
}

ALboolean initialize_buffer(void)
{
    ALenum ALstatus;
    ALenum format;
    ALsizei size;
    ALvoid* data;
    ALsizei freq;
    ALboolean loop;

    ALstatus = alGetError();
    if (ALstatus != AL_NO_ERROR)
        printf("alBuffer\n%s\n\n", AL_errors[ALstatus]);

    alGenBuffers(NUM_BUFFERS, &buffer);
    ALstatus = alGetError();
    if (ALstatus != AL_NO_ERROR)
    {
        printf("alBuffer\n%s\n\n", AL_errors[ALstatus]);
        return AL_FALSE;
    }
    if (alIsBuffer(buffer) == AL_FALSE)
    {
        printf("Failed to validate buffer.\n");
        return AL_FALSE;
    }

    alutLoadWAVFile((ALbyte *)"test.wav", &format, &data, &size, &freq, &loop);
    alBufferData(buffer, format, data, size, freq);
    alutUnloadWAV(format, data, size, freq);

    alSourcei(source, AL_BUFFER, buffer);
    alSourcei(source, AL_LOOPING, AL_TRUE);
    alSourcef(source, AL_PITCH, 1.0F);
    ALstatus = alGetError();
    if (ALstatus != AL_NO_ERROR)
    {
        printf("alBuffer\n%s\n\n", AL_errors[ALstatus]);
        return AL_FALSE;
    }
    return AL_TRUE;
}

ALboolean initialize_source(void)
{
    ALenum ALstatus;
    ALint query;

    ALstatus = alGetError(); /* Each error check zeroes error status. */
    if (ALstatus != AL_NO_ERROR)
        printf("Warning:  Request initialized since a prior error.\n");

    alGenSources(NUM_SOURCES, &source);
    ALstatus = alGetError();
    if (ALstatus != AL_NO_ERROR)
    {
        printf("alSource\n%s\n\n", AL_errors[ALstatus]);
        return AL_FALSE;
    }
    if (alIsSource(source) == AL_FALSE)
    {
        printf("Failed to validate source.\n");
        return AL_FALSE;
    }
    alSource3f(source, AL_POSITION, 0.0F, 0.0F, 0.0F);
    alSource3f(source, AL_VELOCITY, 0.0F, 0.0F, 0.0F);
    alSourcef(source, AL_GAIN, 1.0F);

/*
 * The following are specific to the sources only.
 * You cannot apply these attributes to the listener.
 */
    alSourcei(source, AL_SOURCE_RELATIVE, AL_FALSE);
    alGetSourcei(source, AL_SOURCE_TYPE, &query); /* READ-ONLY */
    alSourcei(source, AL_LOOPING, AL_FALSE);
    alSourcei(source, AL_BUFFER, AL_NONE);
    alGetSourcei(source, AL_BUFFERS_QUEUED, &query); /* READ-ONLY */
    alGetSourcei(source, AL_BUFFERS_PROCESSED, &query); /* READ-ONLY */
    alSourcef(source, AL_MIN_GAIN, 0.0F);
    alSourcef(source, AL_MAX_GAIN, 1.0F);
    alSourcef(source, AL_REFERENCE_DISTANCE, 1.0F);
    alSourcef(source, AL_ROLLOFF_FACTOR, 1.0F);
 /* alSourcei(source, AL_MAX_DISTANCE, MAX_FLOAT); */
    alSourcef(source, AL_PITCH, 1.0F);
    alSource3f(source, AL_DIRECTION, 0.0F, 0.0F, 0.0F);
    alSourcef(source, AL_CONE_INNER_ANGLE, +360.0F);
    alSourcef(source, AL_CONE_OUTER_ANGLE, +360.0F);
    alSourcef(source, AL_SEC_OFFSET, 0.0F);
    alSourcef(source, AL_SAMPLE_OFFSET, 0.0F);
    alSourcef(source, AL_BYTE_OFFSET, 0.0F);
    ALstatus = alGetError();
    if (ALstatus != AL_NO_ERROR)
    {
        printf("alSource\n%s\n\n", AL_errors[ALstatus]);
        return AL_FALSE;
    }
    return AL_TRUE;
}

int main(void)
{
    ALboolean success;
    ALCboolean passed;
    ALCdevice* device;
    ALCcontext* context;

    device = init_AL_device();
    context = alcCreateContext(device, NULL);
    if (context == NULL)
    {
        printf("Failed to initialize AL.\n");
        return 0;
    }
    passed = alcMakeContextCurrent(context);
    if (passed == ALC_FALSE)
    {
        printf("Failed to attach DC.\n");
        return 0;
    }

    log_AL_states();
    success = initialize_listener() & initialize_buffer() & initialize_source();
    if (success == AL_FALSE)
    {
        printf("Fatal error.  Stopping.\n\n");
        return 0;
    }
    alSourceQueueBuffers(source, NUM_BUFFERS, &buffer);
    log_buffer_attributes();

    printf(
        "OpenAL test keys:  \n"\
        "\n"\
        "P) alSourcePlay\n"\
        "H) alSourcePause (hold)\n"\
        "S) alSourceStop\n"\
        "R) alSourceRewind\n"\
        "V) Re-define the volume coefficient AL_GAIN scale.\n"\
        "Q) Frees RAM, releases the AL context, and quits\n\n");

    do
    {
        ALint query;

        alGetSourcei(source, AL_SOURCE_STATE, &query);
#if (0)
        DEBUG_SOURCE_STATE(query);
#endif
        switch (getchar() & ~0x20) /* lowercase-to-uppercase conversion */
        {
            case 'P':
                if (query == AL_INITIAL)
                    printf("from %s to %s\n", "AL_INITIAL", "AL_PLAYING");
            /* For AL_PAUSED, OpenAL 1.1 spec RECALLS, not forces, the state. */
                if (query == AL_STOPPED)
                    printf("from %s to %s to %s\n", "AL_STOPPED",
                        "AL_INITIAL", "AL_PLAYING");
                alSourcePlay(source);
                continue;
            case 'H':
                if (query == AL_PLAYING)
                    printf("from %s to %s\n", "AL_PLAYING", "AL_PAUSED");
                else
                    printf("NOP\n"); /* no errors, action or state change */
                alSourcePause(source);
                continue;
            case 'S':
                if (query == AL_INITIAL || query == AL_STOPPED)
                    printf("NOP\n"); /* no errors, action or state change */
                if (query == AL_PLAYING)
                    printf("from %s to %s\n", "AL_PLAYING", "AL_STOPPED");
                if (query == AL_PAUSED)
                    printf("from %s to %s\n", "AL_PAUSED", "AL_STOPPED");
                alSourceStop(source);
                continue;
            case 'R':
                if (query == AL_INITIAL)
                    printf("NOP\n");
                if (query == AL_PLAYING)
                    printf("from %s to %s to %s\n", "AL_PLAYING",
                        "AL_STOPPED", "AL_INITIAL");
                if (query == AL_PAUSED)
                    printf("from %s to %s\n", "AL_PAUSED", "AL_INITIAL");
                if (query == AL_STOPPED)
                    printf("from %s to %s\n", "AL_STOPPED", "AL_INITIAL");
                alSourceRewind(source);
                continue;
            case 'V': {
                ALfloat gain;

                printf("Enter new volume scale:  ");
                scanf("%f", &gain);
                change_volume(gain);
                continue; }
            case 'Q':
                goto EXIT;
        };
    } while (success == success);
EXIT:
    alSourceUnqueueBuffers(source, NUM_BUFFERS, &buffer);
    passed = finish_AL_context();
    if (passed == ALC_FALSE)
        printf("Failed to invalidate current ALC.\n");
    return 0;
}
