//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgQuaternion.hpp"
#include "FgApproxEqual.hpp"
#include "FgRandom.hpp"
#include "FgMain.hpp"

using namespace std;

namespace Fg {

double
tanDeltaMag(const QuaternionD & lhs,const QuaternionD & rhs)
{
    Vec4D    lv = lhs.asVec4(),
                rv = rhs.asVec4();
    // Ensure we compare appropriate relative signs of the vectors:
    if (cDot(lv,rv) < 0.0)
        rv *= -1.0;
    return cMag(rv-lv);
}

void
fgQuaternionTest(CLArgs const &)
{
    QuaternionD           id;
    // Axis rotations:
    randSeedRepeatable();
    for (size_t ii=0; ii<5; ++ii) {
        double          r = randNormal();
        Mat33D          qx = cRotateX(r).asMatrix(),
                        mx = matRotateX(r),
                        dx = qx - mx;
        FGASSERT(cMax(mapAbs(dx.m)) < epsilonD()*8);
        Mat33D          qy = cRotateY(r).asMatrix(),
                        my = matRotateY(r),
                        dy = qy - my;
        FGASSERT(cMax(mapAbs(dy.m)) < epsilonD()*8);
        Mat33D          qz = cRotateZ(r).asMatrix(),
                        mz = matRotateZ(r),
                        dz = qz - mz;
        FGASSERT(cMax(mapAbs(dz.m)) < epsilonD()*8);
    }
    // asMatrix is orthonormal:
    for (size_t ii=0; ii<5; ++ii) {
        QuaternionD     q = QuaternionD::rand();
        Mat33D          m = q.asMatrix(),
                        del = m * m.transpose() - Mat33D::identity();
        FGASSERT(cMax(mapAbs(del.m)) < epsilonD()*8);
    }
    // Composition:
    for (size_t ii=0; ii<5; ++ii) {
        QuaternionD     q0 = QuaternionD::rand(),
                        q1 = QuaternionD::rand(),
                        q2 = q1 * q0;
        Mat33D          m0 = q0.asMatrix(),
                        m1 = q1.asMatrix(),
                        m2 = m1 * m0,
                        m2q = q2.asMatrix(),
                        del = m2q-m2;
        FGASSERT(cMax(mapAbs(del.m)) < epsilonD()*8);
    }
    // Inverse:
    for (size_t ii=0; ii<5; ++ii) {
        QuaternionD     q0 = QuaternionD::rand(),
                        q1 = q0.inverse(),
                        q2 = q1 * q0;
        FGASSERT(sqrt(tanDeltaMag(q2,id)) < epsilonD()*8);
    }
}

}
