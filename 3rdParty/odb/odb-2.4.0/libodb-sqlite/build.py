import os
import shutil
import subprocess

from askapdev.rbuild.builders import Autotools as Builder

package_name="Healpix_3.31"
path = "{0}/src/cxx".format(package_name)

def run_autoconf():
    "HealPix requires autoconf prior to configure"
    print("running autoconf step for {0}".format(path))
    subprocess.call("autoconf", cwd=path)

def install():
    """HealPix C++ makefile does not provide an install target, so we copy the
    files manually"""
    src_path = os.path.join(path, "auto")
    dst_path = os.path.join(os.getcwd(), "install")

    # copytree requires that the destination not exist
    if os.path.exists(dst_path):
        shutil.rmtree(dst_path)

    shutil.copytree(src_path, dst_path, symlinks=True)


builder = Builder(
    archivename="Healpix_3.31_2016Aug26.tar.gz",
    pkgname=package_name,
    buildsubdir="src/cxx",
    installcommand=None,
)

# setup the cfitsio dependency
cfitsio = builder.dep.get_install_path("cfitsio")
builder.add_option("--with-libcfitsio={0}".format(cfitsio))

builder.add_option("--prefix={0}/install".format(os.getcwd()))

builder.remote_archive = "Healpix_3.31_2016Aug26.tar.gz"

builder.nowarnings = True

builder.add_precallback(run_autoconf)
builder.add_postcallback(install)

builder.build()
