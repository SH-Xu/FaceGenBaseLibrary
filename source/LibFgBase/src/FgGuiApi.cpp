//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"
#include "FgGuiApi.hpp"
#include "FgCommand.hpp"
#include "FgTime.hpp"
#include "FgImageIo.hpp"

using namespace std;

namespace Fg {

void
fgGuiTestmDialogSplashScreen(CLArgs const &)
{
    std::function<void(void)>     f = guiDialogSplashScreen();
    fgout << fgnl << "Splash screen displayed, waiting 3 seconds ... \n";
    fgSleep(3);
    f();
}

GuiVal<string>
guiImageFormat(string const & label,bool warnTransparency,Ustring const & store)
{
    GuiVal<string>      ret;
    Ustrings            descs;
    Strings             exts;
    for (ImgFileFormat const & iff : imgFileFormats()) {
        descs.push_back(iff.description);
        exts.push_back(iff.extensions.at(0));
    }
    if (warnTransparency)
        descs[1] += " (no transparency)";
    IPT<size_t>         idxN = (store.empty()) ? makeIPT<size_t>(0) : makeSavedIPT<size_t>(0,store);
    ret.win = guiGroupbox(label,guiRadio(descs,idxN));
    ret.valN = link1<size_t,string>(idxN,[=](size_t const & idx){return exts.at(idx);});
    return ret;
}

void
fgTestmGui2(CLArgs const &)
{
    Ustring        store = fgDirUserAppDataLocalFaceGen("base","testm gui2");
    GuiVal<vector<bool> >    checkboxes;
    {
        Ustrings                   labels;
        labels.push_back("Box 1");
        labels.push_back("Box 2");
        vector<bool>                defaults;
        defaults.push_back(false);
        defaults.push_back(true);
        checkboxes = guiCheckboxes(labels,defaults);
    }
    GuiPtr    img;
    {
        IPT<ImgC4UC>    imgN = makeIPT(imgLoadAnyFormat(dataDir()+"base/trees.jpg"));
        IPT<Vec2Fs>      ptsN = makeIPT(fgSvec(Vec2F(0.5,0.5)));
        img = guiImage(imgN,ptsN);
    }
    GuiPtr    radio;
    {
        Ustrings           strs = {"Choice 1","Choice 2"};
        IPT<size_t>         selN(0);
        radio = guiRadio(strs,selN);
    }
    GuiPtr    txt;
    {
        GuiPtr      t1 = guiText("Hello world."),
                    t2 = guiTextEdit(makeIPT(Ustring("Edit me"))),
                    t3 = guiTextEditFixed(makeIPT(3.14),VecD2(-9.0,9.0)),
                    t4 = guiTextEditFloat(makeIPT(2.73),VecD2(-9.0,9.0),6);
        txt = guiSplit(false,fgSvec(t1,t2,t3,t4));
    }
    GuiPtr    sliders;
    {
        Ustrings           labs = {"Slider 1","Slider 2"};
        GuiSliders       sldrs = guiSliders(labs,VecD2(-1,1),0,0.1);
        sliders = guiSplit(false,sldrs.sliders);
    }
    GuiPtr        scroll;
    {
        string          text1,text2;
        for (size_t ii=0; ii<100; ++ii)
            text1 += "Tab 1 Line " + toStrDigits(ii,3) + "\n";
        for (size_t ii=0; ii<10; ++ii)
            text2 += "Tab 2 Line " + toStrDigits(ii,3) + "\n";
        GuiTabDef        tab1 = guiTab("Tab 1",guiSplitScroll(fgSvec(guiText(text1)))),
                        tab2 = guiTab("Tab 2",guiSplitScroll(fgSvec(guiText(text2))));
        scroll = guiTabs(fgSvec(tab1,tab2));
    }
    GuiPtr    left = guiSplit(false,checkboxes.win,radio),
                tab1 = guiSplit(true,left,img),
                tab2 = guiSplit(false,txt,sliders),
                win = guiTabs(fgSvec(guiTab("Tab1",tab1),guiTab("Tab2",tab2),guiTab("Scroll",scroll)));
    guiStartImpl("GUI2 testm",win,fgDirUserAppDataLocalFaceGen("Base","GUI2 testm"));
}

}

// */
