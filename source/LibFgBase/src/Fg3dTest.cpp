//
// Coypright (c) 2020 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgTestUtils.hpp"
#include "Fg3dMeshOps.hpp"
#include "Fg3dMeshIo.hpp"
#include "FgApproxEqual.hpp"
#include "Fg3dDisplay.hpp"
#include "FgImgDisplay.hpp"
#include "FgCommand.hpp"
#include "Fg3dTopology.hpp"
#include "FgAffine1.hpp"
#include "FgBuild.hpp"
#include "FgCoordSystem.hpp"

using namespace std;

namespace Fg {

static
void
addSubdivisions(
    vector<Mesh> &  meshes,
    Mesh const &    mesh)
{
    meshes.push_back(mesh);
    for (uint ii=0; ii<5; ++ii)
        meshes.push_back(fgSubdivide(meshes.back()));
}

static
void
test3dMeshSubdivision(CLArgs const &)
{
    Mesh        meanMesh = loadTri(dataDir()+"base/Jane.tri");
    meanMesh.surfaces[0] = meanMesh.surfaces[0].convertToTris();
    fgout << meanMesh;
    // Test subdivision of surface points:
    size_t              numPts = meanMesh.surfPointNum();
    vector<Vec3F>    surfPoints0(numPts);
    for (uint ii=0; ii<numPts; ++ii)
        surfPoints0[ii] = meanMesh.surfPointPos(ii);
    meanMesh = fgSubdivide(meanMesh,false);
    vector<Vec3F>    surfPoints1(numPts);
    for (size_t ii=0; ii<numPts; ++ii) {
        surfPoints1[ii] = meanMesh.surfPointPos(ii);
        FGASSERT(
            approxEqualRel(surfPoints0[ii][0],surfPoints1[ii][0]) &&
            approxEqualRel(surfPoints0[ii][1],surfPoints1[ii][1]) &&
            approxEqualRel(surfPoints0[ii][2],surfPoints1[ii][2]));
    }    
}

void
fg3dReadWobjTest(CLArgs const &)
{
    TestDir   tmp("readObj");
    ofstream    ofs("square.obj");
    ofs << 
        "v 0.0 0.0 0.0\n"
        "v 0.0 1.0 0.0\n"
        "v 1.0 1.0 0.0\n"
        "v 1.0 0.0 0.0\n"
        "vt 0.0 0.0\n"
        "vt 0.0 1.0\n"
        "vt 1.0 1.0\n"
        "vt 1.0 0.0\n"
        "g plane\n"
        "usemtl plane\n"
        "f 1/1 2/2 3/3 4/4\n"
        ;
    ofs.close();
    Mesh    mesh = loadWObj("square.obj");
    meshView(mesh);
}

void
fgTextureImageMappingRenderTest(CLArgs const &)
{
    ofstream    ofs("square.obj");
    ofs << 
        "v 0.0 0.0 0.0\n"
        "v 0.0 1.0 0.0\n"
        "v 1.0 1.0 0.0\n"
        "v 1.0 0.0 0.0\n"
        "vt 0.0 0.0\n"
        "vt 0.0 1.0\n"
        "vt 1.0 1.0\n"
        "vt 1.0 0.0\n"
        "g plane\n"
        "usemtl plane\n"
        "f 1/1 2/2 3/3 4/4\n"
        ;
    ofs.close();
    Mesh    mesh = loadWObj("square.obj");
    string      textureFile("base/test/TextureMapOrdering.jpg");
    imgLoadAnyFormat(dataDir()+textureFile,mesh.surfaces[0].albedoMapRef());
    meshView(mesh);
}

void
fgSubdivisionTest(CLArgs const &)
{
    vector<Mesh>    meshes;
    addSubdivisions(meshes,fgTetrahedron());
    addSubdivisions(meshes,fgTetrahedron(true));
    addSubdivisions(meshes,fgPyramid());
    addSubdivisions(meshes,fgPyramid(true));
    addSubdivisions(meshes,fg3dCube());
    addSubdivisions(meshes,fg3dCube(true));
    addSubdivisions(meshes,fgOctahedron());
    addSubdivisions(meshes,fgNTent(5));
    addSubdivisions(meshes,fgNTent(6));
    addSubdivisions(meshes,fgNTent(7));
    addSubdivisions(meshes,fgNTent(8));
    meshView(meshes,true);
}

static
void
edgeDist(CLArgs const &)
{
    Mesh        mesh = loadTri(dataDir()+"base/Jane.tri");
    Surf     surf = mergeSurfaces(mesh.surfaces).convertToTris();
    Fg3dTopology    topo(mesh.verts,surf.tris.posInds);
    size_t          vertIdx = 0;    // Randomly choose the first
    vector<float>   edgeDists = topo.edgeDistanceMap(mesh.verts,vertIdx);
    float           distMax = 0;
    for (size_t ii=0; ii<edgeDists.size(); ++ii)
        if (edgeDists[ii] < maxFloat())
            setIfGreater(distMax,edgeDists[ii]);
    float           distToCol = 255.99f / distMax;
    vector<uchar>   colVal(edgeDists.size(),255);
    for (size_t ii=0; ii<colVal.size(); ++ii)
        if (edgeDists[ii] < maxFloat())
            colVal[ii] = uint(distToCol * edgeDists[ii]);
    mesh.surfaces[0].setAlbedoMap(ImgC4UC(128,128,RgbaUC(255)));
    AffineEw2F    otcsToIpcs = cOtcsToIpcs(Vec2UI(128));
    for (size_t tt=0; tt<surf.tris.size(); ++tt) {
        Vec3UI   vertInds = surf.tris.posInds[tt];
        Vec3UI   uvInds = surf.tris.uvInds[tt];
        for (uint ii=0; ii<3; ++ii) {
            RgbaUC    col(255);
            col.red() = colVal[vertInds[ii]];
            col.green() = 255 - col.red();
            mesh.surfaces[0].material.albedoMap->paint(Vec2UI(otcsToIpcs*mesh.uvs[uvInds[ii]]),col);
        }
    }
    meshView(mesh);
}

void fgSave3dsTest(CLArgs const &);
void fgSaveLwoTest(CLArgs const &);
void fgSaveMaTest(CLArgs const &);
void fgSaveFbxTest(CLArgs const &);
void fgSaveDaeTest(CLArgs const &);
void fgSaveObjTest(CLArgs const &);
void fgSavePlyTest(CLArgs const &);
void fgSaveXsiTest(CLArgs const &);

void
fg3dTest(CLArgs const & args)
{
    Cmds   cmds;
    cmds.push_back(Cmd(fgSave3dsTest,"3ds",".3DS file format export"));
    cmds.push_back(Cmd(fgSaveLwoTest,"lwo","Lightwve object file format export"));
    cmds.push_back(Cmd(fgSaveMaTest,"ma","Maya ASCII file format export"));
    cmds.push_back(Cmd(fgSaveDaeTest, "dae", "Collada DAE format export"));
    cmds.push_back(Cmd(fgSaveFbxTest, "fbx", ".FBX file format export"));
    cmds.push_back(Cmd(fgSaveObjTest, "obj", "Wavefront OBJ ASCII file format export"));
    cmds.push_back(Cmd(fgSavePlyTest, "ply", ".PLY file format export"));
#ifdef _MSC_VER     // Precision differences with gcc/clang:
    cmds.push_back(Cmd(fgSaveXsiTest, "xsi", ".XSI file format export"));
#endif
    doMenu(args,cmds,true,false,true);
}

void fgSaveFgmeshTest(CLArgs const &);

void
fg3dTestMan(CLArgs const & args)
{
    Cmds   cmds;
    cmds.push_back(Cmd(edgeDist,"edgeDist"));
    cmds.push_back(Cmd(fgSaveFgmeshTest,"fgmesh","FaceGen mesh file format export"));  // Uses GUI
    cmds.push_back(Cmd(test3dMeshSubdivision,"subdivision"));
    doMenu(args,cmds,true,false,true);
}

}

// */
