/// @brief The HEALPix index of this component
/// UCD: 
boost::int64_t healpix_index;

/// @brief J2000 right ascension (deg)
/// UCD: pos.eq.ra;meta.main
double ra;

/// @brief J2000 declination (deg)
/// UCD: pos.eq.dec;meta.main
double dec;

/// @brief Error in Right Ascension (arcsec)
/// UCD: stat.error;pos.eq.ra
float ra_err;

/// @brief Error in Declination (arcsec)
/// UCD: stat.error;pos.eq.dec
float dec_err;

/// @brief Frequency (MHz)
/// UCD: em.freq
float freq;

/// @brief Peak flux density (mJy/beam)
/// UCD: phot.flux.density;stat.max;em.radio;stat.fit
float flux_peak;

/// @brief Error in peak flux density (mJy/beam)
/// UCD: stat.error;phot.flux.density;stat.max;em.radio;stat.fit
float flux_peak_err;

/// @brief Integrated flux density (mJy)
/// UCD: phot.flux.density;em.radio;stat.fit
float flux_int;

/// @brief Error in integrated flux density (mJy)
/// UCD: stat.error;phot.flux.density;em.radio;stat.fit
float flux_int_err;

/// @brief Spectral index (First Taylor term)
/// UCD: spect.index;em.radio
float spectral_index;

/// @brief Spectral curvature (Second Taylor term)
/// UCD: askap:spect.curvature;em.radio
float spectral_curvature;

/// @brief Reference wavelength squared (m^2)
/// UCD: askap:em.wl.squared
double lambda_ref_sq;

/// @brief Peak polarised intensity from a three-point parabolic fit (mJy/beam)
/// UCD: phot.flux.density;phys.polarization.rotMeasure;stat.max;stat.fit
double pol_peak_fit;

/// @brief Peak polarised intensity, corrected for bias, from a three-point parabolic fit (mJy/beam)
/// UCD: phot.flux.density;phys.polarization.rotMeasure;stat.max;stat.fit;askap:meta.corrected
double pol_peak_fit_debias;

/// @brief Uncertainty in pol_peak_fit (mJy/beam)
/// UCD: stat.error;phot.flux.density;phys.polarization.rotMeasure;stat.max;stat.fit
double pol_peak_fit_err;

/// @brief Signal-to-noise ratio of the peak polarisation
/// UCD: stat.snr;phot.flux.density;phys.polarization.rotMeasure;stat.max;stat.fit
double pol_peak_fit_snr;

/// @brief Uncertainty in pol_peak_fit_snr
/// UCD: stat.error;stat.snr;phot.flux.density;phys.polarization.rotMeasure;stat.max;stat.fit
double pol_peak_fit_snr_err;

/// @brief Faraday Depth from fit to peak in Faraday Dispersion Function (rad/m^2)
/// UCD: phys.polarization.rotMeasure;stat.fit
double fd_peak_fit;

/// @brief uncertainty in fd_peak_fit (rad/m^2)
/// UCD: stat.error;phys.polarization.rotMeasure;stat.fit
double fd_peak_fit_err;

