cmake_minimum_required(VERSION 3.10)

project(project)

include_directories(${PROJECT_SOURCE_DIR}/include)

#adaugam fisierele sursa comuse
set(COMMON_SOURCES
	src/common/error_handling.c
	src/common/error_log.c
)

#adaugam fisierele sursa pentru server
set(SERV_SOURCES
	src/server/server.c
	src/server/tcp.c
	src/server/unix.c
	src/server/log.c
	src/server/handle_client.c
	src/server/handle_admin.c
	src/server/connection.c
	${COMMON_SOURCES}
)

#cream executabilul pentru server
add_executable(server ${SERV_SOURCES})

#adaugam fisierele sursa pentru clientul simplu
set(CLI_SOURCES
	src/client/client.c
	${COMMON_SOURCES}
)

#cream executabilul pentru clientul simplu
add_executable(client ${CLI_SOURCES})

#adaugam fisierele sursa pentru clientul administrator
set(ADMIN_SOURCES
	src/admin/admin.c
	${COMMON_SOURCES}
)

#cream executabilul pentru clientul administrator
add_executable(admin ${ADMIN_SOURCES})
