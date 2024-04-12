find_package(Sphinx REQUIRED)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/conf.py.in ${CMAKE_CURRENT_SOURCE_DIR}/conf.py @ONLY)

set(SPHINX_SOURCE ${CMAKE_CURRENT_SOURCE_DIR})
set(SPHINX_BUILD ${PROJECT_SOURCE_DIR}/docroot)

add_custom_target("Sphinx-${PROJECT_NAME}-html" ALL
	COMMAND ${SPHINX_EXECUTABLE} -b dirhtml
	${SPHINX_SOURCE} ${SPHINX_BUILD}
	COMMENT "Generating documentation with Sphinx")

add_custom_target("Sphinx-${PROJECT_NAME}-man" ALL
	COMMAND ${SPHINX_EXECUTABLE} -b man
	${SPHINX_SOURCE} ${CMAKE_CURRENT_BINARY_DIR}
	COMMENT "Generating manual pages with Sphinx")

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/salt.1
	DESTINATION ${CMAKE_INSTALL_DATADIR}/man/man1)
