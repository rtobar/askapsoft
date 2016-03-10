set (tests
tGaussianMultipletSpectralElement
tLorentzianSpectralElement
tProfileFit1D
tSpectralFit
)
include_directories(${CASACORE_INCLUDE_DIR})

foreach (test ${tests})
    add_executable (${test} ${test}.cc)
    target_link_libraries (${test} casa_components)
    add_test (${test} ${CMAKE_SOURCE_DIR}/cmake/cmake_assay ./${test})
endforeach (test)
