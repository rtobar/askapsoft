#!/bin/bash -l
#
# A script to run diagnostic tasks on the data products that have been
# created, producing plots and other files used for QA and related
# purposes. 
#
# @copyright (c) 2017 CSIRO
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

DO_IT=$RUN_DIAGNOSTICS

if [ "${DO_IT}" == "true" ]; then

    sbatchfile=$slurms/runDiagnostics.sbatch
    cat > "$sbatchfile" <<EOFOUTER
#!/bin/bash -l
#SBATCH --partition=${QUEUE}
#SBATCH --clusters=${CLUSTER}
${ACCOUNT_REQUEST}
${RESERVATION_REQUEST}
#SBATCH --time=${JOB_TIME_DIAGNOSTICS}
#SBATCH --ntasks=1
#SBATCH --ntasks-per-node=1
#SBATCH --job-name=diagnostics
${EMAIL_REQUEST}
${exportDirective}
#SBATCH --output=$slurmOut/slurm-diagnostics-%j.out

${askapsoftModuleCommands}

BASEDIR=${BASEDIR}
cd $OUTPUT
. ${PIPELINEDIR}/utils.sh

# Make a copy of this sbatch file for posterity
sedstr="s/sbatch/\${SLURM_JOB_ID}\.sbatch/g"
thisfile=$sbatchfile
cp \$thisfile "\$(echo \$thisfile | sed -e "\$sedstr")"

# Define the lists of image names, types, 
ADD_FITS_SUFFIX=true
. "${getArtifacts}"

# Make PNG images of continuum images, overlaid with weights contours
# and fitted components

log=${logs}/diagnostics_\${SLURM_JOB_ID}.log
parset=${parsets}/diagnostics_\${SLURM_JOB_ID}.in    

if [ "$(which makeThumbnailImage.py 2> ${tmp}/whchmkthumb)" == "" ]; then

    for((i=0;i<\${#casdaTwoDimImageNames[@]};i++)); do
    
        if [ "\${casdaTwoDimImageTypes[i]}" == "cont_restored_T0" ]; then

            # make a second plot showing catalogued source positions
            seldir=$(echo \${casdaTwoDimImageNames[i]} | sed -e 's/image\.i/selavy_image\.i/g')
            if [ -e "\${seldir}/selavy-\${casdaTwoDimImageNames[i]}.components.txt" ]; then
                cat >> "\$parset" <<EOF
###### Image #\${i} catalogue #############
makeThumbnail.image = \${casdaTwoDimImageNames[i]}
makeThumbnail.imageTitle = \${casdaTwoDimThumbTitles[i]}
makeThumbnail.catalogue = \${seldir}/selavy-\${casdaTwoDimImageNames[i]}.components.txt
makeThumbnail.outdir = ${diagnostics}
makeThumbnail.imageSuffix = ${THUMBNAIL_SUFFIX}
makeThumbnail.zmin = ${THUMBNAIL_GREYSCALE_MIN}
makeThumbnail.zmax = ${THUMBNAIL_GREYSCALE_MAX}
makeThumbnail.imageSizes = [16]
makeThumbnail.imageSizeNames = [sources]
makeThumbnail.showWeightsContours = True
EOF
    
                NCORES=1
                NPPN=1
                aprun -n \${NCORES} -N \${NPPN} ${makeThumbnails} -c "\${parset}" >> "\${log}"
                err=\$?
                if [ \$err != 0 ]; then
                    echo "ERROR - Sources thumbnail creation failed for image \${casdaTwoDimImageNames[i]}" | tee -a "\${log}"
                    exit \$err
                fi

            fi

        fi

    done

else

    echo "No image thumbnails with catalogues produced, since $makeThumbnails doesn't exist in this module"

fi

#####################

# Make thumbnail images of the noise maps created by Selavy

if [ "$(which makeThumbnailImage.py 2> ${tmp}/whchmkthumb)" == "" ]; then

    for((i=0;i<\${#casdaTwoDimImageNames[@]};i++)); do
    
        if [ "\${casdaTwoDimImageTypes[i]}" == "cont_restored_T0" ]; then

            # make a second plot showing catalogued source positions
            seldir=$(echo \${casdaTwoDimImageNames[i]} | sed -e 's/image\.i/selavy_image\.i/g')
            noisemapbase="\${seldir}/noiseMap.\${casdaTwoDimImageNames[i]}"
            if [ ! -e "\${noisemapbase}.fits" ] && [ -e "\${noisemapbase}.img" ]; then
                # need to convert to FITS
                casaim=\${noisemapbase}.img
                fitsim=\${noisemapbase}.fits
                ${fitsConvertText}
            fi
            if [ -e "\${seldir}/noiseMap.\${casdaTwoDimImageNames[i]}.fits" ]; then
                cat >> "\$parset" <<EOF
###### Image #\${i} catalogue #############
makeThumbnail.image = \${noisemapbase}.fits
makeThumbnail.imageTitle = "\${casdaTwoDimThumbTitles[i]} - noise map"
makeThumbnail.catalogue = \${seldir}/selavy-\${casdaTwoDimImageNames[i]}.components.txt
makeThumbnail.outdir = ${diagnostics}
makeThumbnail.imageSuffix = ${THUMBNAIL_SUFFIX}
makeThumbnail.zmin = ${THUMBNAIL_GREYSCALE_MIN}
makeThumbnail.zmax = ${THUMBNAIL_GREYSCALE_MAX}
makeThumbnail.imageSizes = [16]
makeThumbnail.imageSizeNames = [sources]
makeThumbnail.showWeightsContours = True
EOF
    
                NCORES=1
                NPPN=1
                aprun -n \${NCORES} -N \${NPPN} ${makeThumbnails} -c "\${parset}" >> "\${log}"
                err=\$?
                if [ \$err != 0 ]; then
                    echo "ERROR - Sources thumbnail creation failed for image \${noisemapbase}.fits" | tee -a "\${log}"
                    exit \$err
                fi

            fi

        fi

    done

else

    echo "No image thumbnails with catalogues produced, since $makeThumbnails doesn't exist in this module"

fi

EOFOUTER

        if [ "${SUBMIT_JOBS}" == "true" ]; then
        dep=""
        if [ "${ALL_JOB_IDS}" != "" ]; then
            dep="-d afterok:$(echo "${ALL_JOB_IDS}" | sed -e 's/,/:/g')"
        fi
        ID_DIAG=$(sbatch ${dep} "$sbatchfile" | awk '{print $4}')
        recordJob "${ID_DIAG}" "Job to create diagnostic plots, with flags \"${dep}\""
    else
        echo "Would submit job to create diagnostic plots, with slurm file $sbatchfile"
    fi

fi


