//
// Coypright (c) 2020 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgMath.hpp"
#include "FgMatrixSolver.hpp"
#include "FgRandom.hpp"
#include "FgMain.hpp"
#include "FgTime.hpp"
#include "FgCommand.hpp"
#include "FgApproxEqual.hpp"
#include "FgSyntax.hpp"

using namespace std;

namespace Fg {

MatUT33D
choleskyDecompose(MatS33D S)
{
    MatUT33D        U;
    U.m[0] = sqrt(S.diag[0]);
    U.m[1] = S.offd[0] / U.m[0];
    U.m[2] = S.offd[1] / U.m[0];
    double          tmp0 = S.diag[1] - sqr(S.offd[0])/S.diag[0];
    U.m[3] = sqrt(tmp0);
    double          tmp1 = S.offd[2] - S.offd[1]*S.offd[0]/S.diag[0];
    U.m[4] = tmp1 / U.m[3];
    U.m[5] = sqrt(S.diag[2] - sqr(S.offd[1])/S.diag[0] - sqr(tmp1)/tmp0);
    return U;
}

void
testCholesky(CLArgs const &)
{
    randSeedRepeatable();
    double          resid1 = 0.0;
    size_t          N = 1000;
    for (size_t ii=0; ii<N; ++ii) {
        MatS33D         spds = randMatSpd3D(3.0);
        MatUT33D        ch = choleskyDecompose(spds);
        MatS33D         lu = ch.luProduct();
        Mat33D          out = lu.asMatrixC(),
                        spd = spds.asMatrixC(),
                        del = out - spd;
        resid1 += cMag(del.m) / cMag(spd.m);
        FGASSERT(isApproxEqualRelPrec(out,spd));
    }
    fgout << fgnl << "Cholesky unsafe RMS residual: " << sqrt(resid1/N);
}

Vec3D
solve(MatS33D A,Vec3D b)
{
    MatUT33D            cd = choleskyDecompose(A);
    MatUT33D            I = cd.inverse();
    Vec3D               c = I.tranposeMul(b);
    return I * c;
}

MatD
EigsRsm::matrix() const
{
    MatD       rhs = vecs.transpose();
    for (size_t rr=0; rr<rhs.nrows; ++rr)
        for (size_t cc=0; cc<rhs.ncols; ++cc)
            rhs.rc(rr,cc) *= vals[cc];
    return vecs * rhs;
}

namespace {

MatD
randSymmMatrix(uint dim)
{
    MatD  mat(dim,dim);
    for (uint ii=0; ii<dim; ii++) {
        for (uint jj=ii; jj<dim; jj++) {
            mat.rc(ii,jj) = randUniform();
            mat.rc(jj,ii) = mat.rc(ii,jj);
        }
    }
    return mat;
}

typedef function<void(MatD const &,Doubles &,MatD &)>  FnSolve;

void
testSymmEigenProblem(uint dim,FnSolve solve,bool print)
{
    // Random symmetric matrix, uniform distribution:
    MatD           mat = randSymmMatrix(dim);
    Doubles              eigVals;
    MatD           eigVecs;
    Timer             timer;
    solve(mat,eigVals,eigVecs);
    size_t              time = timer.readMs();
    // What is the pre-diagonalization speedup:
    MatD           innerHess = eigVecs.transpose() * mat * eigVecs,
                        innerEigvecs;
    Doubles              innerEigVals;
    timer.start();
    solve(innerHess,innerEigVals,innerEigvecs);
    size_t              timeInner = timer.readMs();
    MatD           eigValMat(dim,dim);
    eigValMat.setZero();
    for (uint ii=0; ii<dim; ii++)
        eigValMat.rc(ii,ii) = eigVals[ii];
    MatD       recon = eigVecs * eigValMat * eigVecs.transpose();
    double          residual = 0.0;
    for (uint ii=0; ii<dim; ii++)
        for (uint jj=ii; jj<dim; jj++)
            residual += sqr(recon.rc(ii,jj) - mat.rc(ii,jj));
    residual = sqrt(residual/double(dim*dim));      // root of mean value
    // RMS residual appears to go with the square root of the matrix dimension:
    double          tol = epsilonD() * sqrt(dim) * 2.0;
    fgout << fgnl << "Dim: " << dim << fgpush
        << fgnl << "RMS Residual: " << residual << " RR/tol " << residual/tol
        << fgnl << "Time: " << time
        << fgnl << "Inner time: " << timeInner;
    if (print) {
        FGOUT1(mat);
        FGOUT1(innerHess);
        FGOUT1(eigVals);
        FGOUT1(eigVecs);
        FGOUT1(innerEigVals);
        FGOUT1(innerEigvecs);
        FGOUT1(eigVecs.transpose() * eigVecs);
    }
    fgout << fgpop;
    FGASSERT(residual < tol);
}

void
testAsymEigs(CLArgs const &)
{
    Mat33D        mat = matRotateAxis(1.0,Vec3D(1,1,1));
    EigsC<3>      eigs = cEigs(mat);
    // We have to test against the reconstructed matrix as the order of eigvals/vecs will
    // differ on different platforms (eg. gcc):
    Mat33D        regress = cReal(eigs.vecs * asDiagMat(eigs.vals) * fgHermitian(eigs.vecs));
    double          residual = cRms(mat - regress);
    FGASSERT(residual < 0.0000001);
    fgout << eigs
        << fgnl << "Residual: " << residual;
}

void
testSymmEigenAuto(CLArgs const &)
{
    randSeedRepeatable();
    testSymmEigenProblem(10,cEigsRsm_,true);
    testSymmEigenProblem(30,cEigsRsm_,false);
#ifndef _DEBUG
    testSymmEigenProblem(100,cEigsRsm_,false);
    testSymmEigenProblem(300,cEigsRsm_,false);
#endif
}

void
testSymmEigenTime(CLArgs const & args)
{
    if (fgAutomatedTest(args))
        return;
    Syntax            syn(args,"<size>");
    // Random symmetric matrix, uniform distribution:
    size_t              dim = syn.nextAs<size_t>();
    randSeedRepeatable();
    MatD           mat = randSymmMatrix(uint(dim));
    Doubles              eigVals;
    MatD           eigVecs;
    Timer             timer;
    cEigsRsm_(mat,eigVals,eigVecs);
    size_t              time = timer.readMs();
    MatD           eigValMat(dim,dim);
    eigValMat.setZero();
    for (uint ii=0; ii<dim; ii++)
        eigValMat.rc(ii,ii) = eigVals[ii];
    MatD       recon = eigVecs * eigValMat * eigVecs.transpose();
    double          residual = 0.0;
    for (uint ii=0; ii<dim; ii++)
        for (uint jj=ii; jj<dim; jj++)
            residual += sqr(recon.rc(ii,jj) - mat.rc(ii,jj));
    residual = sqrt(residual/double(dim*dim));      // root of mean value
    // RMS residual appears to go with the square root of the matrix dimension:
    double          tol = epsilonD() * sqrt(dim) * 2.0;
    fgout << fgnl << "Dim: " << dim << fgpush
        << fgnl << "RMS Residual: " << residual << " RR/tol " << residual/tol
        << fgnl << "Time: " << time;
    fgout << fgpop;
    FGASSERT(residual < tol);
}

void
testSymmEigen(CLArgs const & args)
{
    Cmds      cmds;
    cmds.push_back(Cmd(testSymmEigenAuto,"auto","Automated tests"));
    cmds.push_back(Cmd(testSymmEigenTime,"time","Timing test"));
    doMenu(args,cmds,true);
}

}

void
fgMatrixSolverTest(CLArgs const & args)
{
    Cmds        cmds {
        {testCholesky,"chol","Cholesky 3x3 decomposition"},
        {testAsymEigs,"asym","Arbitrary real matrix eigensystem"},
        {testSymmEigen,"symm","Real symmetric matrix eigensystem"}
    };
    doMenu(args,cmds,true);
}

}
