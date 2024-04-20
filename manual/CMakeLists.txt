find_package(Sphinx REQUIRED)

configure_file(conf.py.in ${CMAKE_CURRENT_SOURCE_DIR}/conf.py @ONLY)

# set(SPHINX_SOURCE ${CMAKE_CURRENT_SOURCE_DIR})
# set(SPHINX_BUILD ${PROJECT_SOURCE_DIR}/docroot)

# add_custom_target("Sphinx-${PROJECT_NAME}-html" ALL
# 	COMMAND ${SPHINX_EXECUTABLE} -b dirhtml
# 	${SPHINX_SOURCE} ${SPHINX_BUILD}
# 	BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}
# 	COMMENT "Generating documentation with Sphinx")

add_custom_target(
	COMMAND ${SPHINX_EXECUTABLE} -b man ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/salt.rst
	BYPRODUCTS salt.1
	DEPENDS salt.rst
	COMMENT "Generating manual pages with Sphinx")

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/salt.1
	DESTINATION ${CMAKE_INSTALL_DATADIR}/man/man1)
