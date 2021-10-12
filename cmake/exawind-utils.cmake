function(set_cuda_build_properties target)
  get_target_property(_tgt_src ${target} SOURCES)
  list(FILTER _tgt_src INCLUDE REGEX "\\.cpp")
  set_source_files_properties(${_tgt_src} PROPERTIES LANGUAGE CUDA)
  set_target_properties(${target} PROPERTIES CUDA_SEPARABLE_COMPILATION ON)
  set_target_properties(${target} PROPERTIES CUDA_ARCHITECTURES "70")
  set_target_properties(${target} PROPERTIES CUDA_RESOLVE_DEVICE_SYMBOLS ON)
  #set_target_properties(${target} PROPERTIES LINKER_LANGUAGE CUDA)
endfunction(set_cuda_build_properties)
