=== =================================================
== common
mat4  GetViewMatrix() {
#ifdef LOFI_HAS_view
    return mat4(LOFI_GET_view());
#else
    return mat4(1);
#endif
}
mat4  GetInverseViewMatrix() {
#ifdef LOFI_HAS_invview
  return mat4(LOFI_GET_invview());
#else
    return mat4(1);
#endif
}
mat4  GetProjectionMatrix() {
#ifdef LOFI_HAS_projection
  return mat4(LOFI_GET_projection());
#else
    return mat4(1);
#endif
}
mat4  GetModelMatrix() {
#ifdef LOFI_HAS_model
  return mat4(LOFI_GET_model());
#else
  return mat4(1);
#endif
}
mat4  GetMVPMatrix() {
  return GetProjectionMatrix() * GetViewMatrix() * GetModelMatrix();
}