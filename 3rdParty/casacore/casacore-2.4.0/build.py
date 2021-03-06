import os.path

from askapdev.rbuild.builders import CMake as Builder
import askapdev.rbuild.utils as utils

# CMake doesn't know about ROOT_DIR for blas and lapack, so need to
# explicitly name them.  Want to use the dynamic libraries in order 
# to avoid link problems with missing FORTRAN symbols.
platform =  utils.get_platform()
builder = Builder()
builder.remote_archive = "https://github.com/casacore/casacore/archive/v2.4.0.zip"

cfitsio = builder.dep.get_install_path("cfitsio")
wcslib  = builder.dep.get_install_path("wcslib")
fftw3   = builder.dep.get_install_path("fftw3")

# these work
builder.add_option("-DCFITSIO_ROOT_DIR=%s" % cfitsio)
builder.add_option("-DWCSLIB_ROOT_DIR=%s" % wcslib)
# but FFTW3_ROOT_DIR don't for the include part
builder.add_option("-DFFTW3_DISABLE_THREADS=ON")
builder.add_option("-DFFTW3_ROOT_DIR=%s" % fftw3)
builder.add_option("-DFFTW3_INCLUDE_DIRS=%s/include" % fftw3)
builder.add_option("-DUSE_FFTW3=ON")
builder.add_option("-DUSE_THREADS=ON")
# save some time
builder.add_option("-DBUILD_TESTING=OFF")
builder.add_option("-DBUILD_PYTHON=OFF")
builder.nowarnings = True

# Force use of raw GNU compilers. This is due to bug #5798 soon on the Cray XC30.
# Builds using the newer cmake (2.8.12) fail when cmake uses the Cray compiler
# wrappers
if platform['system'] == 'Darwin':
   
    if (int(platform['tversion'][1]) >= 10):
        builder.add_option("-DCMAKE_Fortran_FLAGS=-Wa,-q")
else:
# On darwin casa can spot the accelerate framework    
    blas    = builder.dep.get_install_path("blas")
    lapack  = builder.dep.get_install_path("lapack")
    
    # CMake doesn't know about ROOT_DIR for these packages, so be explicit
    builder.add_option("-DBLAS_LIBRARIES=%s" % os.path.join(blas, 'lib', 'libblas.a'))
    builder.add_option("-DLAPACK_LIBRARIES=%s" % os.path.join(lapack, 'lib', 'liblapack.a'))
    builder.add_option("-DCMAKE_C_COMPILER=gcc")
    builder.add_option("-DCMAKE_CXX_COMPILER=g++")

builder.add_option("-DCXX11=ON")


builder.build()
