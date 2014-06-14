find_package(KDE4 REQUIRED)
find_package(QJSON REQUIRED)
find_package(QCA2 REQUIRED)

find_package(KdepimLibs REQUIRED)

find_package(TelepathyQt4 0.9.2.1 REQUIRED)
find_package(TelepathyQt4Service 0.9.2.1 REQUIRED)

include(KDE4Defaults)

include_directories(
    ${QJSON_INCLUDE_DIR}
    ${QCA2_INCLUDE_DIR}
    ${KDE4_INCLUDES}
    ${TELEPATHY_QT4_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}
)

include(KDE4Defaults)
include_directories(${KDE4_INCLUDES})

set(kdeconnect_telepathy_SRCS
    ../kdeconnectplugin.cpp
    ../../networkpackage.cpp
    ../../filetransferjob.cpp
    ../../kdebugnamespace.cpp
    telepathyplugin.cpp
    telepathyplugin.h
)

kde4_add_plugin(kdeconnect_telepathy ${kdeconnect_telepathy_SRCS})

target_link_libraries(kdeconnect_telepathy kdeconnect
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KIO_LIBS}
    ${QT_QTNETWORK_LIBRARY}
    ${QJSON_LIBRARIES}
    ${QCA2_LIBRARIES}
    ${TELEPATHY_QT4_LIBRARIES}
    ${TELEPATHY_QT4_SERVICE_LIBRARIES}
    ${KDEPIMLIBS_KABC_LIBS}
    -lsimplecm
)

install(TARGETS kdeconnect_telepathy DESTINATION ${PLUGIN_INSTALL_DIR} )
install(FILES kdeconnect_telepathy.desktop DESTINATION ${SERVICES_INSTALL_DIR} )

