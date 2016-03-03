define_module(pysurfrec LIBRARY LINKS surfrec boost-python)
add_custom_target(rename_pysurfrec_lib ALL COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/python/libpysurfrec.so ${CMAKE_BINARY_DIR}/python/surfrec.so)
add_dependencies(rename_pysurfrec_lib pysurfrec)
