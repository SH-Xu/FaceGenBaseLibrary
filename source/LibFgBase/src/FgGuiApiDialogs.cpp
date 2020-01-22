//
// Coypright (c) 2020 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgGuiApiDialogs.hpp"

using namespace std;


namespace Fg {

namespace {

void
fileLoad2(Ustring fileTypesDescription,Strings extensions,string storeID,IPT<Ustring> output)
{
    Opt<Ustring>     fname = guiDialogFileLoad(fileTypesDescription,extensions,storeID);
    if (fname.valid())
        output.set(fname.val());
}

}

GuiPtr
guiLoadButton(
    Ustring const &            buttonText,
    Ustring const &            fileTypesDescription,
    Strings const &              extensions,
    string const &              storeID,
    const IPT<Ustring> &       selection)
{
    GuiButton          b;
    b.label = buttonText;
    b.action = bind(fileLoad2,fileTypesDescription,extensions,storeID,selection);
    return make_shared<GuiButton>(b);
}

}

// */
