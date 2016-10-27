#!/bin/bash -l
#
# Launches a job to mosaic all individual beam continuum images to a
# single image. After completion, runs the source-finder on the
# mosaicked image.
#
# @copyright (c) 2016 CSIRO
# Australia Telescope National Facility (ATNF)
# Commonwealth Scientific and Industrial Research Organisation (CSIRO)
# PO Box 76, Epping NSW 1710, Australia
# atnf-enquiries@csiro.au
#
# This file is part of the ASKAP software distribution.
#
# The ASKAP software distribution is free software: you can redistribute it
# and/or modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of the License,
# or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
#
# @author Matthew Whiting <Matthew.Whiting@csiro.au>
#

ID_LINMOS_FIELD_CONT=""

DO_IT=$DO_MOSAIC

# Get the name of the mosaicked image
imageCode=restored
FIELD="."
setImageProperties cont

if [ $CLOBBER == false ] && [ -e ${OUTPUT}/${imageName} ]; then
    if [ $DO_IT == true ]; then
        echo "Image ${imageName} exists, so not running continuum mosaicking"
    fi
    DO_IT=false
fi

if [ $DO_IT == true ]; then

    sbatchfile=$slurms/linmos_all.sbatch
    cat > $sbatchfile <<EOFOUTER
#!/bin/bash -l
#SBATCH --partition=${QUEUE}
#SBATCH --clusters=${CLUSTER}
${ACCOUNT_REQUEST}
${RESERVATION_REQUEST}
#SBATCH --time=${JOB_TIME_LINMOS}
#SBATCH --ntasks=1
#SBATCH --ntasks-per-node=1
#SBATCH --job-name=linmosFull
${EMAIL_REQUEST}
${exportDirective}
#SBATCH --output=$slurmOut/slurm-linmos-%j.out

${askapsoftModuleCommands}

BASEDIR=${BASEDIR}
cd $OUTPUT
. ${PIPELINEDIR}/utils.sh	

# Make a copy of this sbatch file for posterity
sedstr="s/sbatch/\${SLURM_JOB_ID}\.sbatch/g"
cp $sbatchfile \`echo $sbatchfile | sed -e \$sedstr\`

nterms=${NUM_TAYLOR_TERMS}
IMAGE_BASE_CONT=${IMAGE_BASE_CONT}
SB_SCIENCE=${SB_SCIENCE}

for imageCode in restored altrestored image residual; do 

    for((TERM=0;TERM<\${nterms};TERM++)); do

        imList=""
        wtList=""
        BEAM=all
        for FIELD in ${FIELD_LIST}; do
            setImageProperties cont
            if [ -e \${FIELD}/\${imageName} ]; then
                if [ "\${imList}" == "" ]; then
                    imList="\${FIELD}/\${imageName}"
                    wtList="\${FIELD}/\${weightsImage}"
                else
                    imList="\${imList},\${FIELD}/\${imageName}"
                    wtList="\${wtList},\${FIELD}/\${weightsImage}"
                fi
            fi
        done
        if [ "\${imList}" != "" ]; then
            FIELD="."
            setImageProperties cont
            echo "Mosaicking to form \${imageName}"
            parset=${parsets}/science_linmosFull_\${imageCode}_\${SLURM_JOB_ID}.in
            log=${logs}/science_linmosFull_\${imageCode}_\${SLURM_JOB_ID}.log
            cat > \${parset} << EOFINNER
linmos.names            = [\${imList}]
linmos.weights          = [\${wtList}]
linmos.outname          = \$imageName
linmos.outweight        = \$weightsImage
linmos.weighttype       = FromWeightImages
EOFINNER

            NCORES=1
            NPPN=1
            aprun -n \${NCORES} -N \${NPPN} $linmos -c \$parset > \$log
            err=\$?
            for im in `echo \${imList} | sed -e 's/,/ /g'`; do
                rejuvenate \$im
            done
            extractStats \${log} \${NCORES} \${SLURM_JOB_ID} \${err} linmosFull_\${imageCode} "txt,csv"
            if [ \$err != 0 ]; then
                exit \$err
            fi
        else
            echo "ERROR - no good images were found for mosaicking image type '\${imageCode}'!"
            writeStats \${SLURM_JOB_ID} linmosFull_\${imageCode} FAIL --- --- --- --- --- txt > stats/stats-\${SLURM_JOB_ID}-linmosFull.txt
            writeStats \${SLURM_JOB_ID} linmosFull_\${imageCode} FAIL --- --- --- --- --- csv > stats/stats-\${SLURM_JOB_ID}-linmosFull.csv
        fi
    done
done
EOFOUTER

    if [ $SUBMIT_JOBS == true ]; then
        FULL_LINMOS_DEP=`echo $FULL_LINMOS_DEP | sed -e 's/afterok/afterany/g'`
	ID_LINMOS_CONT_ALL=`sbatch $FULL_LINMOS_DEP $sbatchfile | awk '{print $4}'`
	recordJob ${ID_LINMOS_CONT_ALL} "Make a mosaic image of the science observation, with flags \"${FULL_LINMOS_DEP}\""
    else
	echo "Would make a mosaic image of the science observation, with slurm file $sbatchfile"
    fi

    echo " "

fi


if [ ${DO_SOURCE_FINDING_MOSAIC} == true ]; then
    # Run the sourcefinder on the mosaicked image.

    # set the $imageBase variable to have 'linmos' in it
    imageCode=restored
    FIELD="."
    setImageProperties cont
    FIELD="SB${SB_SCIENCE}"

    . ${PIPELINEDIR}/sourcefinding.sh

fi
