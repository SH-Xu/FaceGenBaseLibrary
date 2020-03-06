//
// Coypright (c) 2020 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Makes use of boost::filesystem which requires native unicode representation arguments for unicode support
// ie. UTF-16 wstring on Windows, UTF-8 string on *nix.

#ifndef FGFILESYSTEM_HPP
#define FGFILESYSTEM_HPP

#include "FgStdLibs.hpp"
#include "FgBoostLibs.hpp"
#include "FgStdString.hpp"
#include "FgString.hpp"
#include "FgException.hpp"
#include "FgPath.hpp"

// **************************************************************************************
//                          SYSTEM DIRECTORIES
// **************************************************************************************

namespace Fg {

// Get root all-users application data directory (delimited):
// WARNING: See warning below.
Ustring
getDirSystemAppData();

// Get (and create if necessary) the all-users application data directory for the specified application.
// Note that for some users, access is not granted. I am unable to replicate this even if these
// directories are created by an admin user and the file within them is modified by a non-admin user....
Ustring
getDirSystemAppData(Ustring const & groupName,Ustring const & appName);

// Avoid using Windows 'roaming' directories as they only roam the WDS LAN, not personal cloud
// (see user documents directory below):
Ustring
getDirUserAppDataRoaming();

// Place to store local app data for this user:
Ustring
getDirUserAppDataLocal();

// As above but verifies/creates given subPath
Ustring
getDirUserAppDataLocal(const Svec<String> & subDirs);

// As above but verifies/creates subdirectory for "FaceGen" then for specified:
inline
Ustring
getDirUserAppDataLocalFaceGen(String const & subd0,String const & subd1)
{return getDirUserAppDataLocal(svec<String>("FaceGen",subd0,subd1)); }

// Can and does sometimes fail on Windows, possibly when using roaming identities.
// If it fails but 'throwOnFail' is false, it returns the empty string.
// WINDOWS: If user has OneDrive installed, new directories created here will be created within
//     the OneDrive/Documents/ directory instead of the local drive one.
Ustring
getUserDocsDir(bool throwOnFail=true);

// This has not been known to fail on Windows:
Ustring
getPublicDocsDir();

// Find FaceGen data directory from path of current executable, searching up one directory
// at a time for a directory named 'data' containing the file '_facegen_data_dir.flag'.
// If 'throwIfNotFound' is false, check the return value for the empty string (failure):
Ustring const & dataDir(bool throwIfNotFound=true);

// Manually set data directory. Useful for sandboxed platforms and debugging apps on native
// platforms:
void
setDataDir(Ustring const & dirEndingWithSlash);

// **************************************************************************************
//                          OPERATIONS ON THE FILESYSTEM
// **************************************************************************************

// We can't use boost::filesystem::is_directory inline here since it doesn't work on Windows 10 as of
// 18.04 update.
bool
isDirectory(Ustring const & path);   // Doesn't throw - returns false for invalid path

// Both 'src' and 'dst' must be file paths; 'dst' should not be a directory.
// Note that both the creation time and last modification time are preserved on the copy !
void
fileCopy(Ustring const & srcFilePath,Ustring const & dstFilePath,bool overwrite = false);

// Copy then delete original (safer than rename which doesn't work across volumes):
void
fileMove(Ustring const & srcFilePath,Ustring const & dstFilePath,bool overwrite = false);

// Will not throw, returns false for any kind of failure on 'fname':
bool
pathExists(Ustring const & path);

inline bool
fileExists(Ustring const & fname)
{return (!isDirectory(fname) && pathExists(fname)); }

inline void
renameNode(Ustring const & from,Ustring const & to)
{return boost::filesystem::rename(from.ns(),to.ns()); }

// Update last written time on existing file (will not create). Avoid large files as it current re-writes:
void
fileTouch(Ustring const & fname);

struct      DirectoryContents
{
    Ustrings    filenames;
    Ustrings    dirnames;   // Not including separators
};

// If dirName is a relative path, the current directory is used as the base point.
// The names strings are returned without the path.
// Directory names "." and ".." are NOT returned.
DirectoryContents
directoryContents(Ustring const & dirName);

// Directory names end with a delimiter:
Ustring
getCurrentDir();

bool                                // true if successful
setCurrentDir(
    Ustring const &    dir,        // Accepts full path or relative path
    bool throwOnFail=true);

bool                                // true if successful
setCurrentDirUp();

// Doesn't remove read-only files / dirs:
inline void
pathRemove(Ustring const & fname)
{boost::filesystem::remove(fname.ns()); }

// Ignores read-only or hidden attribs on Windows (identical to pathRemove on nix):
void
deleteFile(Ustring const &);

// Only works on empty dirs, return true if successful:
bool
removeDirectory(
    Ustring const &    dirName,
    bool                throwOnFail=false);

// Throws on failure:
void
deleteDirectoryRecursive(Ustring const &);      // Full recursive delete

// Accepts full or relative path, but only creates last delimited directory:
bool            // Returns false if the directory already exists, true otherwise
createDirectory(Ustring const &);

// Create all non-existing directories in given path.
// An undelimited name will be created as a directory:
void
createPath(Ustring const &);

Ustring                        // Return the full path of the executable
getExecutablePath();

Ustring                        // Return the full directory of the current application binary
getExecutableDirectory();

// Returns true if the supplied filename is a file which can be read
// by the calling process:
bool
fileReadable(Ustring const & filename);

String
loadRawString(Ustring const & filename);

// Setting 'onlyIfChanged' to false will result in the file always being written,
// regardless of whether the new data may be identical.
// Leaving 'true' is useful to avoid triggering unwanted change detections.
bool    // Returns true if the file was written
saveRaw(String const & data,Ustring const & filename,bool onlyIfChanged=true);

// Returns true if identical:
bool
equateFilesBinary(Ustring const & file1,Ustring const & file2);

// Returns true if identical except for line endings (LF, CRLF or CR):
// Need to use this for text file regression since some users may have their VCS configured (eg. git default)
// to auto convert all text files to CRLF on Windows. UTF-8 aware.
bool
equateFilesText(Ustring const & fname0,Ustring const & fname1);

// Returns false if the given file or directory cannot be read.
// The returned time is NOT compatible with std raw time and will in fact crash getDateTimeString().
// Some *nix systems don't support creation time.
// WINE API bug returns last modification time.
bool
getCreationTime(Ustring const & path,uint64 & time);

// Works for both files and directories.
// Don't use boost::filesystem::last_write_time(); it doesn't work; returns create time on Win.
std::time_t
getLastWriteTime(Ustring const & node);

// Return true if any of the sources have a 'last write time' newer than any of the sinks,
// of if any of the sinks don't exist (an error results if any of the sources don't exist):
bool
fileNewer(Ustrings const & sources,Ustrings const & sinks);

inline
bool
fileNewer(Ustring const & src,Ustring const & dst)
{return fileNewer(svec(src),svec(dst)); }

// Usually only need to include the one last output of a code chunk as 'dst':
inline
bool
fileNewer(Ustrings const & sources,Ustring const & dst)
{return fileNewer(sources,svec(dst)); }

struct  PushDir
{
    Ustrings    orig;

    // Often need to create in different scope from 'push':
    PushDir() {}

    explicit
    PushDir(Ustring const & dir)
    {push(dir); }

    ~PushDir()
    {
        if (!orig.empty())
            setCurrentDir(orig[0]);
    }

    void push(Ustring const & dir)
    {
        orig.push_back(getCurrentDir());
        setCurrentDir(dir);
    }

    void pop()
    {
        setCurrentDir(orig.back());
        orig.resize(orig.size()-1);
    }

    void change(Ustring const & dir)
    {setCurrentDir(dir); }
};

// Very simple glob - only matches '*' at beginning or end of file base name (but not both
// unless whole name is '*') and/or extension.
// A single '*' does not glob with base and extension.
// Does not glob on input directory name.
// RETURNS: Matching filenames without directory:
Ustrings
globFiles(const Path & path);

// As above but the full path is given by 'basePath + keepPath' and the return paths include 'keepPath':
Ustrings
globFiles(Ustring const & basePath,Ustring const & relPath,Ustring const & filePattern);

// Returns name of each matching file & dir:
DirectoryContents
globNodeStartsWith(const Path & path);

// Returns the additional (delta) Ustring of every file that starts with the same base name and has
// additional characters and the same extension:
Ustrings
globBaseVariants(const Ustring & pathBaseExt);

// 'toDir' must exist.
// Returns true if there were any files in 'toDir' with the same name as a 'fromDir' file:
bool
fgCopyAllFiles(Ustring const & fromDir,Ustring const & toDir,bool overwrite=false);

// Throws an exception if the filename already exists in the current directory:
void
fgCopyToCurrentDir(const Path & file);

// WARNING: Does not check if dirs are sym/hard links so be careful.
// The tip of 'toDir' will be created.
// Will throw on overwrite of any file or directory:
void
copyRecursive(Ustring const & fromDir,Ustring const & toDir);

// Copy 'src' to 'dst' if 'src' is newer or 'dst' (or its path) doesn't exist.
// Doesn't work reliably across network shares due to time differences.
void
mirrorFile(const Path & src,const Path & dst);

}

#endif
