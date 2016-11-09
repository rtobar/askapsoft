    // @brief Optimistic concurrency lock version
    // @units none
    #pragma db version
    unsigned long version;

    // @brief Primary key unique identifier
    // @units none
    #pragma db index
    #pragma db id auto
    long continuum_component_id;

    // @brief Scheduling Block identifier
    // @units none
    #pragma db index
    long sb_id;

    // @brief J2000 right ascension in decimal degrees
    // @units deg
    double ra_deg_cont;

    // @brief J2000 declination in decimal degrees
    // @units deg
    double dec_deg_cont;

    // @brief Error in Right Ascension 
    // @units arcsec
    float ra_err;

    // @brief Error in Declination
    // @units arcsec
    float dec_err;

    // @brief Frequency 
    // @units MHz
    float freq;

    // @brief Peak flux density 
    // @units mJy/beam
    float flux_peak;

    // @brief Error in peak flux density
    // @units mJy/beam
    float flux_peak_err;

    // @brief Integrated flux density
    // @units mJy
    float flux_int;

    // @brief Error in integrated flux density
    // @units mJy
    float flux_int_err;

    // @brief FWHM major axis before deconvolution
    // @units arcsec
    float maj_axis;

    // @brief FWHM minor axis before deconvolution
    // @units arcsec
    float min_axis;

    // @brief Position angle before deconvolution
    // @units deg
    float pos_ang;

    // @brief Error in major axis before deconvolution
    // @units arcsec
    float maj_axis_err;

    // @brief Error in minor axis before deconvolution
    // @units arcsec
    float min_axis_err;

    // @brief Error in position angle before deconvolution
    // @units deg
    float pos_ang_err;

    // @brief FWHM major axis after deconvolution
    // @units arcsec
    float maj_axis_deconv;

    // @brief FWHM minor axis after deconvolution
    // @units arcsec
    float min_axis_deconv;

    // @brief Position angle after deconvolution
    // @units deg
    float pos_ang_deconv;

    // @brief Chi-squared value of Gaussian fit
    // @units none
    float chi_squared_fit;

    // @brief RMS residual of Gaussian fit
    // @units mJy/beam
    float rms_fit_Gauss;

    // @brief Spectral index (First Taylor term)
    // @units none
    float spectral_index;

    // @brief Spectral curvature (Second Taylor term)
    // @units none
    float spectral_curvature;

    // @brief rms noise level in image
    // @units mJy/beam
    float rms_image;

    // @brief Source has siblings
    // @units none
    int flag_c1;

    // @brief Component parameters are initial estimate, not from fit
    // @units none
    int flag_c2;

    // @brief Placeholder flag3
    // @units none
    int flag_c3;

    // @brief Placeholder flag4
    // @units none
    int flag_c4;

