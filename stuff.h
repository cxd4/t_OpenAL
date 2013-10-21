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
