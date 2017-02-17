find_package(KF5 REQUIRED COMPONENTS Notifications)
find_package( PythonInterp REQUIRED )
find_package( PythonLibs REQUIRED )
include_directories ( ${PYTHON_INCLUDE_DIRS} )

set(kdeconnect_pyext_SRCS
    pyext.cpp
    script.cpp
    modules/module.cpp
)

kdeconnect_add_plugin(kdeconnect_pyext JSON kdeconnect_pyext.json SOURCES ${kdeconnect_pyext_SRCS})

target_link_libraries(kdeconnect_pyext
    kdeconnectcore
    Qt5::DBus
    KF5::I18n
    KF5::Notifications
    ${PYTHON_LIBRARIES}
)
