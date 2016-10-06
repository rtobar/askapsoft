import os
import subprocess

from askapdev.rbuild.builders import Autotools as Builder

package_name="Healpix_3.31"

def run_autoconf():
    path = "{0}/src/cxx".format(package_name)
    print("running autoconf step for {0}".format(path))
    subprocess.call("autoconf", cwd=path)

builder = Builder(
    archivename="Healpix_3.31_2016Aug26.tar.gz",
    pkgname=package_name,
    buildsubdir="src/cxx",
)

builder.remote_archive = "Healpix_3.31_2016Aug26.tar.gz"

# builder.add_option('--with-libraries=python,date_time,filesystem,program_options,thread,system,regex --prefix=builder._prefix' )
builder.nowarnings = True

builder.add_precallback(run_autoconf)

builder.build()
