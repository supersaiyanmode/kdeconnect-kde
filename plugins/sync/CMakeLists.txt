find_package(KF5 REQUIRED COMPONENTS Notifications KIO)

set(kdeconnect_sync_SRCS
    syncplugin.cpp
)

kdeconnect_add_plugin(kdeconnect_sync JSON kdeconnect_sync.json SOURCES ${kdeconnect_sync_SRCS})

target_link_libraries(kdeconnect_sync
    kdeconnectcore
    Qt5::DBus
    KF5::I18n
    KF5::Notifications
    KF5::KIOWidgets
)
