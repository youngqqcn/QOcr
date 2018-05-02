DISTFILES += \
    $$PWD/json.pri \
    $$PWD/CMakeLists.txt

HEADERS += \
    $$PWD/allocator.h \
    $$PWD/assertions.h \
    $$PWD/autolink.h \
    $$PWD/config.h \
    $$PWD/features.h \
    $$PWD/forwards.h \
    $$PWD/json.h \
    $$PWD/json_tool.h \
    $$PWD/reader.h \
    $$PWD/value.h \
    $$PWD/version.h \
    $$PWD/version.h.in \
    $$PWD/writer.h

SOURCES += \
    $$PWD/json_reader.cpp \
    $$PWD/json_value.cpp \
    $$PWD/json_valueiterator.inl \
    $$PWD/json_writer.cpp
