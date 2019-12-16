#ifndef __ICOMMON_H__
#define __ICOMMON_H__

#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define C_BIT_DEPTH		8
#define C_PADH			16
#define C_PADV			16
#define C_VB_NUM_MAX	16
#define C_CHN_NUM_MAX	16
#define C_NATIVE_ALIGN	32
#define C_VB_ALIGN		256
#define C_PIXEL_MAX     ((1 << C_BIT_DEPTH)-1)
#define C_MAX_WH		2000
/* Unions for type-punning.
 * Mn: load or store n bits, aligned, native-endian
 * CPn: copy n bits, aligned, native-endian
 * we don't use memcpy for CPn because memcpy's args aren't assumed to be aligned */
typedef union {uint16_t i; uint8_t  c[2];} c_union16_t;
typedef union {uint32_t i; uint16_t b[2]; uint8_t  c[4];} c_union32_t;
typedef union {uint64_t i; uint32_t a[2]; uint16_t b[4]; uint8_t c[8];} c_union64_t;
typedef struct {uint64_t i[2];} c_uint128_t;
typedef union {c_uint128_t i; uint64_t a[2]; uint32_t b[4]; uint16_t c[8]; uint8_t d[16];} c_union128_t;

#define C_M16(src)			(((c_union16_t*)(src))->i)
#define C_M32(src)			(((c_union32_t*)(src))->i)
#define C_M64(src)			(((c_union64_t*)(src))->i)
#define C_M128(src)			(((c_union128_t*)(src))->i)
#define C_M128_ZERO			((c_uint128_t){{0,0}})
#define C_CP16(dst,src)		M16(dst) = M16(src)
#define C_CP32(dst,src)		M32(dst) = M32(src)
#define C_CP64(dst,src)		M64(dst) = M64(src)
#define C_CP128(dst,src)	M128(dst) = M128(src)

#define C_MIN(a,b)			((a)<(b) ? (a) : (b))
#define C_MAX(a,b)			((a)>(b) ? (a) : (b))
#define C_MIN3(a,b,c)		C_MIN((a),C_MIN((b),(c)))
#define C_MAX3(a,b,c)		C_MAX((a),C_MAX((b),(c)))
#define C_MIN4(a,b,c,d)		C_MIN((a),C_MIN3((b),(c),(d)))
#define C_MAX4(a,b,c,d)		C_MAX((a),C_MAX3((b),(c),(d)))
#define C_XCHG(type,a,b)	do{ type t = a; a = b; b = t; } while(0)
#define C_FIX8(f)			((int)(f*(1<<8)+.5))
#define C_ALIGN(x,a)		(((x)+((a)-1))&~((a)-1))
#define ARRAY_ELEMS(a)		((sizeof(a))/(sizeof(a[0])))

/****************************************************************************
 * reduce_fraction:
 ****************************************************************************/
#define REDUCE_FRACTION(name, type)\
void name(type *n, type *d)\
{                   \
    type a = *n;    \
    type b = *d;    \
    type c;         \
    if(!a || !b)  \
        return;     \
    c = a % b;      \
    while(c){       \
        a = b;      \
        b = c;      \
        c = a % b;  \
    }               \
    *n /= b;        \
    *d /= b;        \
}

typedef uint8_t  pixel;
typedef uint32_t pixel4;

/* should be same as i264's csp definition */
typedef enum c_csp {
	C_CSP_I400	= 0x000,    /* yuv 4:0:0 planar */
	C_CSP_I420	= 0x001,    /* yuv 4:2:0 planar */
	C_CSP_I422	= 0x002,    /* yuv 4:2:2 planar */
	C_CSP_I444	= 0x003,    /* yuv 4:4:4 planar */
    C_CSP_COUNT = 0x004,    /* Number of supported internal color spaces */
	C_CSP_NV12	= 0x004,    /* yuv 4:2:0, with one y plane and one packed u+v plane */
	C_CSP_NV21	= 0x005,    /* yuv 4:2:0, with one y plane and one packed v+u plane*/
	C_CSP_T420	= 0x006,	/* yuv 4:2:0, with one y plane and one tile packed u+v plane */
	C_CSP_MAX	= 0x007,
} c_csp_t;

typedef enum c_mc_type {
	NV12_TO_NV12	= 0,
	I420_TO_NV12	= 1,
	NV21_TO_NV12	= 2,
	T420_TO_NV12	= 3,

	NV12_TO_T420	= 4,
	I420_TO_T420	= 5,
	NV21_TO_T420	= 6,
	T420_TO_T420	= 7,
	MC_MAX			= 8,
} c_mc_type_t;

typedef struct c_mc_function {
	void (*copy_frame[MC_MAX])(pixel *dst[2], int i_dst_stride[2],
			pixel *src[4], int i_src_stride[4], int width, int height);
} c_mc_function_t;

typedef enum c_prot {
	C_H264E		= 0,
	C_JPEGE,
	C_JPEGD,
	C_H265E,
	C_PROT_MAX,
} c_protol_t;

typedef enum c_soc_type {
	C_M200		= 0,
	C_T10,
	C_T20,
	C_T30,
	C_SOC_MAX,
} c_soc_type_t;

typedef enum c_soc_vb_type {
	C_SOC_VB_RD	= 0,
	C_SOC_VB_ENC,
	C_SOC_VB_BS,
	C_SOC_VB_DMA,
	C_SOC_VB_EXT,
	C_SOC_VB_NCU,
	C_SOC_VB_MAX,
} c_soc_vb_type_t;

/* Log level */
typedef enum c_log_level {
	C_LOG_NONE          = (-1),
	C_LOG_ERROR         = 0,
	C_LOG_WARNING       = 1,
	C_LOG_INFO          = 2,
	C_LOG_DEBUG         = 3,
} c_log_level_t;

typedef enum c_overscan {
	C_OVERSCAN_UNDEF	= 0,
	C_OVERSCAN_NONE		= 1,
	C_OVERSCAN_HAVE		= 2,
} c_overscan_t;

typedef enum c_vf {
	C_VF_COMPONENT		= 0,
	C_VF_PAL			= 1,
	C_VF_NTSC			= 2,
	C_VF_SECAM			= 3,
	C_VF_MAC			= 4,
	C_VF_UNDEF			= 5,
	C_VF_RESERVED		= 6,
} c_vf_t;

typedef enum c_skip_type {
	C_STYPE_N1X			= 0,
	C_STYPE_N2X			= 1,
	C_STYPE_N4X			= 2,
	C_STYPE_HN1_FALSE	= 3,
	C_STYPE_HN1_TRUE	= 4,
	C_STYPE_H1M_FALSE	= 5,
	C_STYPE_H1M_TRUE	= 6,
} c_skip_type_t;

typedef enum c_frame_skip_type {
	C_FS_IDR		= 0,
	C_FS_LBASE      = 1,
	C_FS_SBASE      = 2,
	C_FS_ENHANCE	= 3,
} c_fsktype_t;

/* c_chroma_format:
 * Specifies the chroma formats that icommon supports encoding. When this
 * value is non-zero, then it represents a C_CSP_* that is the only
 * chroma format that icommon supports encoding. If the value is 0 then
 * there are no restrictions.
 */
typedef enum c_pic_struct
{
    C_PIC_STRUCT_AUTO              = 0, // automatically decide (default)
    C_PIC_STRUCT_PROGRESSIVE       = 1, // progressive frame
    // "TOP" and "BOTTOM" are not supported in icommon (PAFF only)
    C_PIC_STRUCT_TOP_BOTTOM        = 4, // top field followed by bottom
    C_PIC_STRUCT_BOTTOM_TOP        = 5, // bottom field followed by top
    C_PIC_STRUCT_TOP_BOTTOM_TOP    = 6, // top field, bottom field, top field repeated
    C_PIC_STRUCT_BOTTOM_TOP_BOTTOM = 7, // bottom field, top field, bottom field repeated
    C_PIC_STRUCT_DOUBLE            = 8, // double frame
    C_PIC_STRUCT_TRIPLE            = 9, // triple frame
} c_pic_struct_t;

typedef struct c_img {
    c_csp_t i_csp;       /* Colorspace */
    int     i_plane;     /* Number of image planes */
    uint32_t    i_stride[4]; /* Strides for each plane */
	uint32_t    i_lines[4];
	uint8_t *alloc_plane[4];
    uint8_t *plane[4];   /* Pointers to each plane */
	int		is_vir;
	int		is_use_ncu;
	int		ncu_middate_size;
	uint32_t ncu_out_p;
	uint32_t ncu_middate_p;
	void	*priv;
} c_img_t;

typedef struct {
	int	i_skip_type;
	int	m;
	int n;
	int max_same_scenecnt;
	int b_enable_scenecut;
	int b_black_enhance;
} c_high_skip_t;

typedef struct i264e_superfrm_param {
    int mode;
    int iframe_bits_thresd;
    int pframe_bits_thresd;
    int bframe_bits_thresd;
    int priority;
} c_superfrm_param_t;

typedef struct
{
	uint8_t	roi_en;
	uint8_t	roi_md;
	int8_t	roi_qp;
	uint8_t	roi_lmbx;
	uint8_t	roi_rmbx;
	uint8_t	roi_umby;
	uint8_t	roi_bmby;
} c_roi_t;

typedef struct watermark watermark_t;

extern void c_reduce_fraction(uint32_t *n, uint32_t *d);
extern void c_reduce_fraction64(uint64_t *n, uint64_t *d);
extern void c_log_default(const char *module, int i_level, const char *psz_fmt, va_list arg);
extern void c_log(int i_level, const char *psz_fmt, ... );
extern int64_t c_mdate(void);
extern void *c_malloc(int size, int align);
extern void *c_malloc_check_zero(int size, int align, const char *format, ...);
extern void c_free(void *addr);
extern intptr_t c_virt_to_phys(intptr_t vaddr);
extern intptr_t c_phys_to_virt(intptr_t paddr);
extern int c_mc_init(c_mc_function_t *mc);
extern int c_mc_type(c_csp_t dst, c_csp_t src);
extern int c_align(int x, int align, int disalign);
extern int c_clip3(int v, int i_min, int i_max);
extern double c_clip3f(double v, double f_min, double f_max);
extern int c_median(int a, int b, int c);
extern void *c_kalloc(int size, int align);
extern void c_kfree(void *addr);
extern intptr_t c_kvirt_to_kphys(intptr_t vaddr);
extern intptr_t c_kphys_to_kvirt(intptr_t paddr);
extern void c_resize_simd(uint8_t* src, uint8_t* dst, int src_w, int src_h, int dst_w, int dst_h, bool is_clip, int clip_offset, int clip_w, int clip_h, c_csp_t format, uint16_t *resize_buf);
extern void c_resize_c(uint8_t* src, uint8_t* dst, int src_w, int src_h, int dst_w, int dst_h, bool is_clip, int clip_offset, int clip_w, int clip_h, c_csp_t format, uint16_t *resize_buf);
extern int c_save_to_file(int fd, void *buf, int size);

extern watermark_t *watermark_init(void);
extern int embed_watermark(watermark_t *handler, unsigned char *ybuf, int w, int h);
extern int check_watermark(watermark_t *handler, unsigned char *ybuf, int w, int h);
extern void watermark_deinit(watermark_t *handler);

#ifdef __cplusplus
}
#endif
#endif /* __ICOMMON_H__ */
