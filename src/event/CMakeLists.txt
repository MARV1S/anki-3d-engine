FILE(GLOB ANKI_EVENT_SOURCES *.cpp)
FILE(GLOB ANKI_EVENT_HEADERS *.h)

ADD_LIBRARY(ankievent ${ANKI_EVENT_SOURCES} ${ANKI_EVENT_HEADERS})
