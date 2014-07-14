set(kdeconnect_calendar_SRCS
    calendarplugin.cpp
)

kde4_add_plugin(kdeconnect_calendar ${kdeconnect_calendar_SRCS})

target_link_libraries(kdeconnect_calendar kdeconnectcore ${QT_QTGUI_LIBRARY} ${KDEPIMLIBS_AKONADI_LIBS} ${KDEPIMLIBS_AKONADI_CALENDAR_LIBS} ${KDEPIMLIBS_AKONADI_KCAL_LIBS} ${KDEPIMLIBS_KCALCORE_LIBS})

install(TARGETS kdeconnect_calendar DESTINATION ${PLUGIN_INSTALL_DIR} )
install(FILES kdeconnect_calendar.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
