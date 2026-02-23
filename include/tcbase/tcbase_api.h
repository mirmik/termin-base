#ifndef TCBASE_API_H
#define TCBASE_API_H

#ifdef _WIN32
    #ifdef TCBASE_EXPORTS
        #define TCBASE_API __declspec(dllexport)
    #else
        #define TCBASE_API __declspec(dllimport)
    #endif
#else
    #define TCBASE_API
#endif

#endif // TCBASE_API_H
