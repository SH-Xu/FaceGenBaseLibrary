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

#ifdef _MSC_VER
    #pragma warning(push,0)     // Eigen triggers lots of warnings
#endif

#define EIGEN_MPL2_ONLY     // Only use permissive licensed source files from Eigen
#include "Eigen/Dense"
#include "Eigen/Core"

#ifdef _MSC_VER
    #pragma warning(pop)
#endif

using namespace std;

using namespace Eigen;

namespace Fg {

Vec3D
solve(Mat33D A_,Vec3D b_)
{
    Matrix3d        A;
    Vector3d        b;
    for (uint rr=0; rr<3; ++rr) {
        for (uint cc=0; cc<3; ++cc)
            A(rr,cc) = A_.rc(rr,cc);
        b[rr] = b_[rr];
    }
    // If the matrix is singular, this returns a solution vector with one or more components equal to zero:
    ColPivHouseholderQR<Matrix3d>   alg(A);
    Vector3d        r = alg.solve(b);
    return Vec3D {r[0],r[1],r[2]};
}

void
cEigsRsm_(MatD const & rsm,Doubles & vals,MatD & vecs)
{
    size_t              dim = rsm.ncols;
    FGASSERT(rsm.nrows == dim);
    // Ensure exact symmetry (results will be wrong otherwise) and valid values and copy into Eigen format:
    MatrixXd            mat(dim,dim);
    for (size_t rr=0; rr<dim; ++rr) {
        for (size_t cc=rr; cc<dim; ++cc) {
            double          v = (rsm.rc(rr,cc) + rsm.rc(cc,rr)) * 0.5;
            FGASSERT(boost::math::isfinite(v));
            mat(rr,cc) = v;
            mat(cc,rr) = v;
        }
    }
    // Eigen runtime is more than 3x faster than equivalent JAMA or NRC function on 1000x1000 random RSM,
    // but yields slightly larger residual errors than JAMA, which itself is about 2x larger than NRC:
    SelfAdjointEigenSolver<MatrixXd>    es(mat);
    const MatrixXd &    eigVecs = es.eigenvectors();
    const VectorXd &    eigVals = es.eigenvalues();
    vals.resize(dim);
    vecs.resize(dim,dim);
    for (size_t rr=0; rr<dim; ++rr)
        vals[rr] = eigVals(rr);
    for (size_t rr=0; rr<dim; ++rr)
        for (size_t cc=0; cc<dim; ++cc)
            vecs.rc(rr,cc) = eigVecs(rr,cc);        // MatrixXd takes (row,col) order
}

EigsRsmC<3>
cEigsRsm(const Mat<double,3,3> & rsm)
{
    Matrix3d            mat;
    for (size_t rr=0; rr<3; ++rr) {
        for (size_t cc=rr; cc<3; ++cc) {
            double          v = (rsm.rc(rr,cc) + rsm.rc(cc,rr)) * 0.5;
            FGASSERT(boost::math::isfinite(v));
            mat(rr,cc) = v;
            mat(cc,rr) = v;
        }
    }
    // Eigen supposedly has 3x3 specialization of this in closed-form:
    SelfAdjointEigenSolver<Matrix3d>    es(mat);
    const Matrix3d &    eigVecs = es.eigenvectors();
    const Vector3d &    eigVals = es.eigenvalues();
    EigsRsmC<3>       ret;
    for (size_t rr=0; rr<3; ++rr)
        ret.vals[rr] = eigVals(rr);
    for (size_t rr=0; rr<3; ++rr)
        for (size_t cc=0; cc<3; ++cc)
            ret.vecs.rc(rr,cc) = eigVecs(rr,cc);
    return ret;
}

EigsRsmC<4>
cEigsRsm(const Mat<double,4,4> & rsm)
{
    Matrix4d            mat;
    for (size_t rr=0; rr<4; ++rr) {
        for (size_t cc=rr; cc<4; ++cc) {
            double          v = (rsm.rc(rr,cc) + rsm.rc(cc,rr)) * 0.5;
            FGASSERT(boost::math::isfinite(v));
            mat(rr,cc) = v;
            mat(cc,rr) = v;
        }
    }
    SelfAdjointEigenSolver<Matrix4d>    es(mat);
    const Matrix4d &    eigVecs = es.eigenvectors();
    const Vector4d &    eigVals = es.eigenvalues();
    EigsRsmC<4>       ret;
    for (size_t rr=0; rr<4; ++rr)
        ret.vals[rr] = eigVals(rr);
    for (size_t rr=0; rr<4; ++rr)
        for (size_t cc=0; cc<4; ++cc)
            ret.vecs.rc(rr,cc) = eigVecs(rr,cc);
    return ret;
}

template<size_t dim>
EigsC<dim>
fgEigsT(const Mat<double,dim,dim> & in)
{
    // Ensure valid value and copy into Eigen format:
    MatrixXd            mat(dim,dim);
    for (size_t rr=0; rr<dim; ++rr) {
        for (size_t cc=0; cc<dim; ++cc) {
            double          v = in.rc(rr,cc);
            FGASSERT(boost::math::isfinite(v));
            mat(rr,cc) = v;                 // MatrixXd takes (row,col) order
        }
    }
    EigenSolver<MatrixXd>   es;
    es.compute(mat);
    EigsC<dim>            ret;
    const VectorXcd &       eigVals = es.eigenvalues();
    const MatrixXcd &       eigVecs = es.eigenvectors();
    for (size_t rr=0; rr<dim; ++rr)
        ret.vals[rr] = eigVals(rr);
    for (size_t cc=0; cc<dim; ++cc)
        for (size_t rr=0; rr<dim; ++rr)
            ret.vecs.rc(rr,cc) = eigVecs(rr,cc);
    return ret;
}

// Eigen does not have specialized solutions of 'EigenSolve' for small values
// so we just insantiate generic version above:
EigsC<3>
cEigs(Mat33D const & mat)
{return fgEigsT<3>(mat); }

EigsC<4>
cEigs(Mat44D const & mat)
{return fgEigsT<4>(mat); }

}
