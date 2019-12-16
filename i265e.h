/*****************************************************************************
 * Copyright (C) 2017-2020 Ingenic Semiconductor CO.,LTD.
 *
 * Authors: Justin Kang <pengtao.kang@ingenic.com>
 *
 *****************************************************************************/

#ifndef __I265E_H__
#define __I265E_H__

#include <stdint.h>

#include "icommon.h"

#ifdef __cplusplus
extern "C" {
#endif

#define I265E_EXTENDED_SAR       255 /* aspect ratio explicitly specified as width:height */

typedef struct i265e i265e_t;

static const char * const i265e_source_csp_names[] = { "i400", "i420", "i422", "i444", "nv12", "t420", 0 };

typedef enum {
	I265E_RC_CQP                = 0,
	I265E_RC_CBR                = 1,
	I265E_RC_VBR                = 2,
	I265E_RC_SMART              = 3,
    I265E_RC_INV                = 4,
} i265e_rc_method_t;

typedef enum {
    I265E_AQ_NONE                   = 0,
    I265E_AQ_VARIANCE               = 1,
    I265E_AQ_AUTO_VARIANCE          = 2,
    I265E_AQ_AUTO_VARIANCE_BIASED   = 3,
} i265e_aq_mode_t;

typedef enum {
    I265E_REF_LIMIT_ALL             = 0,
    I265E_REF_LIMIT_DEPTH           = 1,
    I265E_REF_LIMIT_CU              = 2,
} i265e_ref_limit_depth_t;

typedef enum {
    I265E_NAL_CODED_SLICE_TRAIL_N = 0,
    I265E_NAL_CODED_SLICE_TRAIL_R,
    I265E_NAL_CODED_SLICE_TSA_N,
    I265E_NAL_CODED_SLICE_TLA_R,
    I265E_NAL_CODED_SLICE_STSA_N,
    I265E_NAL_CODED_SLICE_STSA_R,
    I265E_NAL_CODED_SLICE_RADL_N,
    I265E_NAL_CODED_SLICE_RADL_R,
    I265E_NAL_CODED_SLICE_RASL_N,
    I265E_NAL_CODED_SLICE_RASL_R,
    I265E_NAL_CODED_SLICE_BLA_W_LP = 16,
    I265E_NAL_CODED_SLICE_BLA_W_RADL,
    I265E_NAL_CODED_SLICE_BLA_N_LP,
    I265E_NAL_CODED_SLICE_IDR_W_RADL,
    I265E_NAL_CODED_SLICE_IDR_N_LP,
    I265E_NAL_CODED_SLICE_CRA,
    I265E_NAL_VPS = 32,
    I265E_NAL_SPS,
    I265E_NAL_PPS,
    I265E_NAL_ACCESS_UNIT_DELIMITER,
    I265E_NAL_EOS,
    I265E_NAL_EOB,
    I265E_NAL_FILLER_DATA,
    I265E_NAL_PREFIX_SEI,
    I265E_NAL_SUFFIX_SEI,
    I265E_NAL_INVALID = 64,
} i265e_nal_type_t;

/* The data within the payload is already NAL-encapsulated; the type is merely
 * in the struct for easy access by the calling application.  All data returned
 * in an i265e_nal_t, including the data in payload, is no longer valid after the
 * next call to i265e_encode.  Thus it must be used or copied before
 * calling i265e_encode again. */
typedef struct {
    uint32_t    i_type;        /* i265e_nal_type_t */
    int         i_payload;   /* size in bytes */
    uint8_t*    p_payload;
} i265e_nal_t;

/* Arbitrary User SEI
 * Payload size is in bytes and the payload pointer must be non-NULL.
 * Payload types and syntax can be found in Annex D of the H.265 Specification.
 * SEI Payload Alignment bits as described in Annex D must be included at the
 * end of the payload if needed. The payload should not be NAL-encapsulated.
 * Payloads are written in the order of input */

typedef enum {
    I265E_SEI_BUFFERING_PERIOD                     = 0,
    I265E_SEI_PICTURE_TIMING                       = 1,
    I265E_SEI_PAN_SCAN_RECT                        = 2,
    I265E_SEI_FILLER_PAYLOAD                       = 3,
    I265E_SEI_USER_DATA_REGISTERED_ITU_T_T35       = 4,
    I265E_SEI_USER_DATA_UNREGISTERED               = 5,
    I265E_SEI_RECOVERY_POINT                       = 6,
    I265E_SEI_SCENE_INFO                           = 9,
    I265E_SEI_FULL_FRAME_SNAPSHOT                  = 15,
    I265E_SEI_PROGRESSIVE_REFINEMENT_SEGMENT_START = 16,
    I265E_SEI_PROGRESSIVE_REFINEMENT_SEGMENT_END   = 17,
    I265E_SEI_FILM_GRAIN_CHARACTERISTICS           = 19,
    I265E_SEI_POST_FILTER_HINT                     = 22,
    I265E_SEI_TONE_MAPPING_INFO                    = 23,
    I265E_SEI_FRAME_PACKING                        = 45,
    I265E_SEI_DISPLAY_ORIENTATION                  = 47,
    I265E_SEI_SOP_DESCRIPTION                      = 128,
    I265E_SEI_ACTIVE_PARAMETER_SETS                = 129,
    I265E_SEI_DECODING_UNIT_INFO                   = 130,
    I265E_SEI_TEMPORAL_LEVEL0_INDEX                = 131,
    I265E_SEI_DECODED_PICTURE_HASH                 = 132,
    I265E_SEI_SCALABLE_NESTING                     = 133,
    I265E_SEI_REGION_REFRESH_INFO                  = 134,
    I265E_SEI_MASTERING_DISPLAY_INFO               = 137,
    I265E_SEI_CONTENT_LIGHT_LEVEL_INFO             = 144,
} i265_sei_payload_type_t;

typedef struct {
    int                     payloadSize;
    i265_sei_payload_type_t payloadType;
    uint8_t                 *payload;
	void					*releaseData;
} i265e_sei_payload_t;

typedef struct {
    int                 numPayloads;
    i265e_sei_payload_t *payloads;
    int					(*releaseFunc)(void *releasePriv, void *releaseData);
    void				*releasePriv;
} i265e_sei_t;

/* Used to pass pictures into the encoder, and to get picture data back out of
 * the encoder.  The input and output semantics are different */
typedef struct {
    /* presentation time stamp: user-specified, returned on output */
    int64_t     pts;
    /* return this pic real coded qp */
    int         qp;
	int64_t     timestamp;
    int         bForceIDR;
    c_img_t     img;
	c_fsktype_t fsktype;
    /* User defined SEI */
    i265e_sei_t userSEI;
    /* extra nals buf variable point */
	uint8_t     **nalsBuffer;
    uint32_t    nalsBufSize;

    int        (*releaseFunc)(void *privData, void *releaseData);
    void        *releaseData;

    void        *privData;
} i265e_pic_t;

typedef enum {
    I265E_DIA_SEARCH,
    I265E_HEX_SEARCH,
    I265E_UMH_SEARCH,
    I265E_STAR_SEARCH,
    I265E_SEA_SEARCH,
    I265E_FULL_SEARCH,
} i265e_me_method_t;

typedef enum {
    I265E_TYPE_AUTO     = 0,  /* Let i265e choose the right type */
    I265E_TYPE_IDR      = 1,
    I265E_TYPE_I        = 2,
    I265E_TYPE_P        = 3,
    I265E_TYPE_BREF     = 4,  /* Non-disposable B-frame */
    I265E_TYPE_B        = 5,
} i265e_frame_type_t;

#define IS_I265E_TYPE_I(x) ((x) == I265E_TYPE_I || (x) == I265E_TYPE_IDR)
#define IS_I265E_TYPE_B(x) ((x) == I265E_TYPE_B || (x) == I265E_TYPE_BREF)

typedef struct {
    c_soc_type_t    socType;
    bool            bUserNalbuf;
    bool            bWaterMark;
    uint32_t        taskNum;
    uint32_t        threadNum;

    /* frame color to gray Properties */
    bool            bEnableColor2Gray;

    /* frame control Properties */
    uint32_t        outFpsNum;
    uint32_t        outFpsDen;
    uint32_t        gopSize;

    /*== Internal Picture Specification ==*/
    int             totalFrames;
    /* Color space of internal pictures, must match color space of input
     * pictures */
    c_csp_t         internalCsp;
    /* Width (in pixels) of the source pictures. If this width is not an even
     * multiple of 4, the encoder will pad the pictures internally to meet this
     * minimum requirement. All valid HEVC widths are supported */
    uint32_t        sourceWidth;
    /* Height (in pixels) of the source pictures. If this height is not an even
     * multiple of 4, the encoder will pad the pictures internally to meet this
     * minimum requirement. All valid HEVC heights are supported */
    uint32_t        sourceHeight;
    uint32_t        sourceMaxWidth;
    uint32_t        sourceMaxHeight;
    /* Interlace type of source pictures. 0 - progressive pictures (default).
     * 1 - top field first, 2 - bottom field first. HEVC encodes interlaced
     * content as fields, they must be provided to the encoder in the correct
     * temporal order */
    bool            interlaceMode;

    /* Enable wavefront parallel processing, greatly increases parallelism for
     * less than 1% compression efficiency loss. Requires a thread pool, enabled
     * by default */
    bool            bEnableWavefront;

    /*ratecontrol Properties*/
    /* Generally a small signed integer which offsets the QP used to quantize
     * the Cb chroma residual (delta from luma QP specified by rate-control).
     * Default is 0, which is recommended */
    int             cbQpOffset;

    /* Generally a small signed integer which offsets the QP used to quantize
     * the Cr chroma residual (delta from luma QP specified by rate-control).
     * Default is 0, which is recommended */
    int             crQpOffset;
    struct
    {
        /* Explicit mode of rate-control, necessary for API users. It must
         * be one of the I265E_RC_METHODS enum values. */
        int         rateControlMode;

        /* Base QP to use for Constant QP rate control. Adaptive QP may alter
         * the QP used for each block. If a QP is specified on the command line
         * CQP rate control is implied. Default: 32 */
        int         qp;

        /* target bitrate for Average BitRate (ABR) rate control. If a non- zero
         * bitrate is specified on the command line, ABR is implied. Default 0 */
        int         bitrate;

        /* qComp sets the quantizer curve compression factor. It weights the frame
         * quantizer based on the complexity of residual (measured by lookahead).
         * Default value is 0.6. Increasing it to 1 will effectively generate CQP */
        double      qCompress;

        /* QP offset between I/P and P/B frames. Default ipfactor: 1.4
         * Default pbFactor: 1.3 */
        double      ipFactor;
        double      pbFactor;

        /* Max QP difference between frames. Default: 3 */
        int         qpStep;

        /* Max QP difference between gops. Default: 15 */
        int         gopStep;

        /* Enable adaptive quantization. This mode distributes available bits between all
         * CTUs of a frame, assigning more bits to low complexity areas. Turning
         * this ON will usually affect PSNR negatively, however SSIM and visual quality
         * generally improves. Default: I265E_AQ_VARIANCE */
        int         aqMode;

        /* Sets the strength of AQ bias towards low detail CTUs. Valid only if
         * AQ is enabled. Default value: 1.0. Acceptable values between 0.0 and 3.0 */
        double      aqStrength;

        /* Sets the maximum rate the VBV buffer should be assumed to refill at
         * Default is zero */
        int         vbvMaxBitrate;

        /* Sets the size of the VBV buffer in kilobits. Default is zero */
        int         vbvBufferSize;

        /* Sets how full the VBV buffer must be before playback starts. If it is less than
         * 1, then the initial fill is vbv-init * vbvBufferSize. Otherwise, it is
         * interpreted as the initial fill in kbits. Default is 0.9 */
        double      vbvBufferInit;

        /* Enable adaptive quantization at CU granularity. This parameter specifies
         * the minimum CU size at which QP can be adjusted, i.e. Quantization Group
         * (QG) size. Allowed values are 64, 32, 16, 8 provided it falls within the
         * inclusuve range [maxCUSize, minCUSize]. Experimental, default: maxCUSize */
        uint32_t    qgSize;

        /* sets a hard upper limit on QP */
        int         qpMax;

        /* sets a hard lower limit on QP */
        int         qpMin;

        /* sets ip bias */
        int         ibias;

        /* ratecontrol time unit */
        uint32_t    staticTime;

        /* fluncture level*/
        uint32_t    flucLvl;

        /* qpgmode*/
        int         qpgMode;

        /* start to adjust qp level */
        uint32_t    changePos;

        /* quality level */
        uint32_t    qualityLvl;
    } rc;

    c_roi_t         roi[16];

    /* skip control parameters */
    c_high_skip_t   hskip;
    int             maxHSkipType;

    /* super frame parameters */
    c_superfrm_param_t superFrm;

    /*== Video Usability Information ==*/
    struct
    {
        /* Aspect ratio idc to be added to the VUI.  The default is 0 indicating
         * the apsect ratio is unspecified. If set to I265E_EXTENDED_SAR then
         * sarWidth and sarHeight must also be set */
        int aspectRatioIdc;

        /* Sample Aspect Ratio width in arbitrary units to be added to the VUI
         * only if aspectRatioIdc is set to I265E_EXTENDED_SAR.  This is the width
         * of an individual pixel. If this is set then sarHeight must also be set */
        int sarWidth;

        /* Sample Aspect Ratio height in arbitrary units to be added to the VUI.
         * only if aspectRatioIdc is set to I265E_EXTENDED_SAR.  This is the width
         * of an individual pixel. If this is set then sarWidth must also be set */
        int sarHeight;

        /* Enable overscan info present flag in the VUI.  If this is set then
         * bEnabledOverscanAppropriateFlag will be added to the VUI. The default
         * is false */
        bool bEnableOverscanInfoPresentFlag;

        /* Enable overscan appropriate flag.  The status of this flag is added
         * to the VUI only if bEnableOverscanInfoPresentFlag is set. If this
         * flag is set then cropped decoded pictures may be output for display.
         * The default is false */
        bool bEnableOverscanAppropriateFlag;

        /* Video signal type present flag of the VUI.  If this is set then
         * videoFormat, bEnableVideoFullRangeFlag and
         * bEnableColorDescriptionPresentFlag will be added to the VUI. The
         * default is false */
        bool bEnableVideoSignalTypePresentFlag;

        /* Video format of the source video.  0 = component, 1 = PAL, 2 = NTSC,
         * 3 = SECAM, 4 = MAC, 5 = unspecified video format is the default */
        int videoFormat;

        /* Video full range flag indicates the black level and range of the luma
         * and chroma signals as derived from E′Y, E′PB, and E′PR or E′R, E′G,
         * and E′B real-valued component signals. The default is false */
        bool bEnableVideoFullRangeFlag;

        /* Color description present flag in the VUI. If this is set then
         * color_primaries, transfer_characteristics and matrix_coeffs are to be
         * added to the VUI. The default is false */
        bool bEnableColorDescriptionPresentFlag;

        /* Color primaries holds the chromacity coordinates of the source
         * primaries. The default is 2 */
        int colorPrimaries;

        /* Transfer characteristics indicates the opto-electronic transfer
         * characteristic of the source picture. The default is 2 */
        int transferCharacteristics;

        /* Matrix coefficients used to derive the luma and chroma signals from
         * the red, blue and green primaries. The default is 2 */
        int matrixCoeffs;

        /* Chroma location info present flag adds chroma_sample_loc_type_top_field and
         * chroma_sample_loc_type_bottom_field to the VUI. The default is false */
        bool bEnableChromaLocInfoPresentFlag;

        /* Chroma sample location type top field holds the chroma location in
         * the top field. The default is 0 */
        int chromaSampleLocTypeTopField;

        /* Chroma sample location type bottom field holds the chroma location in
         * the bottom field. The default is 0 */
        int chromaSampleLocTypeBottomField;

        /* Default display window flag adds def_disp_win_left_offset,
         * def_disp_win_right_offset, def_disp_win_top_offset and
         * def_disp_win_bottom_offset to the VUI. The default is false */
        bool bEnableDefaultDisplayWindowFlag;

        /* Default display window left offset holds the left offset with the
         * conformance cropping window to further crop the displayed window */
        int defDispWinLeftOffset;

        /* Default display window right offset holds the right offset with the
         * conformance cropping window to further crop the displayed window */
        int defDispWinRightOffset;

        /* Default display window top offset holds the top offset with the
         * conformance cropping window to further crop the displayed window */
        int defDispWinTopOffset;

        /* Default display window bottom offset holds the bottom offset with the
         * conformance cropping window to further crop the displayed window */
        int defDispWinBottomOffset;
    } vui;

    /* Maximum Content light level(MaxCLL), specified as integer that indicates the
     * maximum pixel intensity level in units of 1 candela per square metre of the
     * bitstream. i265e will also calculate MaxCLL programmatically from the input
     * pixel values and set in the Content light level info SEI */
    uint32_t        maxCLL;
    /* Maximum Frame Average Light Level(MaxFALL), specified as integer that indicates
     * the maximum frame average intensity level in units of 1 candela per square
     * metre of the bitstream. i265e will also calculate MaxFALL programmatically
     * from the input pixel values and set in the Content light level info SEI */
    uint32_t        maxFALL;
    /* Maximum of the picture order count */
    int             log2MaxPocLsb;
    /* Maximum count of Slices of picture, the value range is [1, maximum rows] */
    uint32_t        maxSlices;
    /* Emit VUI Timing info, an optional VUI field */
    bool            bEmitVUITimingInfo;
    /* Emit HRD Timing info */
    bool            bEmitVUIHRDInfo;
    /* Adaptive Quantization based on relative motion */
    bool            bAQMotion;
    /* Enables the buffering period SEI and picture timing SEI to signal the HRD
     * parameters. Default is disabled */
    bool            bEmitHRDSEI;
    /* Enable luma and chroma offsets for HDR/WCG content.
     * Default is disabled */
    bool            bHDROpt;

    /*== Profile / Tier / Level ==*/

    /* Note: the profile is specified by i265e_param_apply_profile() */

    /* Minimum decoder requirement level. Defaults to 0, which implies auto-
     * detection by the encoder. If specified, the encoder will attempt to bring
     * the encode specifications within that specified level. If the encoder is
     * unable to reach the level it issues a warning and emits the actual
     * decoder requirement. If the requested requirement level is higher than
     * the actual level, the actual requirement level is signaled. The value is
     * an specified as an integer with the level times 10, for example level
     * "5.1" is specified as 51, and level "5.0" is specified as 50. */
    int       levelIdc;

    /* if levelIdc is specified (non-zero) this flag will differentiate between
     * Main (0) and High (1) tier. Default is Main tier (0) */
    bool      bHighTier;

    /* The maximum number of L0 references a P or B slice may use. This
     * influences the size of the decoded picture buffer. The higher this
     * number, the more reference frames there will be available for motion
     * search, improving compression efficiency of most video at a cost of
     * performance. Value must be between 1 and 16, default is 3 */
    int       maxNumReferences;

    /* Bitstream Options */
    /* Flag indicating whether VPS, SPS and PPS headers should be output with
     * each keyframe. Default true */
    bool            bRepeatHeaders;
    /* Enable Temporal Sub Layers while encoding, signals NAL units of coded
     * slices with their temporalId. Output bitstreams can be extracted either
     * at the base temporal layer (layer 0) with roughly half the frame rate or
     * at a higher temporal layer (layer 1) that decodes all the frames in the
     * sequence. */
    bool            bEnableTemporalSubLayers;

    /*== GOP structure and slice type decisions (lookahead) ==*/
    /* Scene cuts closer together than this are coded as I, not IDR. */
    uint32_t        keyframeMin;

    /* Maximum keyframe distance or intra period in number of frames. If 0 or 1,
     * all frames are I frames. A negative value is casted to MAX_INT internally
     * which effectively makes frame 0 the only I frame. Default is 250 */
    uint32_t        keyframeMax;

    /*== Coding Unit (CU) definitions ==*/

    /* Maximum CU width and height in pixels.  The size must be 64, 32, or 16.
     * The higher the size, the more efficiently i265e can encode areas of low
     * complexity, greatly improving compression efficiency at large
     * resolutions.  The smaller the size, the more effective wavefront and
     * frame parallelism will become because of the increase in rows. default 64
     * All encoders within the same process must use the same maxCUSize, until
     * all encoders are closed and i265e_cleanup() is called to reset the value. */
    uint32_t        maxCUSize;

    /* Minimum CU width and height in pixels.  The size must be 64, 32, 16, or
     * 8. Default 8. All encoders within the same process must use the same
     * minCUSize. */
    uint32_t        minCUSize;

    /* Enable rectangular motion prediction partitions (vertical and
     * horizontal), available at all CU depths from 64x64 to 8x8. Default is
     * disabled */
    bool            bEnableRectInter;

    /* Enable asymmetrical motion predictions.  At CU depths 64, 32, and 16, it
     * is possible to use 25%/75% split partitions in the up, down, right, left
     * directions. For some material this can improve compression efficiency at
     * the cost of extra analysis. bEnableRectInter must be enabled for this
     * feature to be used. Default disabled */
    bool            bEnableAMP;

    /*== Residual Quadtree Transform Unit (TU) definitions ==*/

    /* Maximum TU width and height in pixels. The size must be 32, 16, 8 or 4.
     * The larger the size the more efficiently the residual can be compressed
     * by the DCT transforms, at the expense of more computation */
    uint32_t        maxTUSize;

    /* The additional depth the residual quad-tree is allowed to recurse beyond
     * the coding quad-tree, for inter coded blocks. This must be between 1 and
     * 4. The higher the value the more efficiently the residual can be
     * compressed by the DCT transforms, at the expense of much more compute */
    uint32_t        tuQTMaxInterDepth;

    /* The additional depth the residual quad-tree is allowed to recurse beyond
     * the coding quad-tree, for intra coded blocks. This must be between 1 and
     * 4. The higher the value the more efficiently the residual can be
     * compressed by the DCT transforms, at the expense of much more compute */
    uint32_t        tuQTMaxIntraDepth;

    /* Enable early exit decisions for inter coded blocks to avoid recursing to
     * higher TU depths. Default: 0 */
    uint32_t        limitTU;

    /* Set the amount of rate-distortion analysis to use within quant. 0 implies
     * no rate-distortion optimization. At level 1 rate-distortion cost is used to
     * find optimal rounding values for each level (and allows psy-rdoq to be
     * enabled). At level 2 rate-distortion cost is used to make decimate decisions
     * on each 4x4 coding group (including the cost of signaling the group within
     * the group bitmap).  Psy-rdoq is less effective at preserving energy when
     * RDOQ is at level 2. Default: 0 */
    int             rdoqLevel;

    /* Enable the implicit signaling of the sign bit of the last coefficient of
     * each transform unit. This saves one bit per TU at the expense of figuring
     * out which coefficient can be toggled with the least distortion.
     * Default is enabled */
    bool            bEnableSignHiding;

    /* Allow intra coded blocks to be encoded directly as residual without the
     * DCT transform, when this improves efficiency. Checking whether the block
     * will benefit from this option incurs a performance penalty. Default is
     * disabled */
    bool            bEnableTransformSkip;

    /*== Intra Coding Tools ==*/

    /* Enable constrained intra prediction. This causes intra prediction to
     * input samples that were inter predicted. For some use cases this is
     * believed to me more robust to stream errors, but it has a compression
     * penalty on P and (particularly) B slices. Defaults to disabled */
    bool            bEnableConstrainedIntra;

    /* Enable strong intra smoothing for 32x32 blocks where the reference
     * samples are flat. It may or may not improve compression efficiency,
     * depending on your source material. Defaults to disabled */
    bool            bEnableStrongIntraSmoothing;

    /* Use a faster search method to find the best intra mode. Default is 0 */
    bool            bEnableFastIntra;

    /*== Inter Coding Tools ==*/

    /* The maximum number of merge candidates that are considered during inter
     * analysis.  This number (between 1 and 5) is signaled in the stream
     * headers and determines the number of bits required to signal a merge so
     * it can have significant trade-offs. The smaller this number the higher
     * the performance but the less compression efficiency. Default is 3 */
    uint32_t        maxNumMergeCand;

    /* Limit the motion references used for each search based on the results of
     * previous motion searches already performed for the same CU: If 0 all
     * references are always searched. If I265E_REF_LIMIT_CU all motion searches
     * will restrict themselves to the references selected by the 2Nx2N search
     * at the same depth. If I265E_REF_LIMIT_DEPTH the 2Nx2N motion search will
     * only use references that were selected by the best motion searches of the
     * 4 split CUs at the next lower CU depth.  The two flags may be combined */
    uint32_t        limitReferences;

    /* Limit modes analyzed for each CU using cost metrics from the 4 sub-CUs */
    uint32_t        limitModes;

    /* ME search method (DIA, HEX, UMH, STAR, SEA, FULL). The search patterns
     * (methods) are sorted in increasing complexity, with diamond being the
     * simplest and fastest and full being the slowest.  DIA, HEX, UMH and SEA were
     * adapted from x264 directly. STAR is an adaption of the HEVC reference
     * encoder's three step search, while full is a naive exhaustive search. The
     * default is the star search, it has a good balance of performance and
     * compression efficiency */
    int             searchMethod;

    /* A value between 0 and 7 which adjusts the amount of
     * effort performed during sub-pel refine. Default is 5 */
    int             subpelRefine;

    /* The maximum distance from the motion prediction that the full pel motion
     * search is allowed to progress before terminating. This value can have an
     * effect on frame parallelism, as referenced frames must be at least this
     * many rows of reconstructed pixels ahead of the referencee at all times.
     * (When considering reference lag, the motion prediction must be ignored
     * because it cannot be known ahead of time).  Default is 60, which is the
     * default max CU size (64) minus the luma HPEL half-filter length (4). If a
     * smaller CU size is used, the search range should be similarly reduced */
    int             searchRange;

    /* Enable availability of temporal motion vector for AMVP, default is enabled */
    bool            bEnableTemporalMvp;

    /* Enable weighted prediction in P slices.  This enables weighting analysis
     * in the lookahead, which influences slice decisions, and enables weighting
     * analysis in the main encoder which allows P reference samples to have a
     * weight function applied to them prior to using them for motion
     * compensation.  In video which has lighting changes, it can give a large
     * improvement in compression efficiency. Default is enabled */
    bool            bEnableWeightedPred;

    /* Enable weighted prediction in B slices. Default is disabled */
    bool            bEnableWeightedBiPred;

    /*== Loop Filters ==*/
    /* Enable the deblocking loop filter, which improves visual quality by
     * reducing blocking effects at block edges, particularly at lower bitrates
     * or higher QP. When enabled it adds another CU row of reference lag,
     * reducing frame parallelism effectiveness. Default is enabled */
    bool            bEnableLoopFilter;

    /* deblocking filter tC offset [-6, 6] -6 light filter, 6 strong.
     * This is the coded div2 value, actual offset is doubled at use */
    int             deblockingFilterTCOffset;

    /* deblocking filter Beta offset [-6, 6] -6 light filter, 6 strong
     * This is the coded div2 value, actual offset is doubled at use */
    int             deblockingFilterBetaOffset;

    /* SAO Loop Filter */
    /* Enable the Sample Adaptive Offset loop filter, which reduces distortion
     * effects by adjusting reconstructed sample values based on histogram
     * analysis to better approximate the original samples. When enabled it adds
     * a CU row of reference lag, reducing frame parallelism effectiveness.
     * Default is enabled */
    bool            bEnableSAO;

    /* Note: when deblocking and SAO are both enabled, the loop filter CU lag is
     * only one row, as they operate in series on the same row. */

    /* Select the method in which SAO deals with deblocking boundary pixels. If
     * disabled the right and bottom boundary areas are skipped. If enabled,
     * non-deblocked pixels are used entirely. Default is disabled */
    bool            bSaoNonDeblocked;

    /*== Analysis tools ==*/

    /* A value between 1 and 6 (both inclusive) which determines the level of
     * rate distortion optimizations to perform during mode and depth decisions.
     * The more RDO the better the compression efficiency at a major cost of
     * performance. Default is 3 */
    int             rdLevel;

    /* debug picture */
    int             fdEnc;
    int             fdRef;
    int             fdBs;
    int             fdDec;

    /* log param */
    void            (*pf_log)(const char *module, int i_level, const char *psz, va_list);
    const char *    module;
    /* Enable the measurement and reporting of PSNR. Default is enabled */
    bool            bEnablePsnr;
    /* Enable the measurement and reporting of SSIM. Default is disabled */
    bool            bEnableSsim;
    int             logLevel;

    /* user defined memory management */
    void * (*ckMalloc)(int size, int align);
    void (*ckFree)(void *ptr);
	intptr_t (*ckVirt2Phys)(intptr_t vaddr);
	intptr_t (*ckPhys2Virt)(intptr_t paddr);

    /* Optional callback, Only used when the i265e_param_t sits in memory for an indefinite period of time */
    void (*param_free)(void *);
} i265e_param_t;

typedef enum {
	I265E_RCFG_C2G_ID	= 0,
	I265E_RCFG_CUT_ID,
	I265E_RCFG_ROI_ID,
	I265E_RCFG_RC_ID,
	I265E_RCFG_RC_TRIG_ID,  //unsupported temporally
	I265E_RCFG_FPS_ID,
	I265E_RCFG_ENIDR_ID,
	I265E_RCFG_GOP_ID,
    I265E_RCFG_DN_ID,       //unsupported temporally
	I265E_RCFG_HSKIP_ID,
	I265E_RCFG_BENHANCE_ID,
	I265E_RCFG_MBRC_ID,     //unsupported temporally, move to qpgmode
	I265E_RCFG_CHGREF_ID,   //unsupported temporally
	I265E_RCFG_SUPER_ID,
	I265E_RCFG_TRANS_ID,
    I265E_RCFG_QPGMODE_ID,
} i265e_rcfg_type_t;

typedef struct {
	int left;
	int top;
	int width;
	int height;
} i265e_rcfg_crop_param_t;

typedef struct {
	int     idx;
	c_roi_t roi;
} i265e_rcfg_roi_param_t;

typedef struct {
    i265e_rc_method_t       rcMethod;
    int                     qp;
    int                     qpMax;
    int                     qpMin;
    uint32_t                staticTime;
    int                     bitrate;
    int                     ibias;
    uint32_t                changePos;
    uint32_t                qualityLvl;
    int                     qpStep;
    int                     gopStep;
    uint32_t                flucLvl;
} i265e_rcfg_rc_param_t;

typedef struct {
	uint32_t        fpsNum;
	uint32_t        fpsDen;
} i265e_rcfg_fps_param_t;

typedef struct i265e_rcfg_trans_param {
    int             cbQpOffset;
    int             crQpOffset;
} i265e_rcfg_trans_param_t;

extern int i265e_param_default(i265e_param_t *param);
extern void i265e_param_dump(i265e_param_t *param);
extern i265e_t *i265e_init(i265e_param_t *param);
extern void i265e_deinit(i265e_t *h);
extern int i265e_encode(i265e_t *h, i265e_pic_t *pic_in);
extern int i265e_get_bitstream(i265e_t *h, i265e_nal_t **pp_nal, int *pi_nal, i265e_pic_t **pic_in, i265e_pic_t **pic_out, void **bshandler, void **thandler);
extern int i265e_release_bitstream(i265e_t *h, void *bshandler, void *thandler);
extern void i265e_flush_bitstream(i265e_t *h);
extern void i265e_get_bitstream_cleanup_route(void *h);
extern int i265e_get_param(i265e_t *h, int param_id, void *param);
extern int i265e_set_param(i265e_t *h, int param_id, const void *param);

#ifdef __cplusplus
}
#endif

#endif
