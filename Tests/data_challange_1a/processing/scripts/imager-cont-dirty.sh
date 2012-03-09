##############################################################################
# Continuum Imaging (Dirty)
##############################################################################

cat > cimager-cont-dirty.qsub << EOF
#!/bin/bash
#PBS -W group_list=${QUEUEGROUP}
#PBS -l select=1:ncpus=1:mem=23GB:mpiprocs=1+50:ncpus=6:mem=23GB:mpiprocs=6
#PBS -l walltime=01:00:00
##PBS -M first.last@csiro.au
#PBS -N cont-dirty
#PBS -m a
#PBS -j oe

cd \${PBS_O_WORKDIR}

cat > ${CONFIGDIR}/cimager-cont-dirty.in << EOF_INNER
Cimager.dataset                                 = MS/coarse_chan_%w.ms

Cimager.Images.Names                            = [image.i.dirty]
Cimager.Images.shape                            = [3584,3584]
Cimager.Images.cellsize                         = [10arcsec, 10arcsec]
Cimager.Images.image.i.dirty.frequency          = [1.420e9,1.420e9]
Cimager.Images.image.i.dirty.nchan              = 1
Cimager.Images.image.i.dirty.direction          = [12h30m00.00, -45.00.00.00, J2000]
#
Cimager.gridder.snapshotimaging                 = true
Cimager.gridder.snapshotimaging.wtolerance      = 2000
Cimager.gridder                                 = AWProject
Cimager.gridder.AWProject.wmax                  = 2000
Cimager.gridder.AWProject.nwplanes              = 7
Cimager.gridder.AWProject.oversample            = 4
Cimager.gridder.AWProject.diameter              = 12m
Cimager.gridder.AWProject.blockage              = 2m
Cimager.gridder.AWProject.maxfeeds              = 36
Cimager.gridder.AWProject.maxsupport            = 2048
Cimager.gridder.AWProject.variablesupport       = true
Cimager.gridder.AWProject.offsetsupport         = true
Cimager.gridder.AWProject.frequencydependent    = true
#
Cimager.solver                                  = Dirty
Cimager.solver.Dirty.tolerance                  = 0.1
Cimager.solver.Dirty.verbose                    = True
Cimager.ncycles                                 = 0

Cimager.preconditioner.Names                    = None

# Apply calibration
#Cimager.calibrate                               = true
Cimager.calibrate                               = false
Cimager.calibaccess                             = table
Cimager.calibaccess.table                       = ${CALOUTPUT}
EOF_INNER

mpirun \${ASKAP_ROOT}/Code/Components/Synthesis/synthesis/current/apps/cimager.sh -inputs ${CONFIGDIR}/cimager-cont-dirty.in > ${LOGDIR}/cimager-cont-dirty.log
EOF

if [ ! $DRYRUN ]; then
    echo "Continuum Imager (Dirty): Submitting task"
    if [ ${QSUB_CCAL} ]; then
        QSUB_CONTDIRTY=`${QSUB_CMD} -W depend=afterok:${QSUB_CCAL} cimager-cont-dirty.qsub`
    else
        QSUB_CONTDIRTY=`${QSUB_CMD} cimager-cont-dirty.qsub`
        QSUB_NODEPS="${QSUB_NODEPS} ${QSUB_CONTDIRTY}"
    fi
else
    echo "Continuum Imager (Dirty): Dry Run Only"
fi
