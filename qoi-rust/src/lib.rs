use libc;
use qoi::{decode_header, decode_to_buf, encode_max_len, encode_to_buf};

#[repr(C)]
pub struct qoi_desc {
    width: libc::c_uint,
    height: libc::c_uint,
    channels: libc::c_uchar,
    colorspace: libc::c_uchar,
}

#[no_mangle]
pub extern "C" fn qoi_rust_decode(data: *const libc::c_void, size: libc::c_int, desc: *mut qoi_desc, _: libc::c_int) -> *mut libc::c_void{
    if data.is_null() || desc.is_null() || (size as usize) < qoi::consts::QOI_HEADER_SIZE + qoi::consts::QOI_PADDING_SIZE {
        return std::ptr::null_mut();
    }
    let input = unsafe{std::slice::from_raw_parts(data as *const u8, size as usize)};
    let header_ = decode_header(input);
    if let Err(_) = header_ {
        return std::ptr::null_mut();
    }
    let header = header_.unwrap();
    unsafe {
        (*desc).width = header.width;
        (*desc).height = header.height;
        (*desc).channels = header.channels.as_u8();
        (*desc).colorspace = header.colorspace.as_u8();
    }
    let ptr = unsafe {libc::malloc(header.n_bytes()) as *mut u8};
    let decode_ = decode_to_buf(unsafe{std::slice::from_raw_parts_mut(ptr, header.n_bytes())}, input);
    if let Err(_) = decode_ {
        return std::ptr::null_mut();
    }
    ptr as *mut libc::c_void
}

#[no_mangle]
pub extern "C" fn qoi_rust_encode(data: *const libc::c_void, desc: *const qoi_desc, out_len: *mut libc::c_int) -> *mut libc::c_void{
    if data.is_null() || desc.is_null() || out_len.is_null() || unsafe{(*desc).width == 0 || (*desc).height == 0 || !(3..5).contains(&(*desc).channels) || (*desc).colorspace > 1} {
        return std::ptr::null_mut();
    }
    let input = unsafe{std::slice::from_raw_parts(data as *const u8, ((*desc).width as usize).saturating_mul((*desc).height as usize).saturating_mul((*desc).channels as usize))};
    let width = unsafe {(*desc).width};
    let height = unsafe {(*desc).height};
    let size = unsafe { encode_max_len(width, height, (*desc).channels) };
    let ptr = unsafe {libc::malloc(size) as *mut u8};
    let actual_size = encode_to_buf(unsafe{std::slice::from_raw_parts_mut(ptr, size)}, input, width, height);
    if let Err(_) = actual_size {
        return std::ptr::null_mut();
    }
    unsafe{ *out_len = actual_size.unwrap() as libc::c_int;}
    ptr as *mut libc::c_void
}

#[no_mangle]
pub extern "C" fn qoi_rust_free(data: *mut libc::c_void){
    unsafe{libc::free(data)}
}
