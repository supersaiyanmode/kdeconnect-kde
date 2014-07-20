set(kdeconnect_contact_SRCS
    contactplugin.cpp
)

kde4_add_plugin(kdeconnect_contact ${kdeconnect_contact_SRCS})

target_link_libraries(kdeconnect_contact kdeconnectcore ${QT_QTGUI_LIBRARY} ${KDEPIMLIBS_AKONADI_LIBS} ${KDEPIMLIBS_AKONADI_KABC_LIBS} ${KDEPIMLIBS_AKONADI_CONTACT_LIBS} ${KDEPIMLIBS_KABC_LIBS})

install(TARGETS kdeconnect_contact DESTINATION ${PLUGIN_INSTALL_DIR} )
install(FILES kdeconnect_contact.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
