find_package(KdepimLibs REQUIRED)

find_package(TelepathyQt4 0.9.2.1 REQUIRED)
find_package(TelepathyQt4Service 0.9.2.1 REQUIRED)

include_directories(
    ${TELEPATHY_QT4_INCLUDE_DIR}
)

set(kdeconnect_telepathy_SRCS
    telepathyplugin.cpp
    telepathyplugin.h
)

kde4_add_plugin(kdeconnect_telepathy ${kdeconnect_telepathy_SRCS})

target_link_libraries(kdeconnect_telepathy
   kdeconnectcore
   connectcm
   ${TELEPATHY_QT4_LIBRARIES}
   ${TELEPATHY_QT4_SERVICE_LIBRARIES}
   ${KDEPIMLIBS_KABC_LIBS}
)

install(TARGETS kdeconnect_telepathy DESTINATION ${PLUGIN_INSTALL_DIR} )
install(FILES kdeconnect_telepathy.desktop DESTINATION ${SERVICES_INSTALL_DIR} )

add_subdirectory(telepathy-cm)
