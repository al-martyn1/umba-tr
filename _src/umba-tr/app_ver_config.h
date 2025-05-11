std::string appFullName   = "Umba TR Tool";
std::string appVersion    = "1.0";
std::string appCommitHash;  //UNDONE
std::string appBuildDate  = __DATE__;
std::string appBuildTime  = __TIME__;

const char *appHomeUrl    = "https://github.com/al-martyn1/umba-tr";
const char *appistrPath   = "";

#if defined(WIN32) || defined(_WIN32)
    const char *appSubPath    = "bin/umba-tr.exe";
#else
    const char *appSubPath    = "bin/umba-tr";
#endif

