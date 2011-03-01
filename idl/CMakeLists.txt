set(idl_files pointcloud.idl)

install(FILES ${idl_files} DESTINATION ${RTC_INCLUDE_INSTALL_DIR}/idl
    COMPONENT headers)

