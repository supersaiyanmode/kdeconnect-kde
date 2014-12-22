set(kdeconnect_findmyphone_SRCS
    findmyphoneplugin.cpp
)

kde4_add_plugin(kdeconnect_findmyphone ${kdeconnect_findmyphone_SRCS})

target_link_libraries(kdeconnect_findmyphone kdeconnectcore ${KDE4_KDEUI_LIBS})

install(TARGETS kdeconnect_findmyphone DESTINATION ${PLUGIN_INSTALL_DIR} )
install(FILES kdeconnect_findmyphone.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
