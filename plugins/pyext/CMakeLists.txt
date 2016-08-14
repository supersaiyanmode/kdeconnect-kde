find_package(KF5 REQUIRED COMPONENTS Notifications)

set(kdeconnect_pyext_SRCS
    pyext.cpp
    script.cpp
)

kdeconnect_add_plugin(kdeconnect_pyext JSON kdeconnect_pyext.json SOURCES ${kdeconnect_pyext_SRCS})

target_link_libraries(kdeconnect_pyext
    kdeconnectcore
    Qt5::DBus
    KF5::I18n
    KF5::Notifications
)
