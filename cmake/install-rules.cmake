install(
    TARGETS diy-sqlite_exe
    RUNTIME COMPONENT diy-sqlite_Runtime
)

if(PROJECT_IS_TOP_LEVEL)
  include(CPack)
endif()
