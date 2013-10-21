#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <Windows.h>

#include <al/al.h>
#include <al/alc.h>
#include <al/alut.h>

const char* AL_errors[6] = {
    "AL_NO_ERROR", /* There is no current error. */
    "AL_INVALID_NAME", /* Invalid name parameter. */
    "AL_INVALID_ENUM", /* Invalid parameter. */
    "AL_INVALID_VALUE", /* Invalid enum parameter value. */
    "AL_INVALID_OPERATION", /* Illegal call. */
    "AL_OUT_OF_MEMORY" /* Unable to allocate memory. */
};
const char* AL_source_states[4] = {
    "AL_INITIAL",
    "AL_PLAYING",
    "AL_PAUSED",
    "AL_STOPPED"
};

void DEBUG_SOURCE_STATE(ALint query)
{
/*
 * Convert literal source state macros into zero-based enumerations.
 * The next formula SHOULD always hold true for OpenAL source state macros.
 *
 * #define AL_SOURCE_STATE     0x1010
 * #define AL_INITIAL          0x1011
 * #define AL_PLAYING          0x1012
 * #define AL_PAUSED           0x1013
 * #define AL_STOPPED          0x1014
 */
    query -= AL_SOURCE_STATE + 0x0001;

    if (query & ~0x0003) /* hopefully impossible... */
    {
        printf("Invalid AL source state range!\n");
        query &= 0x0003; /* prevents string enum. memory access violation */
    }
    printf("AL_SOURCE_STATE:  %s\n", AL_source_states[query]);
    return;
}

void log_AL_states(void)
{
    FILE* out;
    register int i, j;
    const ALchar* vendor     = alGetString(AL_VENDOR);
    const ALchar* version    = alGetString(AL_VERSION);
    const ALchar* renderer   = alGetString(AL_RENDERER);
    const ALchar* extensions = alGetString(AL_EXTENSIONS);

    out = fopen("ALSTATES.TXT", "w");
    fprintf(out, "Global OpenAL Context States\n");
    fprintf(out, "\n");
    fprintf(out, "AL_VENDOR    :  %s\n", vendor);
    fprintf(out, "AL_VERSION   :  %s\n", version);
    fprintf(out, "AL_RENDERER  :  %s\n", renderer);
    fprintf(out, "AL_EXTENSIONS:\n");
/* ... The problem with this:  It is a list, space-separated. */
    for (i = 0; ; i++)
    {
        char line[80] = "  * "; /* list bulleting */

        j = 4;
        while (extensions[i] != ' ' && extensions[i] != '\0')
            line[j++] = extensions[i++];
        line[j] = '\n';
        fprintf(out, line);
        if (extensions[i] == '\0')
            break;
    }
    fprintf(out, "\n");
    fprintf(out, "AL_DOPPLER_FACTOR:  %f\n", alGetFloat(AL_DOPPLER_FACTOR));
    fprintf(out, "AL_SPEED_OF_SOUND:  %f\n", alGetFloat(AL_SPEED_OF_SOUND));
    fprintf(out, "AL_DISTANCE_MODEL:  %i\n", alGetInteger(AL_DISTANCE_MODEL));
    fclose(out);
    return;
}

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

ALuint buffer;
ALuint source;

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

    alGenBuffers(1, &buffer);
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

/*
 * Clear the OpenAL internal error flag.
 *
 * For performance reasons, OpenAL does not check if the last AL call failed.
 * It actually checks if *ANY* OpenAL API call failed since last error check.
 *
 * Therefore, we need to guarantee this is zeroed to begin with.
 */
    ALstatus = alGetError(); /* Each error check zeroes error status. */
    if (ALstatus != AL_NO_ERROR)
        printf("Warning:  Request initialized since a prior error.\n");

/*
 * The above attributes may also be set for a source.
 * Unlike the listener object, source objects are dynamic.  Allocate them.
 */
    alGenSources(1, &source);
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

    alGetError();
    success = alutInit(NULL, 0);
    if (success == AL_FALSE)
    {
        printf("Failed to init ALUT.\n");
        return 1;
    }

    log_AL_states();
    success = initialize_listener() & initialize_source() & initialize_buffer();
    if (success == AL_FALSE)
    {
        printf("Fatal error.  Stopping.\n\n");
        return 0;
    }
    alSourceQueueBuffers(source, 1, &buffer);

    printf(
        "OpenAL test keys:  \n"\
        "\n"\
        "P) alSourcePlay\n"\
        "H) alSourcePause (hold)\n"\
        "S) alSourceStop\n"\
        "R) alSourceRewind\n"\
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
            case 'Q':
                goto EXIT;
        };
    } while (success == success);
EXIT:
    alSourceUnqueueBuffers(source, 1, &buffer);
    alDeleteSources(1, &source);
    alDeleteBuffers(1, &buffer);
    alutExit();
    return 0;
}

/*
 * Dependency Walker hates the USER32::ADVAPI32::SHELL32 in late Windows.
 * This dummy function clears the crap up when analyzing dependencies.
 */
static SHELLEXECUTEINFO pExecInfo;
__declspec(dllexport) void __cdecl DllTest(HWND hParent)
{
    pExecInfo.cbSize       = sizeof(SHELLEXECUTEINFO);
    pExecInfo.hwnd         = hParent;
    pExecInfo.lpVerb       = "open";
    pExecInfo.lpFile       = "CMD"; /* command shell on Windows */
    pExecInfo.lpParameters = "dummy text";
    pExecInfo.nShow        = SW_SHOW;
    ShellExecuteEx(&pExecInfo);
    return;
}
