cookiecutter-dcpypackage
========================

A [cookiecutter](https://github.com/audreyr/cookiecutter) template for quickly
rolling out new C++ Ice data services for ASKAP.

This template doesn't really do much, but it took almost no time to setup and
will save time by avoiding the tedious search and replace method of creating
a new application from an existing one.

Usage
-----
Install cookiecutter if you haven't already.
It works fine to install directly into the ASKAP Python environment used by
rbuild (created during bootstrapping). With the askap environment active:

    $ pip install cookiecutter

Then, to create a new project from the template:

    $ cookiecutter <path to sdp-service-template>

The new project will be created in a subdirectory of your current working
directory, so make sure to be in the desired location (or move the files after
generation).
