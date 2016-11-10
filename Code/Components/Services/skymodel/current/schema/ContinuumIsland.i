    // @brief Scheduling Block identifier
    // @units none
    long sb_id;

    // @brief Number of discrete components extracted from the island
    // @units none
    int n_components;

    // @brief J2000 right ascension in decimal degrees
    // @units deg
    double ra_deg_cont;

    // @brief J2000 declination in decimal degrees
    // @units deg
    double dec_deg_cont;

    // @brief Frequency 
    // @units MHz
    float freq;

    // @brief Major axis determined from detected pixels
    // @units arcsec
    float maj_axis;

    // @brief Minor axis determined from detected pixels
    // @units arcsec
    float min_axis;

    // @brief Position angle of major axis (East of North)
    // @units deg
    float pos_ang;

    // @brief Integrated flux density
    // @units mJy
    float flux_int;

    // @brief Peak flux density 
    // @units mJy/beam
    float flux_peak;

    // @brief Minimum x pixel
    // @units none
    int x_min;

    // @brief maximum x pixel
    // @units none
    int x_max;

    // @brief Minimum y pixel
    // @units none
    int y_min;

    // @brief Maximum y pixel
    // @units none
    int y_max;

    // @brief Number of pixels above threshold
    // @units none
    int n_pix;

    // @brief Average x pixel
    // @units none
    float x_ave;

    // @brief Average y pixel
    // @units none
    float y_ave;

    // @brief Flux weighted centroid x pixel
    // @units none
    float x_cen;

    // @brief Flux weighted centroid y pixel
    // @units none
    float y_cen;

    // @brief x pixel corresponding to peak flux density 
    // @units none
    int x_peak;

    // @brief y pixel corresponding to peak flux density 
    // @units none
    int y_peak;

    // @brief Placeholder flag1
    // @units none
    int flag_c1;

    // @brief Placeholder flag2
    // @units none
    int flag_c2;

    // @brief Placeholder flag3
    // @units none
    int flag_c3;

    // @brief Placeholder flag4
    // @units none
    int flag_c4;

