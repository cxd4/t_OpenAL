ALuint buffer;
ALuint source;

#define NUM_BUFFERS     1
#define NUM_SOURCES     1

const char* AL_source_states[4] = {
    "AL_INITIAL",
    "AL_PLAYING",
    "AL_PAUSED",
    "AL_STOPPED"
};

ALCdevice* init_AL_device(void)
{
    ALCdevice* device;

/*
 * Clear the OpenAL internal error flag.
 *
 * For performance reasons, OpenAL does not check if the last AL call failed.
 * It actually checks if *ANY* OpenAL API call failed since last error check.
 *
 * Therefore, we need to guarantee this is zeroed to begin with.
 */
    alGetError();
#if (0)
    success = alutInit(NULL, 0);
    if (success == AL_FALSE)
    {
        printf("Failed to init ALUT.\n");
        return NULL;
    }
#endif
    device = alcOpenDevice(NULL); /* open default device */
    if (device == NULL) /* Plug in some headphones or something! */
        printf("Unable to detect a sound device.\n");
    return (device);
}

ALCboolean finish_AL_context(void)
{
    ALCcontext* context;
    ALCdevice* device;
    ALboolean success;

    alDeleteSources(NUM_SOURCES, &source); /* can be deleted any time */
    alDeleteBuffers(NUM_BUFFERS, &buffer); /* cannot delete if attached */
#if (0)
    alutExit(); /* to-do:  Do we really need ALUT? */
#endif
    context = alcGetCurrentContext();
    device = alcGetContextsDevice(context);
    success  = alcMakeContextCurrent(NULL);
    alcDestroyContext(context);
    success &= alcCloseDevice(device);
    return (success);
}

void change_volume(ALfloat gain)
{
/*
 * Experimental volume changer.
 * OpenAL 1.1 specifications standardize the range 0.0 <= gain <= 1.0.
 * 0.0 is the way to mute and possibly even disable sound processing latency.
 * 1.0 could be the maximum volume ratio.  Some implementations allow higher.
 */
#if (0)
    alSourcef(source, AL_MIN_GAIN, 0.0F); /* can't have negative gains anyway */
    alSourcef(source, AL_MAX_GAIN, gain); /* uncertain ... allow gains > 1.0? */
#endif
    alListenerf(AL_GAIN, gain);
    alSourcef(source, AL_GAIN, gain); /* Pretty much overruled by alListener. */
    return;
}

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
    const ALCboolean all = alcIsExtensionPresent(NULL, "ALC_ENUMERATE_ALL_EXT");
    const ALchar* vendor     = alGetString(AL_VENDOR);
    const ALchar* version    = alGetString(AL_VERSION);
    const ALchar* renderer   = alGetString(AL_RENDERER);
    const ALchar* extensions = alGetString(AL_EXTENSIONS);
    const ALCchar* devices = alcGetString(NULL, ALC_DEVICE_SPECIFIER);
    const ALCchar* capture = alcGetString(NULL, ALC_CAPTURE_DEVICE_SPECIFIER);

    out = fopen("ALSTATES.TXT", "w");
    fprintf(out, "Global OpenAL Context States\n");
    fprintf(out, "\n");
    fprintf(out, "AL_VENDOR    :  %s\n", vendor);
    fprintf(out, "AL_VERSION   :  %s\n", version);
    fprintf(out, "AL_RENDERER  :  %s\n", renderer);
    fprintf(out, "AL_EXTENSIONS:\n");
/* ... The problem with this:  It is a list, space-separated. */
    for (i = 0; ; i++)
    { /* 80-byte MS-DOS text lines...might be too short for some output? :P */
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
    fprintf(out, "\n");
    fprintf(out, "ALC_DEFAULT_DEVICE_SPECIFIER:\n    \"%s\"\n",
        alcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER));
    fprintf(out, "ALC_CAPTURE_DEFAULT_DEVICE_SPECIFIER:\n    \"%s\"\n",
        alcGetString(NULL, ALC_CAPTURE_DEFAULT_DEVICE_SPECIFIER));

/*
 * Unlike the prior OpenAL 1.0 specification by Loki Software, the OpenAL 1.1
 * specification guarantees support for ALC_ENUMERATION_EXT.  This is not,
 * however, sufficient, to enumerate ALL possible sound devices in every case
 * that the newer ALC_ENUMERATE_ALL_EXT extension may encompass.
 */
    if (all == ALC_FALSE)
    {
        printf("To-do:  Revert to older enumeration model?\n");
        goto SKIP_ENUM_ALL_DEVICES;
    }

/*
 * enumerations mode
 * The following two tokens for alcGetString behave differently with a NULL
 * (unspecified) device pointer (for the first argument of the method call).
 *
 * The trick with printing these lists:
 *     1.  Each listed sound output device is separated by a null byte.
 *     2.  The end of the list of device name strings is marked by TWO nulls.
 */
    fprintf(out, "ALC_DEVICE_SPECIFIER (enumerations):\n");
    for (i = 0; devices[i] != '\0'; i++)
    {
        fputs("  * ", out); /* line bulleting */
        while (devices[i] != '\0')
            fputc(devices[i++], out);
        fputc('\n', out);
    }
    fprintf(out, "ALC_CAPTURE_DEVICE_SPECIFIER (enumerations):\n");
    for (i = 0; capture[i] != '\0'; i++)
    {
        fputs("  * ", out); /* line bulleting */
        while (capture[i] != '\0')
            fputc(capture[i++], out);
        fputc('\n', out);
    }
SKIP_ENUM_ALL_DEVICES:
    fclose(out);
    return;
}

void log_buffer_attributes(void)
{
    FILE *out;
    ALint query;

    out = fopen("BUFFERAT.TXT", "w");
    alGetBufferi(buffer, AL_FREQUENCY, &query);
    fprintf(out, "Period  :  %i samples per second\n", query); /* "Hertz" */
    alGetBufferi(buffer, AL_SIZE, &query);
    fprintf(out, "Raw size:  %i bytes\n", query);
    alGetBufferi(buffer, AL_BITS, &query); /* either 8 or 16 b/sample */
    fprintf(out, "Samples :  %i bits each\n", query);
    alGetBufferi(buffer, AL_CHANNELS, &query); /* either stereo or mono */
    fprintf(out, "Channels:  %i\n", query);
    fclose(out);
    return;
}

void setup_EAX_RAM(void)
{
    EAXSetBufferMode g_eaxSetMode;
    EAXGetBufferMode g_eaxGetMode;
    ALint iRAMSizeMB, iRAMFreeMB;
    ALenum g_eXRAMSize, g_eXRAMFree;
    ALenum g_eXRAMAuto, g_eXRAMHardware, g_eXRAMAccessible;
    const ALboolean have_XRAM = alIsExtensionPresent("EAX-RAM");

    if (have_XRAM == AL_FALSE)
        return;

    printf("Found X-RAM support on sound card.\n");
    g_eXRAMSize = alGetEnumValue("AL_EAX_RAM_SIZE");
    g_eXRAMFree = alGetEnumValue("AL_EAX_RAM_FREE");

    iRAMSizeMB = alGetInteger(g_eXRAMSize) / (1024*1024);
    iRAMFreeMB = alGetInteger(g_eXRAMFree) / (1024*1024);
    printf("Available X-RAM:  %i of %i MiB.\n", iRAMFreeMB, iRAMSizeMB);

    g_eXRAMAuto = alGetEnumValue("AL_STORAGE_AUTOMATIC");
    g_eXRAMHardware = alGetEnumValue("AL_STORAGE_HARDWARE");
    g_eXRAMAccessible = alGetEnumValue("AL_STORAGE_ACCESSIBLE");
    g_eaxGetMode = (EAXGetBufferMode)alGetProcAddress("EAXGetBufferMode");
    if (g_eaxGetMode == NULL)
        printf("Failed to detect extension:  %s.\n", "EAXGetBufferMode");
    else
    {
        const ALenum mode = g_eaxGetMode(buffer, NULL);

        if (mode == g_eXRAMAuto)
            printf("AL buffer was %s (%i).\n",
                "AL_STORAGE_AUTOMATIC", mode);
        else if (mode == g_eXRAMHardware)
            printf("AL buffer was %s (%i).\n",
                "AL_STORAGE_HARDWARE", mode);
        else if (mode == g_eXRAMAccessible)
            printf("AL buffer was %s (%i).\n",
                "AL_STORAGE_ACCESSIBLE", mode);
        else /* hopefully impossible? */
            printf("Unknown hardware buffering mode!\n%i\n", mode);
    }

    g_eaxSetMode = (EAXSetBufferMode)alGetProcAddress("EAXSetBufferMode");
    if (g_eaxSetMode == NULL)
        printf("Failed to detect extension:  %s.\n", "EAXSetBufferMode");
    else
    {
        ALboolean success;
     /* int use_XRAM; // user option whether to use XRAM? (to-do) */

/*
 * Use the most accessible RAM for N64 audio streaming.
 * Streaming is generally antagonistic for XRAM support, so on most
 * implementations, this will probably force CPU system memory for storage.
 */
        success = g_eaxSetMode(NUM_BUFFERS, &buffer, g_eXRAMAccessible);
        if (success == AL_FALSE)
        {
            printf("Error controlling buffer EAX!\n");
            return;
        }
/*
 * Force EAX-RAM audio memory for better playback quality (and hopefully
 * faster performance, but not necessarily for streaming N64 audio off DRAM).
 */
        success = g_eaxSetMode(NUM_BUFFERS, &buffer, g_eXRAMHardware);
        if (success == AL_FALSE)
            printf("Error setting buffer EAX!\n");
        else
            printf("EAX audio memory usage has now been enabled.\n");
    }
    return;
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
