import os

from askapdev.rbuild.builders import Autotools as Builder

builder = Builder(
    archivename="Healpix_3.31_2016Aug26.tar.gz",
    pkgname="Healpix_3.31",
    buildsubdir="src/cxx",
)
builder.remote_archive = "Healpix_3.31_2016Aug26.tar.gz"

# builder.add_option('--with-libraries=python,date_time,filesystem,program_options,thread,system,regex --prefix=builder._prefix' )
builder.nowarnings = True

builder.build()
