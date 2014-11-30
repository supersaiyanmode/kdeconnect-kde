include_directories(
    ${TELEPATHY_QT4_INCLUDE_DIR}
)

set(kdeconnect_telepathy_SRCS
    telepathyplugin.cpp
    telepathyplugin.h
)

add_library(kdeconnect_telepathy MODULE ${kdeconnect_telepathy_SRCS})

target_link_libraries(kdeconnect_telepathy
    kdeconnectcore
    connectcm
    KF5::I18n
    KF5::Service
    ${TELEPATHY_QT5_LIBRARIES}
    ${TELEPATHY_QT5_SERVICE_LIBRARIES}
)

install(TARGETS kdeconnect_telepathy DESTINATION ${PLUGIN_INSTALL_DIR} )
install(FILES kdeconnect_telepathy.desktop DESTINATION ${SERVICES_INSTALL_DIR} )

