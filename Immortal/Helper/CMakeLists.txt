project(AppleInterface-Helper)

add_library(${PROJECT_NAME} STATIC
    MacOSPlatform.mm)

target_link_libraries(${PROJECT_NAME} PRIVATE 
    "-framework Cocoa"
    "-framework IOKit"
    "-framework CoreFoundation")
