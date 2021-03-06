//
// Coypright (c) 2020 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgMatrixC.hpp"
#include "FgSyntax.hpp"
#include "FgMath.hpp"
#include "FgRandom.hpp"
#include "FgQuaternion.hpp"
#include "FgCommand.hpp"

#ifdef _MSC_VER
    #pragma warning(push,0)     // Eigen triggers lots of warnings
#endif

#define EIGEN_MPL2_ONLY         // Only use permissive licensed source files from Eigen
#include "Eigen/Dense"
#include "Eigen/Core"

#ifdef _MSC_VER
    #pragma warning(pop)
#endif

using namespace std;

namespace Fg {

Doubles
toDoubles(Floatss const & v)
{
    size_t          sz = 0;
    for (Floats const & fs : v)
        sz += fs.size();
    Doubles         ret;
    ret.reserve(sz);
    for (Floats const & fs : v)
        for (float f : fs)
            ret.push_back(scast<double>(f));
    return ret;
}

// Gaussian elimination can be very simply explicit in this case:
Opt<Vec2F>
solveLinear(Mat22F A,Vec2F b)
{
    Opt<Vec2F>    ret;
    float                   a0 = A[0]*A[3],
                            a1 = A[1]*A[2],
                            a0s = a0*a0,
                            a1s = a1*a1;
    if (a0s > a1s) {
        float           r1 = A[1] / A[3],
                        a11 = A[0] - r1 * A[2],
                        x = (b[0] - r1 * b[1]) / a11,
                        y = (b[1] - A[2] * x) / A[3];
        ret = Vec2F(x,y);
    }
    else if (a0s < a1s) {
        float           r1 = A[0] / A[2],
                        a12 = A[1] - r1 * A[3],
                        y = (b[0] - r1 * b[1]) / a12,
                        x = (b[1] - A[3] * y) / A[2];
        ret = Vec2F(x,y);
    }
    return ret;
}

Opt<Vec3D>
solveLinear(Mat33D A,Vec3D b)
{
    Eigen::Matrix3d         mat;
    Eigen::Vector3d         vec;
    for (uint rr=0; rr<3; ++rr)
        for (uint cc=0; cc<3; ++cc)
            mat(rr,cc) = A.rc(rr,cc);
    for (uint rr=0; rr<3; ++rr)
        vec(rr) = b[rr];
    Opt<Vec3D>         ret;
    // There are many alternatives to this in Eigen: ParialPivLU, FullPivLU, HouseholderQR etc.
    auto                    qr = mat.colPivHouseholderQr();
    if (qr.isInvertible()) {
        Eigen::Vector3d     sol = qr.solve(vec);
        ret = Vec3D(sol(0),sol(1),sol(2));
    }
    return ret;
}

Opt<Vec4D>
solveLinear(Mat44D A,Vec4D b)
{
    Eigen::Matrix4d         mat;
    Eigen::Vector4d         vec;
    for (uint rr=0; rr<4; ++rr)
        for (uint cc=0; cc<4; ++cc)
            mat(rr,cc) = A.rc(rr,cc);
    for (uint rr=0; rr<4; ++rr)
        vec(rr) = b[rr];
    Opt<Vec4D>         ret;
    // There are many alternatives to this in Eigen: ParialPivLU, FullPivLU, HouseholderQR etc.
    auto                    qr = mat.colPivHouseholderQr();
    if (qr.isInvertible()) {
        Eigen::Vector4d     sol = qr.solve(vec);
        ret = Vec4D(sol(0),sol(1),sol(2),sol(3));
    }
    return ret;
}

template<uint size>
static void
testInverse()
{
    Mat<double,size,size> a,b;
    do
        a = Mat<double,size,size>::randNormal();
    while
        (determinant(a) < 0.01);
    b = cInverse(a);
    a = (a * b + b * a) * 0.5;  // cancel errors from near-singularities in matrix
    b.setIdentity();
    double          res = (a-b).len();
    FGASSERT(res < (10.0 * size * size * epsilonD()));
}

static void
testFgMatRotateAxis(CLArgs const &)
{
    randSeedRepeatable();
    for (uint ii=0; ii<100; ii++)
    {
        double          angle = randUniform(-pi(),pi());
        Vec3D        axis = Vec3D::randNormal();
        axis /= axis.len();
        Mat33D     mat = matRotateAxis(angle,axis);
        double          err = (mat * mat.transpose() - Mat33D::identity()).len(),
                        err2 = (mat * axis - axis).len();

        FGASSERT(err < (epsilonD() * 10.0));
        FGASSERT(err2 < (epsilonD() * 10.0));
        FGASSERT(determinant(mat) > 0.0);      // Ensure SO(3) not just O(3)
    }
}

static void
testMatrixCInverse(CLArgs const &)
{
    randSeedRepeatable();
    for (size_t ii=0; ii<10; ++ii) {
        testInverse<2>();
        testInverse<3>();
    }
}

static void
testMatrixUT(CLArgs const &)
{
    for (size_t ii=0; ii<100; ++ii) {
        MatUT33D        m;
    }
}

void
testMatrixC(CLArgs const & args)
{
    Cmds            cmds {
        {testMatrixCInverse,"inv"},
        {testFgMatRotateAxis,"rot"},
        {testMatrixUT,"ut"},
    };
    doMenu(args,cmds,true);
}

Mat32D
fgTanSphere(Vec3D v)
{
    // Find permutation that sorts 'v' smallest to largest:
    Vec3UI           p(0,1,2);
    Vec3D            m = mapSqr(v);
    if (m[0] > m[1])
        std::swap(p[0],p[1]);
    if (m[p[1]] > m[p[2]])
        std::swap(p[1],p[2]);
    if (m[p[0]] > m[p[1]])
        std::swap(p[0],p[1]);
    // Gram-Schmidt starting with least co-linear axes:
    Vec3D        r0(0),
                    vn = normalize(v);
    r0[p[0]] = 1.0;
    r0 -= vn * cDot(vn,r0);
    r0 /= r0.len();
    Vec3D        r1 = crossProduct(vn,r0);
    return catHoriz(r0,r1);
}

MatUT33D
MatUT33D::inverse() const
{
    MatUT33D    ret;
    ret.m[0] = 1.0/m[0];
    ret.m[1] = -m[1]/(m[0]*m[3]);
    ret.m[2] = (m[1]*m[4]/m[3] - m[2])/(m[0]*m[5]);
    ret.m[3] = 1.0/m[3];
    ret.m[4] = -m[4]/(m[3]*m[5]);
    ret.m[5] = 1.0/m[5];
    return ret;
}

std::ostream &
operator<<(std::ostream & os,MatUT33D const & ut)
{
    Vec3D       diag {ut.m[0],ut.m[3],ut.m[5]},
                upper {ut.m[1],ut.m[2],ut.m[4]};
    return os << "Diag: " << diag << " UT: " << upper;
}

MatS33D
randMatSpd3D(double lnEigStdev)
{
    // Create a random 3D SPD by generating log-normal eigvals and a random rotation for the eigvecs:
    Mat33D          D = asDiagMat(mapFunc(Vec3D::randNormal(lnEigStdev),exp)),
                    R = QuaternionD::rand().asMatrix(),
                    M = R.transpose() * D * R;
    // M will have precision-level asymmetry so manually construct return value from upper triangular values
    // (Eigen's QR decomp fails badly if symmetry is not precise):
    return MatS33D {{{M[0],M[4],M[8]}},{{M[1],M[2],M[5]}}};
}

MatS33D::MatS33D(Mat33D const & m) :
    diag {{m[0],m[4],m[8]}},
    offd {{m[1],m[2],m[5]}}
{
    for (uint rr=0; rr<3; ++rr)
        for (uint cc=rr+1; cc<3; ++cc)
            FGASSERT(m.rc(rr,cc) == m.rc(cc,rr));
}

}
