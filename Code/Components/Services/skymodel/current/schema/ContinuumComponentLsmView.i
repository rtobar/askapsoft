/// @brief The HEALPix index of this component
boost::int64_t healpix_index;

/// @brief J2000 right ascension (deg)
double ra_deg_cont;

/// @brief J2000 declination (deg)
double dec_deg_cont;

/// @brief Error in Right Ascension (arcsec)
float ra_err;

/// @brief Error in Declination (arcsec)
float dec_err;

/// @brief Frequency (MHz)
float freq;

/// @brief Peak flux density (mJy/beam)
float flux_peak;

/// @brief Error in peak flux density (mJy/beam)
float flux_peak_err;

/// @brief Integrated flux density (mJy)
float flux_int;

/// @brief Error in integrated flux density (mJy)
float flux_int_err;

/// @brief Spectral index (First Taylor term)
float spectral_index;

/// @brief Spectral curvature (Second Taylor term)
float spectral_curvature;

/// @brief Reference wavelength squared (m^2)
double lambda_ref_sq;

/// @brief Peak polarised intensity from a three-point parabolic fit (mJy/beam)
double pol_peak_fit;

/// @brief Peak polarised intensity, corrected for bias, from a three-point parabolic fit (mJy/beam)
double pol_peak_fit_debias;

/// @brief Uncertainty in pol_peak_fit (mJy/beam)
double pol_peak_fit_err;

/// @brief Signal-to-noise ratio of the peak polarisation
double pol_peak_fit_snr;

/// @brief Uncertainty in pol_peak_fit_snr
double pol_peak_fit_snr_err;

/// @brief Faraday Depth from fit to peak in Faraday Dispersion Function (rad/m^2)
double fd_peak_fit;

/// @brief uncertainty in fd_peak_fit (rad/m^2)
double fd_peak_fit_err;

