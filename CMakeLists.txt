cmake_minimum_required(VERSION 3.10)
project(Delivery-System- C)

set(CMAKE_C_STANDARD 11)


include_directories(/home/student/raylib/src)
link_directories(/home/student/raylib/build/raylib)


add_executable(app
    InputHandler.c
    Dijkstra.c
    main.c        
    Graph.c
    Visualizer.c
)


target_link_libraries(app
    raylib
    GL
    m
    pthread
    dl
    rt
    X11
)
