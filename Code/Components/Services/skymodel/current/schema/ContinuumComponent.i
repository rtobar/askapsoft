/// @brief The observation date (Posix Date-time)
#pragma db index
#pragma db null
boost::posix_time::ptime observation_date;

/// @brief The HEALPix index of this component
#pragma db index
#pragma db not_null
boost::int64_t healpix_index;

/// @brief Scheduling Block identifier
#pragma db index
#pragma db null
boost::int64_t sb_id;

/// @brief Component identifier
#pragma db null
std::string component_id;

/// @brief J2000 right ascension (deg)
#pragma db not_null
double ra_deg_cont;

/// @brief J2000 declination (deg)
#pragma db not_null
double dec_deg_cont;

/// @brief Error in Right Ascension (arcsec)
#pragma db not_null
float ra_err;

/// @brief Error in Declination (arcsec)
#pragma db not_null
float dec_err;

/// @brief Frequency (MHz)
#pragma db not_null
float freq;

/// @brief Peak flux density (mJy/beam)
#pragma db not_null
float flux_peak;

/// @brief Error in peak flux density (mJy/beam)
#pragma db not_null
float flux_peak_err;

/// @brief Integrated flux density (mJy)
#pragma db not_null
float flux_int;

/// @brief Error in integrated flux density (mJy)
#pragma db not_null
float flux_int_err;

/// @brief FWHM major axis before deconvolution (arcsec)
#pragma db not_null
float maj_axis;

/// @brief FWHM minor axis before deconvolution (arcsec)
#pragma db not_null
float min_axis;

/// @brief Position angle before deconvolution (deg)
#pragma db not_null
float pos_ang;

/// @brief Error in major axis before deconvolution (arcsec)
#pragma db not_null
float maj_axis_err;

/// @brief Error in minor axis before deconvolution (arcsec)
#pragma db not_null
float min_axis_err;

/// @brief Error in position angle before deconvolution (deg)
#pragma db not_null
float pos_ang_err;

/// @brief FWHM major axis after deconvolution (arcsec)
#pragma db not_null
float maj_axis_deconv;

/// @brief FWHM minor axis after deconvolution (arcsec)
#pragma db not_null
float min_axis_deconv;

/// @brief Position angle after deconvolution (deg)
#pragma db not_null
float pos_ang_deconv;

/// @brief Chi-squared value of Gaussian fit
#pragma db not_null
float chi_squared_fit;

/// @brief RMS residual of Gaussian fit (mJy/beam)
#pragma db not_null
float rms_fit_Gauss;

/// @brief Spectral index (First Taylor term)
#pragma db not_null
float spectral_index;

/// @brief Spectral curvature (Second Taylor term)
#pragma db not_null
float spectral_curvature;

/// @brief rms noise level in image (mJy/beam)
#pragma db not_null
float rms_image;

/// @brief Source has siblings
#pragma db not_null
bool flag_c1;

/// @brief Component parameters are initial estimate, not from fit
#pragma db not_null
bool flag_c2;

