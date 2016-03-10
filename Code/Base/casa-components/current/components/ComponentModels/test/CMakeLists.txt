set (tests
dPointShape
dSkyCompBase
dSpectralModel
dTwoSidedShape
tComponentList
tComponentType
tConstantSpectrum
tDiskShape
tFluxStandard
tFluxStandard2
tGaussianComponent
tGaussianShape
tPointComponent
tPointShape
tSkyCompRep
tSpectralIndex
)

include_directories(${CASACORE_INCLUDE_DIR})

foreach (test ${tests})
    add_executable (${test} ${test}.cc)
    target_link_libraries (${test} casa_components)
    add_test (${test} ${CMAKE_SOURCE_DIR}/cmake/cmake_assay ./${test})
endforeach (test)
