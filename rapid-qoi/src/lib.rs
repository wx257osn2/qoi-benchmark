use rapid_qoi::Colors;

#[repr(C)]
pub struct qoi_desc {
    width: libc::c_uint,
    height: libc::c_uint,
    channels: libc::c_uchar,
    colorspace: libc::c_uchar,
}

const QOI_HEADER_SIZE: usize = 14;
const QOI_PADDING_SIZE: usize = 8;

#[no_mangle]
#[doc(hidden)]
pub unsafe extern "C" fn rapid_qoi_decode(
    data: *const libc::c_void,
    size: libc::c_int,
    desc: *mut qoi_desc,
    _: libc::c_int,
) -> *mut libc::c_void {
    if data.is_null() || desc.is_null() || (size as usize) < QOI_HEADER_SIZE + QOI_PADDING_SIZE {
        return std::ptr::null_mut();
    }
    let input = std::slice::from_raw_parts(data as *const u8, size as usize);
    let header_ = rapid_qoi::Qoi::decode_header(input);
    if header_.is_err() {
        return std::ptr::null_mut();
    }
    let header = header_.unwrap();
    (*desc).width = header.width;
    (*desc).height = header.height;
    (*desc).channels = header.colors.channels() as u8;
    (*desc).colorspace = match header.colors {
        Colors::Srgb | Colors::SrgbLinA => 0u8,
        _ => 1u8,
    };
    let ptr = libc::malloc(header.decoded_size()) as *mut u8;
    let decode_ = header.decode_skip_header(
        &input[QOI_HEADER_SIZE..],
        std::slice::from_raw_parts_mut(ptr, header.decoded_size()),
    );
    if decode_.is_err() {
        return std::ptr::null_mut();
    }
    ptr as *mut libc::c_void
}

#[no_mangle]
#[doc(hidden)]
pub unsafe extern "C" fn rapid_qoi_encode(
    data: *const libc::c_void,
    desc: *const qoi_desc,
    out_len: *mut libc::c_int,
) -> *mut libc::c_void {
    if data.is_null()
        || desc.is_null()
        || out_len.is_null()
        || (*desc).width == 0
        || (*desc).height == 0
        || !(3..5).contains(&(*desc).channels)
        || (*desc).colorspace > 1
    {
        return std::ptr::null_mut();
    }
    let input = std::slice::from_raw_parts(
        data as *const u8,
        ((*desc).width as usize)
            .saturating_mul((*desc).height as usize)
            .saturating_mul((*desc).channels as usize),
    );
    let encoder = rapid_qoi::Qoi {
        width: (*desc).width,
        height: (*desc).height,
        colors: match ((*desc).channels, (*desc).colorspace) {
            (3, 0) => rapid_qoi::Colors::Srgb,
            (4, 0) => rapid_qoi::Colors::SrgbLinA,
            (3, 1) => rapid_qoi::Colors::Rgb,
            (4, 1) => rapid_qoi::Colors::Rgba,
            _ => return std::ptr::null_mut(),
        },
    };
    let size = encoder.encoded_size_limit();
    let ptr = libc::malloc(size) as *mut u8;
    let actual_size = encoder.encode(input, std::slice::from_raw_parts_mut(ptr, size));
    if actual_size.is_err() {
        return std::ptr::null_mut();
    }
    *out_len = actual_size.unwrap() as libc::c_int;
    ptr as *mut libc::c_void
}

#[no_mangle]
#[doc(hidden)]
pub unsafe extern "C" fn rapid_qoi_free(data: *mut libc::c_void) {
    libc::free(data)
}
