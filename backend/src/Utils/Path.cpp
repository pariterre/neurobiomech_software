#define STIMWALKER_API_EXPORTS
#define _CRT_SECURE_NO_WARNINGS
#include "Utils/Path.h"

#include <fstream>
#include <regex>

#include "Utils/String.h"
#include "Utils/Error.h"

#ifdef _WIN32
    #include <direct.h>
    #include <Windows.h>
    #undef max
#else
    #include <sys/stat.h>
    #include <unistd.h>
#endif

using namespace STIMWALKER_NAMESPACE;

utils::Path::Path() :
    m_originalPath(utils::String()),
    m_folder(utils::String()),
    m_isFolderAbsolute(false),
    m_filename(utils::String()),
    m_extension(utils::String()){

}

utils::Path::Path(
    const char *path) :
    m_originalPath(path),
    m_folder(utils::String()),
    m_isFolderAbsolute(false),
    m_filename(utils::String()),
    m_extension(utils::String()){
    parseFileName(m_originalPath, m_folder, m_filename, m_extension);
    setIsFolderAbsolute();
}

utils::Path::Path(
    const utils::String &path) :
    m_originalPath(path),
    m_folder(utils::String()),
    m_isFolderAbsolute(std::make_shared<bool>()),
    m_filename(utils::String()),
    m_extension(utils::String()){
    parseFileName(m_originalPath, m_folder, m_filename, m_extension);
    setIsFolderAbsolute();
}

utils::Path::Path(
    const std::basic_string<char> &path) :
    m_originalPath(path),
    m_folder(utils::String()),
    m_isFolderAbsolute(std::make_shared<bool>()),
    m_filename(utils::String()),
    m_extension(utils::String()){
    parseFileName(m_originalPath, m_folder, m_filename, m_extension);
    setIsFolderAbsolute();
}

bool utils::Path::isFileExist() const {
    return isFileExist(absolutePath());
}

bool utils::Path::isFileExist(
    const Path& path){
    return isFileExist(path.absolutePath());
}

bool utils::Path::isFileExist(
    const utils::String& path){
#ifdef _WIN32
    const char* pathChar = toWindowsFormat(path).c_str();
#else
    const char* pathChar = path.c_str();
#endif
    
    if (FILE *file = fopen(pathChar, "r")) {
        fclose(file);
        return true;
    } else {
        return false;
    }
}

bool utils::Path::isFileReadable() const {
#ifdef _WIN32
    const char* pathChar = toWindowsFormat(absolutePath()).c_str();
#else
    const char* pathChar = absolutePath().c_str();
#endif

    std::ifstream fichier(pathChar);
    bool isOpen(fichier.is_open());
    fichier.close();
    return isOpen;
}

bool utils::Path::isFolderExist() const {
    return isFolderExist(*this);
}

bool utils::Path::isFolderExist(
    const Path &path){
    return isFolderExist(path.folder());
}

bool utils::Path::isFolderExist(
    const utils::String & path){
#ifdef _WIN32
    if (GetFileAttributesA(toWindowsFormat(path).c_str())
            == INVALID_FILE_ATTRIBUTES) {
        return false;    // Le path est invalide
    } else
        // Si on est ici, c'est que quelque chose existe,
        // s'assurer que ça ne soit pas un fichier
        if (isFileExist(path)) {
            return false;
        } else {
            return true;
        }
#else
    struct stat statbuf;
    if (stat(path.c_str(), &statbuf) != -1) {
        return true;
    } else {
        return false;
    }
#endif
}

void utils::Path::parseFileName(
    const utils::String &path,
    utils::String &folder,
    utils::String &filename,
    utils::String &extension){
    utils::String pathSep(toUnixFormat(path));

    size_t sepPos(pathSep.rfind("/"));

    // Stocker le folder
    if (sepPos == std::string::npos)
        // If no separator is found, then there is no separator
        // and therefore the path is ./
    {
        folder = "";
    } else {
        folder = pathSep.substr(0, sepPos) + "/";
    }

    if (folder.find("./") == 0) { // Remove the leading ./ if necessary
        folder = folder.substr(2);
    }

    // Stocker l'extension
    size_t ext(pathSep.rfind("."));
    if (ext != SIZE_MAX) {
        extension = pathSep.substr(ext+1);
    } else {
        extension = "";
    }

    // Stocker le nom de fichier
    filename = pathSep.substr(sepPos+1, ext- sepPos-1);
}

utils::String utils::Path::relativePath() const {
    return relativePath(*this, currentDir());
}

utils::String utils::Path::relativePath(
    const utils::String& relativeTo) const {
    return relativePath(*this, relativeTo);
}

utils::String utils::Path::relativePath(
    const utils::Path &path,
    const utils::String &relativeTo){
    utils::String me(path.absolutePath());
    utils::String currentDir(relativeTo);

    utils::String meFirstPart("");
    utils::String currentDirFirstPart("");

    // Set the separator to the 0 position
    size_t sepMe = std::string::npos;
    size_t sepCurrentDir = std::string::npos;
    do {
        // cut according to previous separator
        me = me.substr(sepMe+1);
        currentDir = currentDir.substr(sepCurrentDir+1);

        // Find the next separator
        sepMe = me.find("/"); // UNIX formalism
        sepCurrentDir = currentDir.find("/");

        // Separate first and last part
        meFirstPart = me.substr(0, sepMe);
        currentDirFirstPart = currentDir.substr(0, sepCurrentDir);

        // While the first part are equal,
        // we still can advance to find de closest relative part
    } while(!meFirstPart.compare(currentDirFirstPart));

    utils::String outPath;
    while (currentDir.compare("")) {
        // Tant que currentDir n'est pas vide, reculer
        // Trouver le prochain séparateur
        sepCurrentDir = currentDir.find("/");

        // Séparer la première et la dernière partie
        if (sepCurrentDir != std::string::npos) {
            // -1 Si on est au dernier dossier
            // et que celui-ci ne se termine pas par "/"
            currentDirFirstPart = currentDir.substr(0, sepCurrentDir);
            currentDir = currentDir.substr(sepCurrentDir+1);
        } else {
            currentDir = "";
        }

        outPath += "../";
        // Tant que les premières parties sont égales,
        // continuer à avancer dans le path
    };
    if (!outPath.compare("") && me.find("../") != 0) {
        outPath += "./";
    }

    outPath += me;

    return outPath;
}

utils::String utils::Path::absoluteFolder(
    const utils::Path &path){
    if (path.m_isFolderAbsolute) {
        return path.folder();
    }

    utils::String base;
#ifdef _WIN32
    utils::String current(currentDir());
    std::smatch matches;

    if (std::regex_search(current, matches, std::regex("^([A-Z]):[\\/].*$"))) {
        base = matches[1].str() + ":/";
    } else {
        utils::Error::raise("I could not find the current drive to estimate the path");
    }
#else
    base = "/";
#endif
    return base + relativePath(path, base);
}

utils::String utils::Path::absoluteFolder() const {
    if (m_isFolderAbsolute) {
        return m_folder;
    } else {
        return currentDir() + m_folder;
    }
}

utils::String utils::Path::absolutePath() const {
    if (m_filename.compare("")) {
        if (m_extension.compare("")) {
            return absoluteFolder() + m_filename + "." + m_extension;
        } else {
            return absoluteFolder() + m_filename;
        }
    } else {
        return absoluteFolder();
    }
}

utils::String utils::Path::toUnixFormat(
    const utils::String& path){
    utils::String pathOut(path);

    // Depending on the string origin, "\\" is either the character "\"
    // escaped or the character "\" written twice. Test for both
    size_t pos(pathOut.rfind("\\\\"));
    while (pos != std::string::npos) {
        pathOut.replace(pos, 2, "/");
        pos = pathOut.rfind("\\\\");
    }

    // However, this next hunk can create false positive each time a
    // legitimate escape character is used (should not happen in a path?)
    pos = pathOut.rfind("\\");
    while (pos != std::string::npos) {
        pathOut.replace(pos, 1, "/");
        pos = pathOut.rfind("\\");
    }
    return pathOut;
}

utils::String utils::Path::toWindowsFormat(
    const utils::String &path){
    utils::String pathOut(path);
    size_t pos(pathOut.rfind("/"));
    while (pos != std::string::npos) {
        pathOut.replace(pos, 1, "\\\\");
        pos = pathOut.rfind("/");
    }
    return pathOut;
}

utils::String utils::Path::originalPath() const {
    return m_originalPath;
}

utils::String utils::Path::folder() const {
    return m_folder;
}

void utils::Path::setFilename(
    const utils::String& name){
    m_filename = name;
}

utils::String utils::Path::filename() const{
    return m_filename;
}

void utils::Path::setExtension(
    const utils::String &ext){
    m_extension = ext;
}

utils::String utils::Path::extension() const {
    return m_extension;
}

void utils::Path::setIsFolderAbsolute(){
    utils::String base;
#ifdef _WIN32
    utils::String current(m_folder);
    std::smatch matches;
    m_isFolderAbsolute = std::regex_search(current, matches, std::regex("^([A-Z]):[\\/].*$"));
#else
    base = "/";
    m_isFolderAbsolute = m_folder.find(base.c_str()) == 0;
#endif
}

utils::String utils::Path::currentDir(){
    char buff[FILENAME_MAX];
#ifdef _WIN32
    utils::Error::check(_getcwd(buff, FILENAME_MAX),
                                "Could not find the current directory");
#else
    utils::Error::check(getcwd(buff, FILENAME_MAX),
                                "Could not find the current directory");
#endif
    return toUnixFormat(buff) + "/";
}

void utils::Path::createFolder() const {
    const utils::String& tp(folder());
    utils::String tp2(tp);

    size_t sep = std::string::npos;
    size_t sepTrack = 0;
    do {
        // Découper en fonction de la position précédente du séparateur
        tp2 = tp2.substr(sep+1);

        // Trouver le prochain séparateur
        sep = tp2.find("/"); // Path are stored in UNIX formalism
        if (sep != std::string::npos) {
            sepTrack += sep + 1 ;

            // Séparer la première et la dernière partie
            if (!isFolderExist(
                        static_cast<utils::String>(
                            tp.substr(0, sepTrack)))) {
#ifdef _WIN32
                _mkdir(toWindowsFormat(tp.substr(0, sepTrack)).c_str());
#else
                mkdir(tp.substr(0, sepTrack).c_str(), 0777);
#endif
            }
        }
    } while (sep != std::string::npos);
}
