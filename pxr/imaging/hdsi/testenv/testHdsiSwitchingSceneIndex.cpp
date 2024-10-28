//
// Copyright 2024 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//

#include "pxr/imaging/hd/retainedDataSource.h"
#include "pxr/imaging/hd/retainedSceneIndex.h"
#include "pxr/imaging/hd/tokens.h"

#include "pxr/imaging/hdsi/switchingSceneIndex.h"

#include <cassert>
#include <iostream>
#include <sstream>
#include <utility>
#include <vector>

PXR_NAMESPACE_USING_DIRECTIVE

//-----------------------------------------------------------------------------

static bool
_TestBasic()
{
    HdRetainedSceneIndexRefPtr siA = HdRetainedSceneIndex::New();
    siA->AddPrims({
        { SdfPath("/Prim"), TfToken("A"), nullptr },
    });

    HdRetainedSceneIndexRefPtr siB = HdRetainedSceneIndex::New();
    siB->AddPrims({
        { SdfPath("/Prim"), TfToken("B"), nullptr },
    });

    auto switchingSi = HdsiSwitchingSceneIndex::New({ siA, siB });
    assert(switchingSi->GetPrim(SdfPath("/Prim")).primType == TfToken("A"));

    switchingSi->SetIndex(1);
    assert(switchingSi->GetPrim(SdfPath("/Prim")).primType == TfToken("B"));

    return true;
}

#define xstr(s) str(s)
#define str(s) #s
#define TEST(X)                                                                \
    std::cout << (++i) << ")" << str(X) << "..." << std::endl;                 \
    if (!X()) {                                                                \
        std::cout << "FAILED" << std::endl;                                    \
        return -1;                                                             \
    }                                                                          \
    else {                                                                     \
        std::cout << "... SUCCEEDED" << std::endl;                             \
    }

int
main(int argc, char** argv)
{
    std::cout << "STARTING testHdsiSwitchingSceneIndex" << std::endl;
    int i = 0;
    TEST(_TestBasic);
    return 0;
}
