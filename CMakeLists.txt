cmake_minimum_required(VERSION 3.5)

project(client_gb VERSION 0.1 LANGUAGES CXX)

option(STRIP_EXECUTABLES "Strip libraries and executables" ON)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/dist)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/dist)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(STRIP_EXECUTABLES)
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -s")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -s")
endif()

### libs

find_package(Qt5 COMPONENTS Widgets Sql Xml XmlPatterns Network REQUIRED)

add_subdirectory(3rdparty/LimeReport)
include_directories(3rdparty/LimeReport/include)

include_directories(3rdparty/miniz-cpp)

add_subdirectory(3rdparty/QXlsx/QXlsx)
include_directories(3rdparty/QXlsx/QXlsx/header)

### end libs

include_directories(src)

set(PROJECT_SOURCES
        src/main.cpp
        src/mainwindow.cpp
        src/mainwindow.h
        src/mainwindow.ui
        src/editor_plan.h
        src/editor_plan.cpp
        src/editor_plan.ui
        src/sql_queries.h
        src/sql_queries.cpp
        src/model_kkrb.h
        src/model_kkrb.cpp
        src/delegate_readonly.h
        src/delegate_readonly.cpp
        src/delegate_number_money.h
        src/delegate_number_money.cpp
        src/delegate_payment_type.h
        src/delegate_payment_type.cpp
        src/delegate_expenses_part.h
        src/delegate_expenses_part.cpp
        src/plan_document.h
        src/plan_document.cpp
        src/plan_document.ui
        src/fact_document.h
        src/fact_document.cpp
        src/fact_document.ui
        src/plan_document_editor.h
        src/plan_document_editor.cpp
        src/plan_document_editor.ui
        src/fact_document_editor.h
        src/fact_document_editor.cpp
        src/fact_document_editor.ui
        src/editor_fact.h
        src/editor_fact.cpp
        src/editor_fact.ui
        src/analitic_plan.h
        src/analitic_plan.cpp
        src/analitic_plan.ui
        src/analitic_finance.h
        src/analitic_finance.cpp
        src/analitic_finance.ui
        src/sqlextendedfilter.h
        src/sqlextendedfilter.cpp
        src/xml_parser.h
        src/xml_parser.cpp
        src/json_service.h
        src/json_service.cpp
        src/settings.h
        src/settings.cpp
        src/settings.ui
        src/error_dialog.h
        src/error_dialog.cpp
        src/error_dialog.ui
        src/updater.h
        src/updater.cpp
        src/delegate_status.h
        src/delegate_status.cpp
        src/migration_db.h
        src/migration_db.cpp
        src/editor_signature.h
        src/editor_signature.cpp
        src/editor_signature.ui
        src/auth_dialog.h
        src/auth_dialog.cpp
        src/auth_dialog.ui
        src/request_manager.h
        src/request_manager.cpp
        src/table_view_utils.h
        src/table_view_utils.cpp
        src/popup.h
        src/popup.cpp
        src/expenses_part.h
        src/expenses_part.cpp
        src/delegate_center_aligment.h
        src/delegate_center_aligment.cpp
        src/mdi_subwindow_event_filter.h
        src/mdi_subwindow_event_filter.cpp
        src/delegate_limited_trimmed_text.h
        src/delegate_limited_trimmed_text.cpp
        src/about.h
        src/about.cpp
        src/about.ui
        res_file.qrc
        migration.qrc
        app.rc
)

add_executable(client_gb
    ${PROJECT_SOURCES}
)

target_link_libraries(client_gb PRIVATE
    Qt5::Widgets
    Qt5::Sql
    Qt5::Xml
    Qt5::XmlPatterns
    Qt5::Network

    QXlsx

    limereport
)

if(WIN32)
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "-mwindows")
endif()

if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_custom_target(windeploy
        COMMAND "windeployqt"
        # --release
        --no-angle
        --no-opengl
        --no-opengl-sw
        --compiler-runtime
        --no-system-d3d-compiler
        --no-webkit
        --no-webkit2
#        --no-translations
        --printsupport
        --qml
        "$<TARGET_FILE:client_gb>"
    )
endif()
