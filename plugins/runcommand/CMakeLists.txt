set(kdeconnect_runcommand_SRCS
    runcommandplugin.cpp
)

kdeconnect_add_plugin(kdeconnect_runcommand JSON kdeconnect_runcommand.json SOURCES ${kdeconnect_runcommand_SRCS})

target_link_libraries(kdeconnect_runcommand
    kdeconnectcore
    Qt5::DBus
    KF5::I18n)
