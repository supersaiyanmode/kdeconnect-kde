set(kdeconnect_reminder_SRCS
    reminderplugin.cpp
)

kde4_add_plugin(kdeconnect_reminder ${kdeconnect_reminder_SRCS})

target_link_libraries(kdeconnect_reminder kdeconnectcore ${QT_QTGUI_LIBRARY} ${KDEPIMLIBS_AKONADI_LIBS} ${KDEPIMLIBS_AKONADI_CALENDAR_LIBS} ${KDEPIMLIBS_AKONADI_KCAL_LIBS} ${KDEPIMLIBS_KCALCORE_LIBS})

install(TARGETS kdeconnect_reminder DESTINATION ${PLUGIN_INSTALL_DIR} )
install(FILES kdeconnect_reminder.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
