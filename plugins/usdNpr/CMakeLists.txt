set(PXR_PREFIX pxr/usd)
set(PXR_PACKAGE usdNpr)

usd_library(usdNpr
    LIBRARIES
        tf
        vt
        sdf
        usd
        usdGeom

    PUBLIC_CLASSES
        contour
        tokens

    PUBLIC_HEADERS
        api.h
        contour.h

    PYTHON_CPPFILES
        moduleDeps.cpp

    PYMODULE_CPPFILES
        module.cpp
        wrapContour.cpp
        wrapTokens.cpp
    
    PYTHON_FILES
        __init__.py

    RESOURCE_FILES
        plugInfo.json
        generatedSchema.usda
        schema.usda
)



