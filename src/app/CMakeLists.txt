qt_add_executable(millennium_serpent
    main.cpp
    mainwindow.cpp
    login.cpp
)
target_link_libraries(millennium_serpent PRIVATE 
    Qt6::Widgets 
    Qt6::Gui
    Qt6::Network
    millennium_crypt
    millennium_server
)

set_target_properties(millennium_serpent PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)