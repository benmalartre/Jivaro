set(PXR_PREFIX pxr/usdPbd)
set(PXR_PACKAGE usdPbd)

usd_library(usdPbd
    LIBRARIES
        arch
        tf
        gf
        vt
        sdf
        usd
        usdGeom

    PUBLIC_CLASSES
        bodyAPI
        collisionAPI
        constraintAPI
        solver
        tokens

    PUBLIC_HEADERS
        api.h
        bodyAPI.h
        collisionAPI.h
        constraintAPI.h
        solver.h 
        tokens.h

    PYTHON_CPPFILES
        moduleDeps.cpp

    PYMODULE_CPPFILES
        module.cpp
        wrapCollisionAPI.cpp
        wrapConstraintAPI.cpp
        wrapBodyAPI.cpp
        wrapSolver.cpp
        wrapTokens.cpp
    
    PYTHON_FILES
        __init__.py

    RESOURCE_FILES
        plugInfo.json
        generatedSchema.usda
        schema.usda
)



