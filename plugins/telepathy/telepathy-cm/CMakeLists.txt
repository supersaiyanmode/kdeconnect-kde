project(connectcm)

add_definitions(-DSIMPLECM_LIBRARY)

set(simplecm_HEADERS
    connection.h
    protocol.h
    textchannel.h
    simplecm_export.h
)

set(simplecm_SOURCES
    connection.cpp
    protocol.cpp
    textchannel.cpp
    main.cpp
    kdeconnecttelepathyprotocol.cpp
)

kde4_add_library(connectcm SHARED ${simplecm_SOURCES} ${simplecm_HEADERS})

# set_target_properties(connectcm PROPERTIES AUTOMOC TRUE)

target_link_libraries(connectcm
    ${QT_QTCORE_LIBRARY}
    ${QT_QTDBUS_LIBRARY}
    ${TELEPATHY_QT4_LIBRARIES}
    ${TELEPATHY_QT4_SERVICE_LIBRARIES}
)

install (TARGETS connectcm
    ${INSTALL_TARGETS_DEFAULT_ARGS}
)


# install(
#     TARGETS simplecm
#     ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
#     COMPONENT Library
# )
#
# install(
#     FILES ${simplecm_HEADERS}
#     DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/simplecm
#     COMPONENT Devel
# )
