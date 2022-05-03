add_library(constyaml OBJECT
  list.cppm
  dict.cppm
  yaml.cppm
)

target_compile_options(constyaml PRIVATE -x c++)
