//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgPlatform.hpp"
#include "FgFileSystem.hpp"
#include "FgException.hpp"
#include "FgOut.hpp"
#include "FgStdStream.hpp"
#include "FgTestUtils.hpp"
#include "FgScopeGuard.hpp"
#include "FgMetaFormat.hpp"
#include "FgCommand.hpp"

using namespace std;

namespace Fg {

static
void
testCurrentDirectory(CLArgs const & args)
{
    FGTESTDIR
    try
    {
        char32_t        ch = 0x00004EE5;            // A Chinese character
        Ustring        chinese(ch);
        Ustring        oldDir = getCurrentDir();
        Ustring        dirName = chinese + fgDirSep();
        createDirectory(dirName);
        setCurrentDir(dirName);
        Ustring        newDir = getCurrentDir();
        Ustring        expected = oldDir + dirName;
        setCurrentDir(oldDir);
        Ustring        restored = getCurrentDir();
        FGASSERT(removeDirectory(dirName));
        fgout << fgnl << "Original directory:    " << oldDir.as_utf8_string();
        fgout << fgnl << "New current directory: " << newDir.as_utf8_string();
        fgout << fgnl << "Expected directory:    " << expected.as_utf8_string();
        fgout << fgnl << "Restored directory:    " << restored.as_utf8_string();
        FGASSERT(expected == newDir);
    }
    catch (FgExceptionNotImplemented const & e) 
    {
        fgout << e.no_tr_message();
    }
}

static
void
testOfstreamUnicode(CLArgs const & args)
{
    FGTESTDIR
    char32_t        cent = 0x000000A2;              // The cent sign
    Ustring        test = Ustring(cent);
    Ofstream      ofs(test);
    FGASSERT(ofs);
    ofs.close();
    pathRemove(test);
}

static
void
testReadableFile(CLArgs const & args)
{
    FGTESTDIR
    std::ofstream ofs("testReadableFile.txt");
    FGASSERT(ofs);
    ofs << "Hi";
    ofs.close();
    FGASSERT(fileReadable("testReadableFile.txt"));
    FGASSERT(!fileReadable("This file does not exist"));
}

static
void
testDeleteDirectory(CLArgs const & args)
{
    FGTESTDIR
    char32_t        ch = 0x000000A2;              // The cent sign
    Ustring        cent = Ustring(ch)+"/";
    Ustring        name = "testDeleteDirectory/";
    createDirectory(name);
    FGASSERT(pathExists(name));
    createDirectory(name+cent);
    fgSaveXml(name+cent+"a",42);
    fgSaveXml(name+"b",21);
    deleteDirectoryRecursive(name);
    FGASSERT(!pathExists(name));
}

static
void
testRecursiveCopy(CLArgs const & args)
{
    FGTESTDIR
    string          path = "silly-v3.4.7/subdir/";
    createPath("tst1/"+path);
    Ofstream      ofs("tst1/"+path+"file");
    ofs << "hello";
    ofs.close();
    fgCopyRecursive("tst1","tst2");
    Ifstream      ifs("tst2/"+path+"file");
    string          hello;
    ifs >> hello;
    FGASSERT(hello == "hello");
}

static
void
testExists(CLArgs const &)
{
    FGASSERT(!pathExists("//doesNotExists"));
}

void
fgFileSystemTest(CLArgs const & args)
{
    vector<Cmd>   cmds;
    cmds.push_back(Cmd(testCurrentDirectory,"curDir"));
    cmds.push_back(Cmd(testOfstreamUnicode,"ofsUni"));
    cmds.push_back(Cmd(testReadableFile,"readable"));
    cmds.push_back(Cmd(testDeleteDirectory,"delDir"));
    cmds.push_back(Cmd(testRecursiveCopy,"recurseCopy"));
    cmds.push_back(Cmd(testExists,"exists"));
    doMenu(args,cmds,true,true,true);
}

}
