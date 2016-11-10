// @brief Band-median value for Stokes I spectrum
// @units mJy/beam
#pragma db not_null
double flux_I_median;

// @brief Band-median value for Stokes Q spectrum
// @units mJy/beam
#pragma db not_null
double flux_Q_median;

// @brief Band-median value for Stokes U spectrum
// @units mJy/beam
#pragma db not_null
double flux_U_median;

// @brief Band-median value for Stokes V spectrum
// @units mJy/beam
#pragma db not_null
double flux_V_median;

// @brief Band-median sensitivity for Stokes I spectrum
// @units mJy/beam
#pragma db not_null
double rms_I;

// @brief Band-median sensitivity for Stokes Q spectrum
// @units mJy/beam
#pragma db not_null
double rms_Q;

// @brief Band-median sensitivity for Stokes U spectrum
// @units mJy/beam
#pragma db not_null
double rms_U;

// @brief Band-median sensitivity for Stokes V spectrum
// @units mJy/beam
#pragma db not_null
double rms_V;

// @brief First order coefficient for polynomial fit to Stokes I spectrum
// @units none
#pragma db not_null
double co_1;

// @brief Second order coefficient for polynomial fit to Stokes I spectrum
// @units none
#pragma db not_null
double co_2;

// @brief Third order coefficient for polynomial fit to Stokes I spectrum
// @units none
#pragma db not_null
double co_3;

// @brief Fourth order coefficient for polynomial fit to Stokes I spectrum 
// @units none
#pragma db not_null
double co_4;

// @brief Fifth order coefficient for polynomial fit to Stokes I spectrum
// @units none
#pragma db not_null
double co_5;

// @brief Reference wavelength squared
// @units m2
#pragma db not_null
double lambda_ref_sq;

// @brief Full-width at half maximum of the rotation measure spread function
// @units rad/m2
#pragma db not_null
double rmsf_fwhm;

// @brief Peak polarised intensity in the Faraday Dispersion Function
// @units mJy/beam
#pragma db not_null
double pol_peak;

// @brief Effective peak polarised intensity after correction for bias 
// @units mJy/beam
#pragma db not_null
double pol_peak_debias;

// @brief Uncertainty in pol_peak
// @units mJy/beam
#pragma db not_null
double pol_peak_err;

// @brief Peak polarised intensity from a three-point parabolic fit 
// @units mJy/beam
#pragma db not_null
double pol_peak_fit;

// @brief Peak polarised intensity, corrected for bias, from a three-point parabolic fit 
// @units mJy/beam
#pragma db not_null
double pol_peak_fit_debias;

// @brief Uncertainty in pol_peak_fit 
// @units mJy/beam
#pragma db not_null
double pol_peak_fit_err;

// @brief Signal-to-noise ratio of the peak polarisation
// @units none
#pragma db not_null
double pol_peak_fit_snr;

// @brief Uncertainty in pol_peak_fit_snr
// @units none
#pragma db not_null
double pol_peak_fit_snr_err;

// @brief Faraday Depth from the channel with the peak of the Faraday Dispersion Function
// @units rad/m2
#pragma db not_null
double fd_peak;

// @brief Uncertainty in far_depth_peak
// @units rad/m2
#pragma db not_null
double fd_peak_err;

// @brief Faraday Depth from fit to peak in Faraday Dispersion Function
// @units rad/m2
#pragma db not_null
double fd_peak_fit;

// @brief uncertainty in fd_peak_fit
// @units rad/m2
#pragma db not_null
double fd_peak_fit_err;

// @brief Polarisation angle at the reference wavelength
// @units deg
#pragma db not_null
double pol_ang_ref;

// @brief Uncertainty in pol_ang_ref
// @units deg
#pragma db not_null
double pol_ang_ref_err;

// @brief Polarisation angle de-rotated to zero wavelength
// @units deg
#pragma db not_null
double pol_ang_zero;

// @brief Uncertainty in pol_ang_zero
// @units deg
#pragma db not_null
double pol_ang_zero_err;

// @brief Fractional polarisation
// @units none
#pragma db not_null
double pol_frac;

// @brief Uncertainty in fractional polarisation
// @units none
#pragma db not_null
double pol_frac_err;

// @brief Statistical measure of polarisation complexity
// @units none
#pragma db not_null
double complex_1;

// @brief Statistical measure of polarisation complexity after removal of a thin-screen model. 
// @units none
#pragma db not_null
double complex_2;

// @brief True if pol_peak_fit is above a threshold value otherwise pol_peak_fit is an upper limit. 
// @units none
#pragma db not_null
bool flag_p1;

// @brief True if FDF peak is close to edge
// @units none
#pragma db not_null
bool flag_p2;

// @brief placeholder flag
// @units none
#pragma db not_null
bool flag_p3;

// @brief placeholder flag
// @units none
#pragma db not_null
bool flag_p4;

