##############################################################################
# Making a spectral cube
##############################################################################


#####################
#  Script to combine individual channel images into a single cube
#   - makes use of the CP/imager/current/apps/makecube utility
#   - requires the following environment variables to be set:
#     * DODELETE - whether to delete all but the first channel images
#     * IMAGEPREFIX - the part of the image name before the channel number
#     * IMAGESUFFIX - the part of the image name after the channel number
#     * FIRSTCH - the first channel number
#     * FINALCH - the last channel number
#     * OUTPUTCUBE - the name of the final cube
#   - input images must be of the form prefix[FIRSTCH..FINALCH]suffix (prefix & suffix can be anything.
#   - the final cube will have its spectral coordinates corrected by a CASA script
#####################


if [ ${IMAGEPREFIX} == "" ] && [ ${IMAGESUFFIX} == "" ]; then
    echo "Have not set \$IMAGEPREFIX or \$IMAGESUFFIX, so not running make-spectral-cube."
elif [ "${OUTPUTCUBE}" == "" ]; then
    echo "Have not set \$OUTPUTCUBE, so not running make-spectral-cube"
else

    IMAGERANGE="${IMAGEPREFIX}[${FIRSTCH}..${FINALCH}]${IMAGESUFFIX}"

    qsubfile=make-spectral-cube--${OUTPUTCUBE}.qsub
    cat > ${qsubfile} <<EOF
#!/bin/bash -l
#PBS -W group_list=${QUEUEGROUP}
#PBS -l walltime=06:00:00
#PBS -l select=1:ncpus=1:mem=2GB:mpiprocs=1
##PBS -M first.last@csiro.au
#PBS -N makecube
#PBS -m bea
#PBS -j oe

cd \$PBS_O_WORKDIR

makecube=${ASKAP_ROOT}/Code/Components/CP/pipelinetasks/current/install/bin/makecube

outfile=${LOGDIR}/makecube-\${PBS_JOBID}.log

doDelete=$DODELETE
echo "Begin makecube \`date\`" > \$outfile
echo "Using images ${IMAGERANGE}, to create ${OUTPUTCUBE}" >> \$outfile

# Test for existence of the images
C=${FIRSTCH}
while [ \$C -le ${FINALCH} ]; do
    if [ ! -e ${IMAGEPREFIX}\${C}${IMAGESUFFIX} ]; then
	echo "Could not find image ${IMAGEPREFIX}\${C}${IMAGESUFFIX}. Exiting."
    fi
    C=\`expr \$C + 1\`
done

# Run makecube
#\$makecube \${base} ${FIRSTCH} ${FINALCH} ${OUTPUTCUBE} > \$outfile
\$makecube "${IMAGERANGE}" ${OUTPUTCUBE} > \$outfile
err=\$?
if [ \$err != 0 ]; then
    echo "Makecube failed with error \$err"
    exit \$err
fi

# Delete images if required
if [ \$doDelete == true ]; then
    C=`expr ${FIRSTCH} + 1`
    while [ \$C -le ${FINALCH} ]; do
        /bin/rm -rf ${IMAGEPREFIX}\${C}${IMAGESUFFIX} 
        C=\`expr \$C + 1\`
    done
fi

EOF

    if [ "${DRYRUN}" == "false" ]; then

        # Submit the jobs
	if [ "${DEPENDS}" ]; then
            QSUB_MAKECUBE=`${QSUB_CMD} ${DEPENDS} ${qsubfile}`
	else
            QSUB_MAKECUBE=`${QSUB_CMD} ${qsubfile}`
            QSUB_NODEPS="${QSUB_NODEPS} ${QSUB_MAKECUBE}"
	fi

    else

	echo "Makecube utility (${OUTPUTCUBE}): Dry Run Only"

    fi


fi
