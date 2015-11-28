qt5_add_resources(kclivingroom_SRCS resources.qrc)

add_executable(kdeconnect-livingroom main.cpp ${kclivingroom_SRCS})
target_link_libraries(kdeconnect-livingroom Qt5::Quick KF5::CoreAddons KF5::I18n kdeconnectinterfaces)

install(TARGETS kdeconnect-livingroom ${INSTALL_TARGETS_DEFAULT_ARGS})
install(PROGRAMS org.kde.kdeconnect.livingroom.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
