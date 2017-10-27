QT += widgets
qtHaveModule(printsupport): QT += printsupport

HEADERS       = imageviewer.h \
    action.h
SOURCES       = imageviewer.cpp \
                main.cpp \
    action.cpp

# install
target.path = $$[QT_INSTALL_EXAMPLES]/widgets/widgets/imageviewer
INSTALLS += target
