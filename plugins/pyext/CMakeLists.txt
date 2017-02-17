find_package(KF5 REQUIRED COMPONENTS Notifications)
find_package( PythonInterp 3 REQUIRED )
find_package( PythonLibs 3 REQUIRED )
set(Python_ADDITIONAL_VERSIONS 3.6)
include_directories ( /usr/include/python3.6m/ ${PYTHON_INCLUDE_DIRS} )

set(kdeconnect_pyext_SRCS
    pyext.cpp
    script.cpp
    modules/module.cpp
    Interpreter/interpreter.cpp
)

kdeconnect_add_plugin(kdeconnect_pyext JSON kdeconnect_pyext.json SOURCES ${kdeconnect_pyext_SRCS})

target_link_libraries(kdeconnect_pyext
    kdeconnectcore
    Qt5::DBus
    KF5::I18n
    KF5::Notifications
    ${PYTHON_LIBRARIES}
)
